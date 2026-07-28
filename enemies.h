#ifndef H_ENEMIES
#define H_ENEMIES

#define TOTAL_ENEMIES 20
void InitEnemies(GameState *currentState);
#endif

#ifdef H_ENEMIES_IMPLEMENTATION
void InitEnemies(GameState *currentState)
{
    int i;
    // we leave 0 (or NULL) to return not found / failure to get a new object for
    // InitObject()
    for (i = 1; i < TOTAL_ENEMIES + 1 && currentState->totalEnemies <= TOTAL_ENEMIES; ++i)
    {
        Objid objid = InitObject(currentState, T_enemy, (ObjSubType)GetRandomValue(0, 2));
        if (objid == NULL)
            TraceLog(LOG_FATAL, "failed allocating enemy object");

        currentState->objects[objid].radius = (float)GetRandomValue(8, 15);
        currentState->objects[objid].rotation = 0;
        currentState->objects[objid].color = PURPLE;
        currentState->objects[objid].sides = GetRandomValue(1, 10);
        currentState->objects[objid].speedMultiplier = (float)1.1;
        currentState->objects[objid].duration = 2.0;

        // Prevent a new spawn enemy to collide with the player
        SetRandomObjectPosition(currentState, &currentState->objects[objid]);
        while (CheckCollisionBetweenObjects(currentState->player, &currentState->objects[objid]))
        {
            SetRandomObjectPosition(currentState, &currentState->objects[objid]);
        }
    }
}
#endif
