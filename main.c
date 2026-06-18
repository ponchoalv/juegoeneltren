#include "raylib.h"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

#define WINDOW_CENTRE_H WINDOW_WIDTH / 2
#define WINDOW_CENTRE_V WINDOW_HEIGHT / 2

#define MAX_OBJECTS 32000
#define OBJ_SIZE 25
#define TOTAL_ENEMIES 16

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
} Object;

Object objects[MAX_OBJECTS];
Object *player;
const int poid = 0;
int totalObjects = TOTAL_ENEMIES + 1;

void initObjects()
{
    int i;
    for (i = 0; i < MAX_OBJECTS; ++i)
    {
        objects[i].type = T_none;
    }
}

void initPlayer()
{
    player->position.x = WINDOW_CENTRE_H;
    player->position.y = WINDOW_CENTRE_V;
    player->type = T_player;
    player->color = GREEN;
    player->rotation = 0;
    player->sides = 3;
    player->radius = 20;
}

void initEnemies()
{
    int i;
    for (i = 1; i < TOTAL_ENEMIES; ++i)
    {
        objects[i].position.x = GetRandomValue(0 + OBJ_SIZE, WINDOW_WIDTH - OBJ_SIZE);
        objects[i].position.y = GetRandomValue(0 + OBJ_SIZE, WINDOW_HEIGHT - OBJ_SIZE);
        objects[i].rotation = 0;
        objects[i].type = T_enemy;
        objects[i].color = RED;
        objects[i].sides = 7;
        objects[i].radius = 15;
    }
}

void render()
{
    int i;
#ifdef SHOW_FPS
    DrawFPS(0, 0);
#endif

    for (i = 0; i < totalObjects; ++i)
    {
        if (objects[i].type == T_none)
            continue;
        DrawPolyLines(objects[i].position, objects[i].sides, objects[i].radius, objects[i].rotation, objects[i].color);
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
        if (i == removeRandomObject)
            objects[i].type = T_none;
        if (i == showRandomObject)
            objects[i].type = T_enemy;
        objects[i].rotation += GetRandomValue(-10, 10);
    }
}

int main()
{
    player = &objects[poid];

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Juego en el tren");
    SetTargetFPS(60);

    initObjects();
    initPlayer();
    initEnemies();

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground((Color){50, 100, 200, 200});
            playerUpdate();
            ai();
            render();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
