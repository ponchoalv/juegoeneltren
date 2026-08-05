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
#include "../nob.h"

#define FLAG_IMPLEMENTATION
#include "../flag.h"

// Some folder paths that we use throughout the build process.
#define RAYLIB_SRC "./raylib/src/"
#define RAYLIB_STATIC "libraylib.a"
#define RAYLIB_WEB_STATIC "libraylib.web.a"

#define RAYLIB_PLATFORM "PLATFORM=PLATFORM_DESKTOP"
#define RAYLIB_WEB_PLATFORM "PLATFORM=PLATFORM_WEB"

void usage(FILE *stream)
{
    fprintf(stream, "Usage: ./deps [OPTIONS]\n");
    fprintf(stream, "OPTIONS:\n");
    flag_print_options(stream);
}

int main(int argc, char **argv)
{
    // This line enables the self-rebuilding. It detects when nob.c is updated and auto rebuilds it then
    // runs it again.
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool *help = flag_bool("help", false, "Print this help to stdout and exit with 0");
    bool *debug = flag_bool("debug", false, "we accept this one but we don't do anything");
    bool *run = flag_bool("run", false, "we accept this one but we don't do anything");
    bool *web = flag_bool("web", false, "Build the game for webassembly (index.htlm in build/web)");
    bool *no_fetch = flag_bool("no-fetch", false, "Do not fetch raylib library from github (used for -web in the CI)");

    if (!flag_parse(argc, argv))
    {
        usage(stderr);
        flag_print_error(stderr);
        return 1;
    }

    if (*help)
    {
        usage(stdout);
        return 0;
    }

    bool is_not_web_and_raylib_was_compiled = !*web && nob_file_exists(RAYLIB_STATIC);
    bool is_web_and_raylib_web_was_compiled = *web && nob_file_exists(RAYLIB_WEB_STATIC);

    if (is_not_web_and_raylib_was_compiled || is_web_and_raylib_web_was_compiled)
    {
        return 0;
    }

    Nob_Cmd cmd = {0};

    if (!nob_file_exists(RAYLIB_SRC))
    {
        if (*no_fetch)
        {
            nob_log(NOB_ERROR, "Folder not found: " RAYLIB_SRC " remember to clone last raylib repo");
            return 1;
        }
        else
        {
            nob_cmd_append(&cmd, "git");
            nob_cmd_append(&cmd, "clone");
            nob_cmd_append(&cmd, "--depth");
            nob_cmd_append(&cmd, "1");
            nob_cmd_append(&cmd, "git@github.com:ponchoalv/raylib.git");
            if (!nob_cmd_run(&cmd))
                return 1;
        }
    }

    if (!nob_set_current_dir(RAYLIB_SRC))
    {
        nob_log(NOB_ERROR, "Cannot move to folder: " RAYLIB_SRC " remember to clone last raylib repo");
        return 1;
    }

    nob_cmd_append(&cmd, "make", "clean");
    // Let's execute the command.
    if (!nob_cmd_run(&cmd))
        return 1;

    // Let's append the command line arguments
#if !defined(_MSC_VER)
    // On POSIX
    nob_cmd_append(&cmd, "make", *web ? RAYLIB_WEB_PLATFORM : RAYLIB_PLATFORM, "RAYLIB_LIBTYPE=STATIC");
#else
    // On MSVC
    nob_cmd_append(&cmd, "make", *web ? RAYLIB_WEB_PLATFORM : RAYLIB_PLATFORM, "RAYLIB_LIBTYPE=STATIC");
#endif // _MSC_VER

    // Let's execute the command.
    if (!nob_cmd_run(&cmd))
        return 1;

    if (!web && nob_file_exists(RAYLIB_STATIC))
    {
        nob_cmd_append(&cmd, "cp", RAYLIB_STATIC, "../../");
    }
    else if (web && nob_file_exists(RAYLIB_WEB_STATIC))
    {
        nob_cmd_append(&cmd, "cp", RAYLIB_WEB_STATIC, "../../");
    }

    else
    {
        nob_log(NOB_ERROR, "Not found: " RAYLIB_STATIC);
        return 1;
    }

    // Let's execute the command.
    if (!nob_cmd_run(&cmd))
        return 1;

    return 0;
}
