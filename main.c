#include "raylib.h"
#include "raymath.h"

#define WINDOW_WIDTH 860
#define WINDOW_HEIGHT 480

#define WINDOW_CENTRE_H WINDOW_WIDTH / 2
#define WINDOW_CENTRE_V WINDOW_HEIGHT / 2

#define MAX_OBJECTS 32000
#define OBJ_SIZE 25
#define TOTAL_ENEMIES 16
// #define TOTAL_RAND_NUMS 25

#define NULL 0

#define SHOW_FPS

typedef enum
{
    T_none,
    T_player,
    T_enemy
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

typedef int Objid;

Object objects[MAX_OBJECTS];
Object *player;
Objid poid = 0;

int totalObjects = 1;
// int *randEnemies;

Objid InitObject(ObjType type)
{
    int i;
    for(i = 1; i < totalObjects && i < MAX_OBJECTS; ++i)
    {
        if (objects[i].type == T_none)
        {
            objects[i].type = type;
            return i;
        }
    }
    return NULL;
}

void InitObjects(void)
{
    int i;
    // 0 is null / not allocated
    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        totalObjects++;
        objects[i].type = T_none;
    }
}

void InitPlayer(void)
{
    poid = InitObject(T_player);
    if (poid == NULL) TraceLog(LOG_FATAL, "failed allocating player object");
    player = &objects[poid];
    player->position.x = WINDOW_CENTRE_H;
    player->position.y = WINDOW_CENTRE_V;
    player->color = GREEN;
    player->rotation = 0;
    player->sides = 3;
    player->radius = 20;
    player->vel = (Vector2){15, 10};
}


// TODO(2026-06-22: quiero hacer que los enemigos que desaparecen lo hagan por 3 segundos)
void InitEnemies(void)
{
    int i;
    for (i = 1; i < TOTAL_ENEMIES; ++i)
    {
        Objid objid = InitObject(T_enemy);
        if (objid == NULL) TraceLog(LOG_FATAL, "failed allocating enemy object");

        objects[objid].position.x = GetRandomValue(0 + OBJ_SIZE, WINDOW_WIDTH - OBJ_SIZE);
        objects[objid].position.y = GetRandomValue(0 + OBJ_SIZE, WINDOW_HEIGHT - OBJ_SIZE);
        objects[objid].rotation = 0;
        objects[objid].color = PURPLE;
        objects[objid].sides = GetRandomValue(1, 10);
        objects[objid].radius = 15;
        objects[objid].vel = (Vector2){10, 5};
        objects[objid].isVisible = 1;
        objects[objid].timeVisible = 0;
    }
}

// void InitializeRandNumbs(void)
// {
//     randEnemies = LoadRandomSequence(TOTAL_RAND_NUMS, 1, TOTAL_ENEMIES);
// }

void ProcessInput(void)
{
    Vector2 mousePosition = GetMousePosition();
    float dx = mousePosition.x - player->position.x;
    float dy = mousePosition.y - player->position.y;
    float rot  = atan2f(dy, dx) * RAD2DEG;
    // float rot = -Vector2LineAngle(player->position, mousePosition) * RAD2DEG;
    player->rotation = rot;

    if(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        player->position.x -= player->vel.x;
        if (player->position.x < 0)
        {
            player->position.x = WINDOW_WIDTH;
        }
    }

    if(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        player->position.x += player->vel.x;
        if (player->position.x > WINDOW_WIDTH)
        {
            player->position.x = 0;
        }
    }

    if(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        player->position.y += player->vel.y;
        if(player->position.y > WINDOW_HEIGHT)
        {
            player->position.y = 0;
        }
    }

    if(IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        player->position.y -= player->vel.y;
        if(player->position.y < 0)
        {
            player->position.y = WINDOW_HEIGHT;
        }
    }
}

void Render(void)
{
    int i;


    BeginDrawing();
        ClearBackground(DARKGRAY);

    #ifdef SHOW_FPS
        DrawFPS(0, 0);
    #endif

        for (i = 1; i < totalObjects; ++i)
        {
            if (objects[i].type == T_none)
                continue;
            switch (objects[i].type) {
            case T_enemy:
                DrawPolyLines(objects[i].position, objects[i].sides, objects[i].radius, objects[i].rotation, objects[i].color);
                break;
            case T_player: {
                float rotationRad = objects[i].rotation * DEG2RAD;

                Vector2 litleTriangle = {
                    objects[i].position.x + cosf(rotationRad) * (objects[i].radius / 2.0f),
                    objects[i].position.y + sinf(rotationRad) * (objects[i].radius / 2.0f)
                };
                DrawPolyLines(objects[i].position, objects[i].sides, objects[i].radius, objects[i].rotation, objects[i].color);
                DrawPoly(litleTriangle, objects[i].sides, objects[i].radius / 2.0f, objects[i].rotation, RED);
                break;
            }
        default:
            continue;
            }
        }

    EndDrawing();
}

void playerUpdate(void)
{
    player->rotation += 1;
}

void Ai(void)
{
    int i = 1;
    // int makeObjectInvisible;

    // makeObjectInvisible = GetRandomValue(1, TOTAL_ENEMIES);
    // double timeNow = GetTime();

    for (i = 1; i < totalObjects; ++i)
    {
        if (i == poid) continue;

        objects[i].rotation += GetRandomValue(-10, 10);
        // if (!objects[i].isVisible && objects[i].timeVisible < timeNow)
        // {
        //     objects[i].isVisible = 1;
        //     objects[i].type = T_enemy;
        // }

        // if (i == makeObjectInvisible)
        // {
        //     objects[i].isVisible = 0;
        //     objects[i].timeVisible = timeNow + 0.3;
        //     objects[i].type = T_none;
        // }
    }
}

void UpdateAndDrawFrame(void)
{
    ProcessInput();
    Ai();
    Render();
}

void TestRandNumbers(void)
{
    int i;
    int *numbers = LoadRandomSequence(10, 0, 100);
    for(i = 0; i < 10; ++i)
    {
        TraceLog(LOG_INFO, "The random number at %d is %d", i, numbers[i]);
    }
}


int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Juego en el tren");
    // SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    // TestRandNumbers();

    // InitializeRandNumbs();
    InitObjects();
    InitPlayer();
    InitEnemies();

    while (!WindowShouldClose())
    {
        UpdateAndDrawFrame();
    }

    CloseWindow();

    return 0;
}
