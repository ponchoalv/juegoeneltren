#ifndef _H_GAME_STATE_
#define _H_GAME_STATE_
#define MAX_OBJECTS 32000
#define NULL 0

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

#endif
