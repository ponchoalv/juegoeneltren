#ifndef H_GAME_STATE
#define H_GAME_STATE

#include <raylib.h>
#include <raymath.h>

#define MAX_OBJECTS 1024
#define NULL 0
// #define B_DEBUG

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 720

#define WINDOW_CENTRE_H (WINDOW_WIDTH / 2.0)
#define WINDOW_CENTRE_V (WINDOW_HEIGHT / 2.0)

#define MOUSE_MARGIN 30

#define ENEMY_COLLISION_REFLECT_SCALE 0.65f

// #define NO_LOSE
typedef enum
{
    SO_bullet,
    SO_collide,
    SO_total
} SoundTs;

typedef enum
{
    T_none,
    T_player,
    T_enemy,
    T_bullet
} ObjType;

typedef enum
{
    OS_none,
    OS_attacker,
    OS_camper
} ObjSubType;

typedef struct Object
{
    Vector2 position;
    float rotation;
    ObjType type;
    Color color;
    int sides;
    float radius;
    Vector2 vel;
    bool isColliding;
    double timeVisible;
    double duration;
    float speedMultiplier;
    int isVisible;
    ObjSubType subType;
} Object;

typedef enum
{
    S_playing,
    S_lose,
    S_menu,
    S_win
} GameState;

typedef int Objid;

typedef struct CurrentState
{
    int activeObjects;
    Sound sounds[SO_total];
    Music music;
    GameState status;
    Object *player;
    Object objects[MAX_OBJECTS];
    Objid poid;
    int score;
    int totalEnemies;
    bool soundsLoaded;
} CurrentState;

Objid InitObject(CurrentState *currentState, ObjType type, ObjSubType subType);
void DestroyObject(CurrentState *currentState, Objid objid);
void InitObjects(CurrentState *currentState);
void WrapObjectPosition(Object *obj);
void SetRandomObjectPosition(Object *obj);
int CheckCollisionBetweenObjects(Object a, Object b);
Vector2 GetOrientationVector(Vector2 from, Vector2 to);
void SetObjectDirAndSpeed(Object *obj, Vector2 to);
void CaptureMouseWithinWindow(void);
void UnloadGameSounds(CurrentState *currentState);
void UpdateStateWithCollisions(CurrentState *currentState, Objid objid, double timeNow);
void DrawPlayingGameState(CurrentState *currentState);
void UpdatePlayingGameState(CurrentState *currentState);
#endif

#ifdef H_GAME_STATE_IMPLEMENTATION
// TODO(gamestate): Maybe I should have the prototype vs implementation under a constant if we I would like to tests
// different versions?
Objid InitObject(CurrentState *currentState, ObjType type, ObjSubType subType)
{
    int i;
    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        if (currentState->objects[i].type == T_none)
        {
            currentState->objects[i].type = type;
            currentState->objects[i].subType = subType;
            currentState->objects[i].isColliding = false;
            currentState->objects[i].duration = 0.0;
            currentState->objects[i].vel = (Vector2){0, 0};
            ++currentState->activeObjects;
            if (type == T_enemy)
                ++currentState->totalEnemies;
            return i;
        }
    }
    return NULL;
}

void DestroyObject(CurrentState *currentState, Objid objid)
{
    if (objid < MAX_OBJECTS)
    {
        if (currentState->objects[objid].type == T_enemy)
            --currentState->totalEnemies;
        currentState->objects[objid].type = T_none;
        --currentState->activeObjects;
    }
}

void InitObjects(CurrentState *currentState)
{
    int i;
    // 0 is null / not allocated
    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        currentState->objects[i].type = T_none;
    }
}

void WrapObjectPosition(Object *obj)
{
    if (obj->position.x < 0)
        obj->position.x = WINDOW_WIDTH;

    if (obj->position.x > WINDOW_WIDTH)
        obj->position.x = 0;

    if (obj->position.y > WINDOW_HEIGHT)
        obj->position.y = 0;

    if (obj->position.y < 0)
        obj->position.y = WINDOW_HEIGHT;
}

void SetRandomObjectPosition(Object *obj)
{
    obj->position.x = (float)GetRandomValue(0 + (int)obj->radius, WINDOW_WIDTH - obj->radius);
    obj->position.y = (float)GetRandomValue(0 + (int)obj->radius, WINDOW_HEIGHT - obj->radius);
}

int CheckCollisionBetweenObjects(Object a, Object b)
{
    return (a.type != T_none && b.type != T_none) && CheckCollisionCircles(a.position, a.radius, b.position, b.radius);
}

Vector2 GetOrientationVector(Vector2 from, Vector2 to)
{
    return Vector2Normalize(((Vector2){to.x - from.x, to.y - from.y}));
}

void SetObjectDirAndSpeed(Object *obj, Vector2 to)
{
    obj->vel =
        Vector2Multiply(GetOrientationVector(obj->position, to), (Vector2){obj->speedMultiplier, obj->speedMultiplier});
}

void CaptureMouseWithinWindow(void)
{
    // added an extra 5 pixels to prevent the mouse to bounce out of
    // the window, not sure if this the right thing to do, was the
    // simplest work around I found
    const int x = GetMouseX();
    const int y = GetMouseY();

    // this logic to capture the mouse is not working properly and make gameplay a bit awkward
    if (WINDOW_WIDTH - MOUSE_MARGIN < x)
        SetMousePosition(WINDOW_WIDTH - MOUSE_MARGIN, y);
    if (MOUSE_MARGIN >= x)
        SetMousePosition(MOUSE_MARGIN, y);
    if (MOUSE_MARGIN >= y)
        SetMousePosition(x, MOUSE_MARGIN);
    if (WINDOW_HEIGHT - MOUSE_MARGIN <= y)
        SetMousePosition(x, WINDOW_HEIGHT - MOUSE_MARGIN);
}

void UnloadGameSounds(CurrentState *currentState)
{
    if (currentState->soundsLoaded)
    {
        for (int i = 0; i < SO_total; i++)
        {
            UnloadSound(currentState->sounds[i]);
        }
        UnloadMusicStream(currentState->music);
    }
}

void UpdateStateWithCollisions(CurrentState *currentState, Objid objid, double timeNow)
{
    int j;

    // TODO(gamestate): Move this out to gamestate.
    if (currentState->objects[objid].type == T_none)
        return;

    for (j = objid + 1; j < MAX_OBJECTS; j++)
    {
        const Object *otherObject = &currentState->objects[j];
        if (CheckCollisionBetweenObjects(currentState->objects[objid], *otherObject))
        {
            // TODO(enemy): Maybe this should be moved to a switch case and then the enemy specific logic to a procedure
            // in enemies.h
            if (currentState->objects[objid].type == T_enemy)
            {
                Object *enemy = &currentState->objects[objid];
                // Enemy -> Enemy x
                if (otherObject->type == T_enemy)
                {
                    // we need to tell the object had collied and
                    // the amount of time we want him to be in a
                    // different trajectory than the default one
                    // (chasing the player)
                    enemy->isColliding = true;
                    enemy->timeVisible = timeNow + enemy->duration;

                    Vector2 normal =
                        Vector2Normalize(Vector2Subtract(enemy->position, currentState->objects[j].position));
                    Vector2 relativeVel = Vector2Subtract(enemy->vel, currentState->objects[j].vel);
                    Vector2 reflectVel = Vector2Reflect(relativeVel, normal);

                    enemy->vel = Vector2Scale(reflectVel, -ENEMY_COLLISION_REFLECT_SCALE);
                    currentState->objects[j].vel = Vector2Scale(reflectVel, ENEMY_COLLISION_REFLECT_SCALE);
                }
                else if (otherObject->type == T_player)
                {
#ifndef NO_LOSE
                    // Enemy -> payer
                    currentState->status = S_lose;
#endif
                }
                else if (otherObject->type == T_bullet)
                {
                    // Enemy -> Bullet
#ifdef B_DEBUG
                    TraceLog(LOG_INFO, "bullet impacted Enemy->Bullet");
#endif
                    PlaySound(currentState->sounds[SO_collide]);
                    DestroyObject(currentState, j);
                    DestroyObject(currentState, objid);
                    ++currentState->score;
                }
            }
            // TODO(player.h): we should move this to a player file maybe?
            else if (currentState->objects[objid].type == T_player)
            {
                // Player -> Enemy
                if (otherObject->type == T_enemy)
                {
#ifndef NO_LOSE
                    currentState->status = S_lose;
#endif
                }
            }
            // TODO(player): we might need to move this to player.h maybe? not sure though
            else if (currentState->objects[objid].type == T_bullet)
            {
                // Bullet -> Enemy
                if (otherObject->type == T_enemy)
                {
#ifdef B_DEBUG
                    TraceLog(LOG_INFO, "bullet impacted Bullet->Enemy");
#endif
                    PlaySound(currentState->sounds[SO_collide]);
                    DestroyObject(currentState, j);
                    DestroyObject(currentState, objid);
                    ++currentState->score;
                }
            }
        }
    }
}

void DrawPlayingGameState(CurrentState *currentState)
{
    int i;
    DrawText(TextFormat("SCORE: %2i", currentState->score), 90, 0, 20, GREEN);
    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        if (currentState->objects[i].type == T_none)
            continue;

        // Both T_enemies and T_player will draw the same thing here:
        DrawPolyLines(currentState->objects[i].position, currentState->objects[i].sides, currentState->objects[i].radius,
                      currentState->objects[i].rotation, currentState->objects[i].color);
        switch (currentState->objects[i].type)
        {
        case T_player:
        {
            const float rotationRad = currentState->objects[i].rotation * DEG2RAD;

            const Vector2 littleTriangle = {
                currentState->objects[i].position.x + cosf(rotationRad) * (currentState->objects[i].radius / 2.0f),
                currentState->objects[i].position.y + sinf(rotationRad) * (currentState->objects[i].radius / 2.0f)};

            DrawPoly(littleTriangle, currentState->objects[i].sides, currentState->objects[i].radius / 2.0f,
                     currentState->objects[i].rotation, RED);
            break;
        }

        case T_bullet:
            DrawPoly(currentState->objects[i].position, currentState->objects[i].sides, currentState->objects[i].radius,
                     currentState->objects[i].rotation, currentState->objects[i].color);
            break;

        default:
            break;
        }
    }
}

void MoveObject(Object *obj)
{
    obj->position.x += obj->vel.x;
    obj->position.y += obj->vel.y;
}

void UpdatePlayingGameState(CurrentState *currentState)
{
    int i = 1;
    const double timeNow = GetTime();

    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        UpdateStateWithCollisions(currentState, i, timeNow);

        // TODO(gamestate): this should be moved to gamestate.h (along side with this methods)
        MoveObject(&currentState->objects[i]);
        WrapObjectPosition(&currentState->objects[i]);

        switch (currentState->objects[i].type)
        {
        case T_enemy:
        {
            // TODO(enemies): should move this to enemies.h I think
            Object *enemy = &currentState->objects[i];
            // Todo implement proper AI / logic to move and attack the player
            // this is not a good experience, we need to find a way to make it
            // feel more real, now is like converging all T_enemies attacker into one point
            enemy->rotation += (float)GetRandomValue(-10, 10);

            bool enemyNotCollidingOrVisibilityTimeOut = (!enemy->isColliding || !(enemy->timeVisible >= GetTime()));
            // when we are not colliding we make sure is being set
            // state to not colliding and also that the attackers
            // are chasing the player.
            if (enemy->subType == OS_attacker && enemyNotCollidingOrVisibilityTimeOut)
            {
                enemy->isColliding = false;
                // WIP: testing if adding some random scalar to the player position would make it more fun
                SetObjectDirAndSpeed(enemy, Vector2Scale(currentState->player->position, (float)GetRandomValue(1, 2)));
            }
            break;
        }
        case T_bullet:
            // TODO(player): we should move this to the player.h
            // destroy the bullet
            if (currentState->objects[i].timeVisible < timeNow)
            {
                DestroyObject(currentState, i);
            }

            // TraceLog(LOG_INFO, "currentState.activeObjects %d",
            // currentState.activeObjects);w
            break;
        default:
            continue;
        }
    }

    if (currentState->totalEnemies == 0)
    {
        currentState->status = S_win;
    }
}
#endif
