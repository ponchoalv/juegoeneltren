#ifndef _H_GAME_STATE_
#define _H_GAME_STATE_

#define MAX_OBJECTS 32000
#define NULL 0

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 720

#define WINDOW_CENTRE_H WINDOW_WIDTH / 2
#define WINDOW_CENTRE_V WINDOW_HEIGHT / 2

#define MOUSE_MARGIN 30

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
    int radius;
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
    S_win
} GameState;

typedef int Objid;

typedef struct CurrentState
{
    int activeObjects;
    GameState status;
    Object *player;
    Object objects[MAX_OBJECTS];
    Objid poid;
    int score;
    int totalEnemies;
} CurrentState;


// TODO(gamestate): Maybe I should have the prototype vs implementation under a constant if we I would like to tests different versions?
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
            currentState->objects[i].vel = (Vector2){0,0};
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
    obj->position.x =
        GetRandomValue(0 + obj->radius, WINDOW_WIDTH - obj->radius);
    obj->position.y =
        GetRandomValue(0 + obj->radius, WINDOW_HEIGHT - obj->radius);
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
    int x = GetMouseX();
    int y = GetMouseY();

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

void UpdateStateWithCollisions(CurrentState *currentState, Objid objid, double timeNow)
{
    int j;

    // TODO(gamestate): Move this out to gamestate.
    if (currentState->objects[objid].type == T_none) return;

    for (j = objid + 1; j < MAX_OBJECTS; j++)
    {
        Object *otherObject = &currentState->objects[j];
        if (CheckCollisionBetweenObjects(currentState->objects[objid], *otherObject))
        {
            // TODO(enemy): Maybe this should be moved to a switch case and then the enemy specific logic to a procedure in enemies.h
            if (currentState->objects[objid].type == T_enemy)
            {
                Object *enemy = &currentState->objects[objid];
                // Enemy -> Enemy x
                if (otherObject->type == T_enemy)
                {
                    // we need to tell the object had collied and
                    // the ammount of time we want him to be in a
                    // different trajectory than the default one
                    // (chasing the player)
                    enemy->isColliding = true;
                    enemy->timeVisible = timeNow + enemy->duration;

                    Vector2 normal =
                        Vector2Normalize(Vector2Subtract(enemy->position, currentState->objects[j].position));
                    Vector2 relativeVel = Vector2Subtract(enemy->vel, currentState->objects[j].vel);
                    Vector2 reflctVel = Vector2Reflect(relativeVel, normal);

                    enemy->vel = Vector2Scale(reflctVel, -ENEMY_COLLISION_REFLECT_SCALE);
                    currentState->objects[j].vel = Vector2Scale(reflctVel, ENEMY_COLLISION_REFLECT_SCALE);
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
                    DestroyObject(currentState, j);
                    DestroyObject(currentState, objid);
                    ++currentState->score;
                }
            }
        }
    }
}

#endif
