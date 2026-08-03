#include <raylib.h>

#define H_GAME_STATE_IMPLEMENTATION
#include "gamestate.h"

#define H_PLAYER_IMPLEMENTATION
#include "player.h"

#define H_ENEMIES_IMPLEMENTATION
#include "enemies.h"

#define H_MUSIC_MENU_IMPLEMENTATION
#include "musicmenu.h"

#define MENU_FONT "fonts/JetBrainsMono-Medium.ttf"

#define SHOW_FPS

static GameState currentState = {0};

static void SetInitialGameState(GameState *currState, const bool loadSound, const bool setScreen)
{
    // first we reset the state of the game to be in playing mode.
    // then we set the counters to the initial status.
    currState->status = S_playing;
    currState->activeObjects = 0;
    currState->score = 0;
    currState->totalEnemies = 0;

    if (loadSound)
    {
        currState->soundsVol = 0.1f;
        currState->musicVol = 0.1f;

        currState->sounds[SO_bullet] = LoadSound("./sounds/slimeball.wav");
        SetSoundVolume(currState->sounds[SO_bullet], currState->soundsVol);
        currState->sounds[SO_collide] = LoadSound("./sounds/boom.wav");
        SetSoundVolume(currState->sounds[SO_collide], currState->soundsVol);
        currState->sounds[SO_menu] = LoadSound("./sounds/menu.wav");
        SetSoundVolume(currState->sounds[SO_menu], currState->soundsVol);
        currState->music = LoadMusicStream("./sounds/music.mp3");
        SetMusicVolume(currentState.music, currState->musicVol);
        currState->soundsLoaded = true;
    }

    if (setScreen)
    {
        currState->screenRes = (Vector2){.x = WINDOW_WIDTH, .y = WINDOW_HEIGHT};
    }

    // here we create all the object.
    // this objects should be visible during playing status.
    InitObjects(currState);
    InitPlayer(currState);
    InitEnemies(currState);
}

static void ProcessInput(void)
{
    CaptureMouseWithinWindow(&currentState);

    switch (currentState.status)
    {
    case S_playing:
        ProcessPlayerInput(&currentState);
        break;
    case S_menu:
        MenuProcessInput(&menuVolume, &currentState);
        break;
    default:
        if (IsKeyPressed(KEY_SPACE))
        {
            SetInitialGameState(&currentState, false, false);
        }
        break;
    }
}

static void DrawScoreColor(const int score, const Color color, const int x, const int y)
{
    const char *score_text = TextFormat("YOUR SCORE WAS: %2i", score);
    DrawText(score_text, (x - MeasureText(score_text, 20)) / 2, y / 2 - 100, 20, color);
}

static void Render(void)
{
    BeginDrawing();

#ifdef SHOW_FPS
    DrawFPS(0, 0);
#endif

    switch (currentState.status)
    {
    case S_playing:
        ClearBackground(BLACK);
        DrawPlayingGameState(&currentState);
        break;
    case S_lose: {
        // TODO(): All text here is hardcoded and will need to be adjustable to the screen size / ratio.
        const char *youLost = "YOU LOST";

        ClearBackground(BLACK);
        DrawScoreColor(currentState.score, RED, (int)currentState.screenRes.x, (int)currentState.screenRes.y);
        DrawText(youLost, ((int)currentState.screenRes.x - MeasureText(youLost, 60)) / 2,
                 (int)currentState.screenRes.y / 2 - 40, 60, RED);
        DrawText("Press [SPACE] to start again",
                 ((int)currentState.screenRes.x - MeasureText("Press [SPACE] to start again", 30)) / 2,
                 (int)currentState.screenRes.y / 2 + 30, 30, RED);
        break;
    }
    case S_win: {
        // TODO(): All text here is hardcoded and will need to be adjustable to the screen size / ratio.
        const char *youWon = "YOU WON";

        ClearBackground(BLACK);
        DrawScoreColor(currentState.score, GREEN, (int)currentState.screenRes.x, (int)currentState.screenRes.y);
        DrawText(youWon, ((int)currentState.screenRes.x - MeasureText(youWon, 60)) / 2,
                 (int)currentState.screenRes.y / 2 - 40, 60, GREEN);
        DrawText("Press [SPACE] to start again",
                 ((int)currentState.screenRes.x - MeasureText("Press [SPACE] to start again", 30)) / 2,
                 (int)currentState.screenRes.y / 2 + 30, 30, GREEN);
        break;
    }
    case S_menu:
        MenuDraw(&menuVolume, &currentState);
        break;
    }

    EndDrawing();
}

static void UpdateGameState(void)
{
    switch (currentState.status)
    {
    case S_playing:
        if (!IsMusicStreamPlaying(currentState.music))
        {
            PlayMusicStream(currentState.music);
        }
        UpdatePlayingGameState(&currentState);
        break;
    case S_lose:
    case S_win:
        if (IsMusicStreamPlaying(currentState.music))
        {
            PauseMusicStream(currentState.music);
        }
        break;
    default:
        // TraceLog(LOG_INFO, "State %s not implemented", currentState.status);
        break;
    }
}

static void UpdateScreenSize(void)
{
    if (IsWindowResized())
    {
        currentState.screenRes.x = (float)GetScreenWidth();
        currentState.screenRes.y = (float)GetScreenHeight();
    }
}

static void UpdateAndDrawFrame(void)
{
    ProcessInput();
    UpdateGameState();
    UpdateMusicStream(currentState.music);
    UpdateScreenSize();
    Render();
}

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Juego en el tren");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitAudioDevice(); // Initialize audio device
    SetInitialGameState(&currentState, true, true);
    MenuLoadFonts(&menuVolume, MENU_FONT);

    while (!WindowShouldClose())
    {
        // en alguna parte del loop hacer PlaySound()
        UpdateAndDrawFrame();
    }

    // tengo que unload los audios acá UnloadSound()
    UnloadGameFXSoundsAndMusic(&currentState);
    MenuUnloadFont(&menuVolume);
    CloseAudioDevice(); // Close audio device
    CloseWindow();

    return 0;
}

/*
 * TODO(1): Add subtypes of enemies that will have different AI behavior:
 *             - Attacker -> follow the player at reduced speed, to stress
 *              the player aim.
 *             - Ambusher -> Follow the player very slowly (at least half
 *              the speed of the attacker) and accelerate randomly but
 *              keeping a minimum distance. To create a feeling of ambush.
 *             - Dumb -> Just do random movements. Not following any one.
 *
 * TODO(2): Maybe add different types of gameplay, eg; change the way of
 *          keeping the score, like time to eliminate all enemies, or
 *          survival mode if enemies spawn for ever.
 *
 * TODO(3): Make the game available for other platforms, thinking on
 *          Web and mobile. But I would start with web now.
 *
 * TODO(4): Add sound support.
 *
 * TODO(5): Define more clear modules and expose them in their
 *          respective header files, like, player.h, enemies.h, etc. and maybe
 *          implement the logic within the same file by using the stb single
 *          header strategy, this will allow me to define different constants
 *          for prototyping different gameplay, etc behind gates
 *
 * TODO(6): Add a Menu screen to control soundFX and music volume.
 *
 */
