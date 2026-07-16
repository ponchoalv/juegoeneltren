#ifndef H_PLAYER
#define H_PLAYER

#define PLAYER_SPEED 0.5f

void FireBullet(CurrentState *currentState, double duration);
void InitPlayer(CurrentState *currentState);
void ProcessPlayerInput(CurrentState *currentState);
#endif

#ifdef H_PLAYER_IMPLEMENTATION 
// I think we need to re-think this to also take into account the current speed of the player
// TODO(player): Tale into account the currnt speed to calculate the speed the bullet will have once fired (vel).
// Maybe I solve the above TODO, I need to make some testing on top of this
void FireBullet(CurrentState *currentState, double duration)
{
    const Vector2 mousePosition = GetMousePosition();
    const Vector2 playerTip = Vector2MoveTowards(currentState->player->position, mousePosition, 25);

    Vector2 vel = GetOrientationVector(currentState->player->position, mousePosition);

    const Objid boid = InitObject(currentState, T_bullet, OS_none);
    currentState->objects[boid].duration = duration;
    currentState->objects[boid].timeVisible = GetTime() + currentState->objects[boid].duration;
    currentState->objects[boid].position = playerTip;
    currentState->objects[boid].speedMultiplier = (float)3.0;
    currentState->objects[boid].vel = Vector2Multiply(
        vel, (Vector2){currentState->objects[boid].speedMultiplier + fabsf(currentState->player->vel.x),
                       currentState->objects[boid].speedMultiplier + fabsf(currentState->player->vel.y)});
    currentState->objects[boid].color = BLUE;
    currentState->objects[boid].radius = 4;
    currentState->objects[boid].sides = 10;
}

void InitPlayer(CurrentState *currentState)
{
    currentState->poid = InitObject(currentState, T_player, OS_none);
    if (currentState->poid == NULL)
        TraceLog(LOG_FATAL, "failed allocating player object");
    currentState->player = &currentState->objects[currentState->poid];
    currentState->player->position.x = WINDOW_CENTRE_H;
    currentState->player->position.y = WINDOW_CENTRE_V;
    currentState->player->color = GREEN;
    currentState->player->rotation = 0;
    currentState->player->sides = 3;
    currentState->player->radius = 20;
    currentState->player->vel = (Vector2){0, 0};
}

void ProcessPlayerInput(CurrentState *currentState)
{
    const Vector2 mousePosition = GetMousePosition();
    const float dx = mousePosition.x - currentState->player->position.x;
    const float dy = mousePosition.y - currentState->player->position.y;
    Vector2 dirToMouse = GetOrientationVector(currentState->player->position, mousePosition);

    // add a bit of constant speed
    dirToMouse = Vector2Scale(dirToMouse, PLAYER_SPEED);
    currentState->player->rotation = atan2f(dy, dx) * RAD2DEG;

    if (IsKeyDown(KEY_SPACE))
    {
        currentState->player->vel = Vector2Add(currentState->player->vel, dirToMouse);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        FireBullet(currentState, 1.0);
    }
}

#endif

// I think we should add the player controller states.
// Draw(), Input(), FireBullet() (maybe), Collision against enemies? or maybe should be part of how the enemy updates
// the word? UpdateCurrentState() (but for the player bits?)
// trying to figure out what to do with the state
