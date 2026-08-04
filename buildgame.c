// This is your build script. You only need to "bootstrap" it once with `cc -o nob nob.c`
// (you can call the executable whatever actually) or `cl nob.c` on MSVC. After that every
// time you run the `nob` executable if it detects that you modifed nob.c it will rebuild
// itself automatically thanks to NOB_GO_REBUILD_URSELF (see below)

// nob.h is an stb-style library https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
// What that means is that it's a single file that acts both like .c and .h files, but by default
// when you include it, it acts only as .h. To make it include implementations of the functions
// you must define NOB_IMPLEMENTATION macro. This is done to give you full control over where
// the implementations go.
#define NOB_IMPLEMENTATION

// Always keep a copy of nob.h in your repo. One of my main pet peeves with build systems like CMake
// and Autotools is that the codebases that use them naturally rot. That is if you do not actively update
// your build scripts, they may not work with the latest version of the build tools. Here we basically
// include the entirety of the source code of the tool along with the code base. It will never get
// outdated (unless you got no standard compliant C compiler lying around, but at that point why are
// you trying to build a C project?)
//
// (In these examples we actually symlinking nob.h, but this is to keep nob.h-s synced among all the
// examples)
#include "nob.h"

#define FLAG_IMPLEMENTATION
#include "./flag.h"

// Some folder paths that we use throughout the build process.
#define BUILD_FOLDER "build/"
#define BUILD_WEB_FOLDER BUILD_FOLDER "web/"
#define SRC_FOLDER "./"
#define DEPS "deps/"
#define INCL_FOLDER DEPS "include/"
#define RAYLIB_WEB_STATIC DEPS "libraylib.web.a"
#define PLATFORM_WEB "-DPLATFORM_WEB"
#define FONTS_FOLDER "fonts"
#define SOUNDS_FOLDER "sounds"

static void append_json_string(Nob_String_Builder *sb, const char *value)
{
    nob_sb_append(sb, '"');
    for (; *value != '\0'; value++)
    {
        if (*value == '"' || *value == '\\')
            nob_sb_append(sb, '\\');
        nob_sb_append(sb, *value);
    }
    nob_sb_append(sb, '"');
}

static bool write_compile_commands(Nob_Cmd command)
{
    Nob_String_Builder database = {0};
    nob_sb_append_cstr(&database, "[\n  {\n    \"directory\": ");
    append_json_string(&database, nob_get_current_dir_temp());
    nob_sb_append_cstr(&database, ",\n    \"file\": \"./main.c\",\n    \"arguments\": [\n");

    for (size_t i = 0; i < command.count; i++)
    {
        nob_sb_append_cstr(&database, "      ");
        append_json_string(&database, command.items[i]);
        nob_sb_append_cstr(&database, i + 1 < command.count ? ",\n" : "\n");
    }
    nob_sb_append_cstr(&database, "    ]\n  }\n]\n");

    bool result = nob_write_entire_file("compile_commands.json", database.items, database.count);
    nob_sb_free(database);
    return result;
}

void usage(FILE *stream)
{
    fprintf(stream, "Usage: ./buildgame [OPTIONS]\n");
    fprintf(stream, "OPTIONS:\n");
    flag_print_options(stream);
}

int main(int argc, char **argv)
{
    // This line enables the self-rebuilding. It detects when nob.c is updated and auto rebuilds it then
    // runs it again.
    NOB_GO_REBUILD_URSELF(argc, argv);

    // It's better to keep all the building artifacts in a separate build folder. Let's create it if it
    // does not exist yet.
    //
    // Majority of the nob command return bool which indicates whether operation has failed or not (true -
    // success, false - failure). If the operation returned false you don't need to log anything, the
    // convention is usually that the function logs what happened to itself. Just do
    // `if (!nob_function()) return;`

    // The working horse of nob is the Nob_Cmd structure. It's a Dynamic Array of strings which represent
    // command line that you want to execute.
    bool *help = flag_bool("help", false, "Print this help to stdout and exit with 0");
    bool *debug = flag_bool("debug", false, "Build the game with debug symbols");
    bool *run = flag_bool("run", false, "Build and run the game in Desktop mode");
    bool *web = flag_bool("web", false, "Build the game for webassembly (index.htlm in build/web)");
    bool *no_fetch = flag_bool("no-fetch", false, "Do not fetch raylib library from github (used for -web in the CI)");
    int i = 1;

    if (!flag_parse(argc, argv))
    {
        usage(stderr);
        flag_print_error(stderr);
        return 1;
    }

    if (*help) {
        usage(stdout);
        return 0;
    }


    Nob_Cmd cmd = {0};

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER))
        return 1;

    if (*web && !nob_mkdir_if_not_exists(BUILD_WEB_FOLDER))
        return 1;

    nob_set_current_dir(DEPS);

    if (!nob_file_exists("deps"))
    {
        nob_cc(&cmd);
        nob_cmd_append(&cmd, "deps.c", "-o", "deps");
        if (!nob_cmd_run(&cmd))
            return 1;
    }

    nob_cmd_append(&cmd, "./deps");

    for (i = 1; i < argc; i++)
    {
        nob_cmd_append(&cmd, argv[i]);
    }

    if (!nob_cmd_run(&cmd))
        return 1;

    nob_set_current_dir("../");

    // Let's append the command line arguments
#if !defined(_MSC_VER)
    // On POSIX
    /// "clang", "-std=c99", "-Wall", "-Wextra", "-Wpedantic", "-framework", "CoreVideo", "-framework", "IOKit",
    /// "-framework", "Cocoa", "-framework", "GLUT", "-framework", "OpenGL", "libraylib.a", "main.c", "-o",
    /// "build/juego_en_el_tren"
    nob_cmd_append(&cmd, *web ? "emcc" : "clang");
    if (*debug)
        nob_cmd_append(&cmd, "-g");
    if (*web)
    {
        /*
          emcc main.c deps/libraylib.web.a \
          -Ideps/include                   \
          -DPLATFORM_WEB -s USE_GLFW=3       \
          --shell-file deps/raylib/src/minshell.html    \
          --preload-file fonts --preload-file sounds    \
          -o build/web/index.html
         */
        nob_cmd_append(&cmd, SRC_FOLDER "main.c", RAYLIB_WEB_STATIC, "-I" INCL_FOLDER, PLATFORM_WEB, "-s", "USE_GLFW=3",
                       "--shell-file", DEPS "raylib/src/minshell.html", "--preload-file", FONTS_FOLDER,
                       "--preload-file", SOUNDS_FOLDER, "-o", BUILD_WEB_FOLDER "index.html");
    }
    else
    {
        nob_cmd_append(&cmd, "-std=c99", "-Wall", "-Wextra", "-Wpedantic", "-I" INCL_FOLDER, "-L" DEPS, "-framework",
                       "CoreVideo", "-framework", "IOKit", "-framework", "Cocoa", "-framework", "GLUT", "-framework",
                       "OpenGL", "-lraylib", SRC_FOLDER "main.c", "-o", BUILD_FOLDER "juego_en_el_tren");
    }
#else
    // TODO On MSVC
    nob_cmd_append(&cmd, "cl", "-std=c99", "-Wall", "-Wextra", "-Wpedantic", "-I" INCL_FOLDER, "-L" DEPS, "-framework",
                   "CoreVideo", "-framework", "IOKit", "-framework", "Cocoa", "-framework", "GLUT", "-framework",
                   "OpenGL", "-lraylib", SRC_FOLDER "main.c", "-o", BUILD_FOLDER "juego_en_el_tren");
#endif // _MSC_VER

    // Let's execute the command.
    Nob_Cmd compile_command = cmd;
    if (!nob_cmd_run(&cmd))
        return 1;
#if !defined(_MSC_VER)
    if (!write_compile_commands(compile_command))
        return 1;
#endif
    // nob_cmd_run() automatically resets the cmd array, so you can nob_cmd_append() more strings
    // into it.

    if (*run)
    {
        nob_cmd_append(&cmd, "./" BUILD_FOLDER "juego_en_el_tren");
        if (!nob_cmd_run(&cmd))
            return 1;
    }

    return 0;
}
