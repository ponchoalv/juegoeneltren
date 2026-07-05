#ifndef _H_GAME_STATE_
#define _H_GAME_STATE_
#define MAX_OBJECTS 32000

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

#endif
