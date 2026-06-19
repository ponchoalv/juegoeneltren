#include "raylib.h"
#include "raymath.h"

#define WINDOW_WIDTH 860
#define WINDOW_HEIGHT 480

#define WINDOW_CENTRE_H WINDOW_WIDTH / 2
#define WINDOW_CENTRE_V WINDOW_HEIGHT / 2

#define MAX_OBJECTS 32000
#define OBJ_SIZE 25
#define TOTAL_ENEMIES 16

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
    float speed;
} Object;

typedef int Objid;

Object objects[MAX_OBJECTS];
Object *player;
Objid poid = 0;

int totalObjects = 1;

Objid initObject(ObjType type)
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

void initObjects()
{
    int i;
    // 0 is null / not allocated
    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        totalObjects++;
        objects[i].type = T_none;
    }
}

void initPlayer()
{
    poid = initObject(T_player);
    if (poid == NULL) TraceLog(LOG_FATAL, "failed allocating player object");
    player = &objects[poid];
    player->position.x = WINDOW_CENTRE_H;
    player->position.y = WINDOW_CENTRE_V;
    player->color = GREEN;
    player->rotation = 0;
    player->sides = 3;
    player->radius = 20;
    player->speed = 1.5;
}

void initEnemies()
{
    int i;
    for (i = 1; i < TOTAL_ENEMIES; ++i)
    {
        Objid objid = initObject(T_enemy);
        if (objid == NULL) TraceLog(LOG_FATAL, "failed allocating enemy object");

        objects[objid].position.x = GetRandomValue(0 + OBJ_SIZE, WINDOW_WIDTH - OBJ_SIZE);
        objects[objid].position.y = GetRandomValue(0 + OBJ_SIZE, WINDOW_HEIGHT - OBJ_SIZE);
        objects[objid].rotation = 0;
        objects[objid].color = PURPLE;
        objects[objid].sides = 5;
        objects[objid].radius = 15;
        objects[objid].speed = 0.1;
    }
}

void processInput()
{
    Vector2 mousePosition = GetMousePosition();
    float dx = mousePosition.x - player->position.x;
    float dy = mousePosition.y - player->position.y;
    float rot  = atan2f(dy, dx) * RAD2DEG;
    player->rotation = rot;

    if(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        player->position.x -= player->speed;
    }

    if(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        player->position.x += player->speed;
    }

    if(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        player->position.y += player->speed;
    }

    if(IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        player->position.y -= player->speed;
    }
}

void render()
{
    int i;
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
}

void playerUpdate()
{
    player->rotation += 1;
}

void ai()
{
    int i = 1;
    int removeRandomObject, showRandomObject;
    removeRandomObject = GetRandomValue(1, TOTAL_ENEMIES);
    showRandomObject = GetRandomValue(1, TOTAL_ENEMIES);

    for (i = 1; i < totalObjects; ++i)
    {
        if (i == poid) continue;
        if (i == removeRandomObject)
            objects[i].type = T_none;
        if (i == showRandomObject)
            objects[i].type = T_enemy;
        objects[i].rotation += GetRandomValue(-10, 10);
    }
}

int main()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Juego en el tren");
    // SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    initObjects();
    initPlayer();
    initEnemies();

    while (!WindowShouldClose())
    {
        processInput();
        BeginDrawing();
            ClearBackground(DARKGRAY);
            // playerUpdate();
            processInput();
            ai();
            render();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
