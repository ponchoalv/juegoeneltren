#include "raylib.h"
#include "raymath.h"

#define WINDOW_WIDTH 860
#define WINDOW_HEIGHT 480

#define WINDOW_CENTRE_H WINDOW_WIDTH / 2
#define WINDOW_CENTRE_V WINDOW_HEIGHT / 2

#define MAX_OBJECTS 32000
#define TOTAL_ENEMIES 20
#define BULLETS_VISIBLE_SECONDS 1
#define BULLETS_SPEED 3
// #define TOTAL_RAND_NUMS 25

#define NULL 0

#define SHOW_FPS

typedef enum
{
    T_none,
    T_player,
    T_enemy,
    T_bullet
} ObjType;

typedef struct Object
{
    Vector2 position;
    float rotation;
    ObjType type;
    Color color;
    int sides;
    int radius;
    Vector2 vel;
    double timeVisible;
    int isVisible;
} Object;

typedef enum
{
    S_playing,
    S_lose,
    S_win
} GameState;

typedef int Objid;

// TODO(2026-06-23): Add game state, something to react to WIN, LOSE, PLAYING
typedef struct CurrentState
{
    int totalObjects;
    GameState status;
    Object *player;
    Object objects[MAX_OBJECTS];
    Objid poid;
    int score;
} CurrentState;

CurrentState currentState = {0};


Objid InitObject(ObjType type)
{
    int i;
    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        if (currentState.objects[i].type == T_none)
        {
            currentState.objects[i].type = type;
            ++currentState.totalObjects;
            return i;
        }
    }
    return NULL;
}

void DestroyObject(Objid objid)
{
    if(objid < MAX_OBJECTS)
    {
        currentState.objects[objid].type = T_none;
        --currentState.totalObjects;
    }
}

void InitObjects(void)
{
    int i;
    // 0 is null / not allocated
    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        currentState.objects[i].type = T_none;
    }
}

void FireBullet(void)
{
    Vector2 mousePosition = GetMousePosition();
    Vector2 playerTip = Vector2MoveTowards(currentState.player->position, mousePosition, 25);

    Vector2 vel = {mousePosition.x - currentState.player->position.x, mousePosition.y - currentState.player->position.y};

    Objid boid = InitObject(T_bullet);
    currentState.objects[boid].timeVisible = GetTime() + BULLETS_VISIBLE_SECONDS;
    currentState.objects[boid].position = playerTip;
    currentState.objects[boid].vel = Vector2Multiply(Vector2Normalize(vel), (Vector2){BULLETS_SPEED, BULLETS_SPEED});
    currentState.objects[boid].color = BLUE;
    currentState.objects[boid].radius = 4;
    currentState.objects[boid].sides = 10;
    // TraceLog(LOG_INFO, "bullet fired with (%f, %f) with direction: (%f,%f)", playerTip.x, playerTip.y, player->vel.x,
    // player->vel.y);
}

void InitPlayer(void)
{
    currentState.poid = InitObject(T_player);
    if (currentState.poid == NULL)
        TraceLog(LOG_FATAL, "failed allocating player object");
    currentState.player = &currentState.objects[currentState.poid];
    currentState.player->position.x = WINDOW_CENTRE_H;
    currentState.player->position.y = WINDOW_CENTRE_V;
    currentState.player->color = GREEN;
    currentState.player->rotation = 0;
    currentState.player->sides = 3;
    currentState.player->radius = 20;
    currentState.player->vel = (Vector2){10, 7};
}

// TODO(2026-06-22: quiero hacer que los enemigos que desaparecen lo hagan por 3 segundos)
void InitEnemies(void)
{
    int i;
    for (i = 1; i < TOTAL_ENEMIES; ++i)
    {
        Objid objid = InitObject(T_enemy);
        if (objid == NULL)
            TraceLog(LOG_FATAL, "failed allocating enemy object");

        currentState.objects[objid].radius = GetRandomValue(8, 15);
        currentState.objects[objid].position.x = GetRandomValue(0 + currentState.objects[objid].radius, WINDOW_WIDTH - currentState.objects[objid].radius);
        currentState.objects[objid].position.y = GetRandomValue(0 + currentState.objects[objid].radius, WINDOW_HEIGHT - currentState.objects[objid].radius);
        currentState.objects[objid].rotation = 0;
        currentState.objects[objid].color = PURPLE;
        currentState.objects[objid].sides = GetRandomValue(1, 10);
    }
}

// void InitializeRandNumbs(void)
// {
//     randEnemies = LoadRandomSequence(TOTAL_RAND_NUMS, 1, TOTAL_ENEMIES);
// }

void InitGame(void)
{
    currentState.status = S_playing;
    InitObjects();
    InitPlayer();
    InitEnemies();
}

void ProcessPlayingInput(void)
{
    Vector2 mousePosition = GetMousePosition();
    float dx = mousePosition.x - currentState.player->position.x;
    float dy = mousePosition.y - currentState.player->position.y;
    float rot = atan2f(dy, dx) * RAD2DEG;
    // float rot = -Vector2LineAngle(player->position, mousePosition) * RAD2DEG;
    currentState.player->rotation = rot;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        currentState.player->position.x -= currentState.player->vel.x;
        if (currentState.player->position.x < 0)
        {
            currentState.player->position.x = WINDOW_WIDTH;
        }
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        currentState.player->position.x += currentState.player->vel.x;
        if (currentState.player->position.x > WINDOW_WIDTH)
        {
            currentState.player->position.x = 0;
        }
    }

    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    {
        currentState.player->position.y += currentState.player->vel.y;
        if (currentState.player->position.y > WINDOW_HEIGHT)
        {
            currentState.player->position.y = 0;
        }
    }

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    {
        currentState.player->position.y -= currentState.player->vel.y;
        if (currentState.player->position.y < 0)
        {
            currentState.player->position.y = WINDOW_HEIGHT;
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        FireBullet();
    }
}

void ProcessInput(void)
{
    switch (currentState.status)
    {
    case S_playing:
        ProcessPlayingInput();
        break;
    default:
        // TraceLog(LOG_INFO, "Game State not implemented");
        // TODO(20270623):Implement input processing for S_win and S_lose;
        break;
    }
}


void DrawPlaying(void)
{
    int i;
    DrawText(TextFormat("SCORE: %2i", currentState.score), 90, 0, 20, GREEN);
    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        if (currentState.objects[i].type == T_none)
            continue;
        switch (currentState.objects[i].type)
        {
        case T_enemy:
            DrawPolyLines(currentState.objects[i].position, currentState.objects[i].sides, currentState.objects[i].radius, currentState.objects[i].rotation,
                          currentState.objects[i].color);
            break;

        case T_player: {
            float rotationRad = currentState.objects[i].rotation * DEG2RAD;

            Vector2 litleTriangle = {currentState.objects[i].position.x + cosf(rotationRad) * (currentState.objects[i].radius / 2.0f),
                                     currentState.objects[i].position.y + sinf(rotationRad) * (currentState.objects[i].radius / 2.0f)};
            DrawPolyLines(currentState.objects[i].position, currentState.objects[i].sides, currentState.objects[i].radius, currentState.objects[i].rotation,
                          currentState.objects[i].color);
            DrawPoly(litleTriangle, currentState.objects[i].sides, currentState.objects[i].radius / 2.0f, currentState.objects[i].rotation, RED);
            break;
        }

        case T_bullet:
            DrawPoly(currentState.objects[i].position, currentState.objects[i].sides, currentState.objects[i].radius, currentState.objects[i].rotation, currentState.objects[i].color);
            break;

        default:
            TraceLog(LOG_FATAL, "UNREACHABLE");
        }
    }
}

void Render(void)
{
    BeginDrawing();
        ClearBackground(DARKGRAY);

    #ifdef SHOW_FPS
        DrawFPS(0, 0);
    #endif

        switch (currentState.status)
        {
        case S_playing:
            DrawPlaying();
            break;
        case S_lose:
            DrawText("YOU LOSE", WINDOW_WIDTH/2.0 - MeasureText("YOU WON", 30), WINDOW_HEIGHT/2 - 30, 60, RED);
            break;
        case S_win:
            DrawText("YOU WON", WINDOW_WIDTH/2.0 - MeasureText("YOU WON",30), WINDOW_HEIGHT/2 - 30, 60, GREEN);
            break;
        }

    EndDrawing();
}

void playerUpdate(void)
{
    currentState.player->rotation += 1;
}

void UpdatePlayingGameState(void)
{
    int i = 1;
    int j = 1;
    // int makeObjectInvisible;

    // makeObjectInvisible = GetRandomValue(1, TOTAL_ENEMIES);
    double timeNow = GetTime();

    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        if (i == currentState.poid)
            continue;

        switch (currentState.objects[i].type)
        {
        case T_none:
            continue;
            break;
        case T_enemy:
            currentState.objects[i].rotation += GetRandomValue(-10, 10);
            break;
        case T_bullet:
            currentState.objects[i].position.x += currentState.objects[i].vel.x;
            currentState.objects[i].position.y += currentState.objects[i].vel.y;

            // destroy the bullet
            if (currentState.objects[i].timeVisible < timeNow)
            {
                DestroyObject(i);
            }
            for (j = 1; j < MAX_OBJECTS; ++j)
            {
                if (j == i)
                    continue;
                if ((currentState.objects[j].type == T_enemy) &&
                    CheckCollisionCircles(currentState.objects[i].position, currentState.objects[i].radius, currentState.objects[j].position,
                                          currentState.objects[j].radius))
                {
                    DestroyObject(i);
                    DestroyObject(j);
                    ++currentState.score;
                    break;
                }
            }

            // TraceLog(LOG_INFO, "currentState.totalObjects %d", currentState.totalObjects);
            if (currentState.totalObjects == 1)
            {
                currentState.status = S_win;
            }
            break;
        default:
            continue;
        }
    }
}

void UpdateGameState(void)
{
    switch (currentState.status)
    {
    case S_playing:
        UpdatePlayingGameState();
        break;
    default:
        // TraceLog(LOG_INFO, "State %s not implemented", currentState.status);
        break;
    }
}

void UpdateAndDrawFrame(void)
{
    ProcessInput();
    UpdateGameState();
    Render();
}

void TestRandNumbers(void)
{
    int i;
    int *numbers = LoadRandomSequence(10, 0, 100);
    for (i = 0; i < 10; ++i)
    {
        TraceLog(LOG_INFO, "The random number at %d is %d", i, numbers[i]);
    }
}

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Juego en el tren");
    SetTargetFPS(60);
    InitGame();

    while (!WindowShouldClose())
    {
        UpdateAndDrawFrame();
    }

    CloseWindow();

    return 0;
}
