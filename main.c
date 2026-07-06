#include <raylib.h>
#include <raymath.h>

#include "gamestate.h"

#define WINDOW_CENTRE_H WINDOW_WIDTH / 2
#define WINDOW_CENTRE_V WINDOW_HEIGHT / 2

#define TOTAL_ENEMIES 20
#define ENEMY_COLLISION_REFLECT_SCALE 0.55f
#define PLAYER_SPEED 0.5f
#define MOUSE_MARGIN 30

#define SHOW_FPS

CurrentState currentState = {0};

int CheckCollisionBetweenObjects(Object a, Object b)
{
    return (a.type != T_none && b.type != T_none) && CheckCollisionCircles(a.position, a.radius, b.position, b.radius);
}

Vector2 GetOrientationVector(Vector2 from, Vector2 to)
{
    return Vector2Normalize(((Vector2){to.x - from.x, to.y - from.y}));
}


// I think we need to re-think this to also take into account the current speed of the player
// TODO(player): Tale into account the currnt speed to calculate the speed the bullet will have once fired (vel).
void FireBullet(double duration)
{
    Vector2 mousePosition = GetMousePosition();
    Vector2 playerTip = Vector2MoveTowards(currentState.player->position, mousePosition, 25);

    Vector2 vel = GetOrientationVector(currentState.player->position, mousePosition);

    Objid boid = InitObject(&currentState, T_bullet, OS_none);
    currentState.objects[boid].duration = duration;
    currentState.objects[boid].timeVisible = GetTime() + currentState.objects[boid].duration;
    currentState.objects[boid].position = playerTip;
    currentState.objects[boid].speedMultiplier = 3.0;
    currentState.objects[boid].vel = Vector2Multiply(
        vel, (Vector2){currentState.objects[boid].speedMultiplier, currentState.objects[boid].speedMultiplier});
    currentState.objects[boid].color = BLUE;
    currentState.objects[boid].radius = 4;
    currentState.objects[boid].sides = 10;
}

void InitPlayer(void)
{
    currentState.poid = InitObject(&currentState, T_player, OS_none);
    if (currentState.poid == NULL)
        TraceLog(LOG_FATAL, "failed allocating player object");
    currentState.player = &currentState.objects[currentState.poid];
    currentState.player->position.x = WINDOW_CENTRE_H;
    currentState.player->position.y = WINDOW_CENTRE_V;
    currentState.player->color = GREEN;
    currentState.player->rotation = 0;
    currentState.player->sides = 3;
    currentState.player->radius = 20;
    currentState.player->vel = (Vector2){0, 0};
}

void InitEnemies(void)
{
    int i;
    // we leave 0 (or NULL) to return not found / failure to get a new object for
    // InitObject()
    for (i = 1; i < TOTAL_ENEMIES + 1 && currentState.totalEnemies <= TOTAL_ENEMIES; ++i)
    {
        Objid objid = InitObject(&currentState, T_enemy, (ObjSubType)GetRandomValue(0, 2));
        if (objid == NULL)
            TraceLog(LOG_FATAL, "failed allocating enemy object");

        currentState.objects[objid].radius = GetRandomValue(8, 15);
        currentState.objects[objid].rotation = 0;
        currentState.objects[objid].color = PURPLE;
        currentState.objects[objid].sides = GetRandomValue(1, 10);
        currentState.objects[objid].speedMultiplier = 1.1;
        currentState.objects[objid].duration = 2.0;

        // Prevent a newly spawn enemy to collide with the player
        SetRandomObjectPosition(&currentState.objects[objid]);
        while (CheckCollisionBetweenObjects(*currentState.player, currentState.objects[objid]))
        {
            SetRandomObjectPosition(&currentState.objects[objid]);
        }
    }
}

void SetInitialGameState(CurrentState *currentState)
{
    // first we reset the state of the game to be in playing mode.
    // then we set the counters to the initial status.
    currentState->status = S_playing;
    currentState->activeObjects = 0;
    currentState->score = 0;
    currentState->totalEnemies = 0;

    // here we create all the object.
    // this objects should be visible during playing status.
    InitObjects(currentState);
    InitPlayer();
    InitEnemies();
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

void ProcessPlayingInput(void)
{
    Vector2 mousePosition = GetMousePosition();
    float dx = mousePosition.x - currentState.player->position.x;
    float dy = mousePosition.y - currentState.player->position.y;
    float rot = atan2f(dy, dx) * RAD2DEG;
    Vector2 dirToMouse = GetOrientationVector(currentState.player->position, mousePosition);
    dirToMouse = Vector2Scale(dirToMouse, PLAYER_SPEED);

    currentState.player->rotation = rot;

    if (IsKeyDown(KEY_SPACE))
    {
        currentState.player->vel = Vector2Add(currentState.player->vel, dirToMouse);
    }

    WrapObjectPosition(currentState.player);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        FireBullet(1.0);
    }
}

void ProcessInput(void)
{
    CaptureMouseWithinWindow();

    switch (currentState.status)
    {
    case S_playing:
        ProcessPlayingInput();
        break;
    default:
        if (IsKeyPressed(KEY_SPACE))
        {
            SetInitialGameState(&currentState);
        }
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

        // Both T_enemies and T_player will draw the same thing here:
        DrawPolyLines(currentState.objects[i].position, currentState.objects[i].sides, currentState.objects[i].radius,
                      currentState.objects[i].rotation, currentState.objects[i].color);
        switch (currentState.objects[i].type)
        {
        case T_player: {
            float rotationRad = currentState.objects[i].rotation * DEG2RAD;

            Vector2 litleTriangle = {
                currentState.objects[i].position.x + cosf(rotationRad) * (currentState.objects[i].radius / 2.0f),
                currentState.objects[i].position.y + sinf(rotationRad) * (currentState.objects[i].radius / 2.0f)};

            DrawPoly(litleTriangle, currentState.objects[i].sides, currentState.objects[i].radius / 2.0f,
                     currentState.objects[i].rotation, RED);
            break;
        }

        case T_bullet:
            DrawPoly(currentState.objects[i].position, currentState.objects[i].sides, currentState.objects[i].radius,
                     currentState.objects[i].rotation, currentState.objects[i].color);
            break;

        default:
            break;
        }
    }
}

void DrawScoreColor(int score, Color color)
{
    const char *score_text = TextFormat("YOUR SCORE WAS: %2i", score);
    DrawText(score_text, (WINDOW_WIDTH - MeasureText(score_text, 20)) / 2.0, WINDOW_HEIGHT / 2 - 100, 20, color);
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
    case S_lose: {
        // TODO(): All text here is hardcoded and will need to be adjustable to the screen size / ratio.
        const char *youLost = "YOU LOST";
        DrawScoreColor(currentState.score, RED);
        DrawText(youLost, (WINDOW_WIDTH - MeasureText(youLost, 60)) / 2.0, WINDOW_HEIGHT / 2 - 40, 60, RED);
        DrawText("Press [SPACE] to start again", (WINDOW_WIDTH - MeasureText("Press [SPACE] to start again", 30)) / 2.0,
                 WINDOW_HEIGHT / 2.0 + 30, 30, RED);
        break;
    }
    case S_win: {
        // TODO(): All text here is hardcoded and will need to be adjustable to the screen size / ratio.
        const char *youWon = "YOU WON";
        DrawScoreColor(currentState.score, GREEN);
        DrawText(youWon, (WINDOW_WIDTH - MeasureText(youWon, 60)) / 2.0, WINDOW_HEIGHT / 2 - 40, 60, GREEN);
        DrawText("Press [SPACE] to start again", (WINDOW_WIDTH - MeasureText("Press [SPACE] to start again", 30)) / 2.0,
                 WINDOW_HEIGHT / 2.0 + 30, 30, GREEN);
        break;
    }
    }

    EndDrawing();
}

void MoveObject(Object *obj)
{
    obj->position.x += obj->vel.x;
    obj->position.y += obj->vel.y;
}

void SetObjectDirAndSpeed(Object *obj, Vector2 to)
{
    obj->vel =
        Vector2Multiply(GetOrientationVector(obj->position, to), (Vector2){obj->speedMultiplier, obj->speedMultiplier});
}

void UpdatePlayingGameState(void)
{
    int i = 1;
    int j = 1;

    double timeNow = GetTime();

    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        switch (currentState.objects[i].type)
        {
        case T_none:
            continue;
            break;
        case T_enemy: {
            Object *enemy = &currentState.objects[i];
            // Todo implement proper AI / logic to move and attack the player
            // this is not a good experience, we need to find a way to make it
            // feel more real, now is like converging all T_enemies attacker into one point
            enemy->rotation += GetRandomValue(-10, 10);
            if (enemy->subType == OS_attacker)
            {
                for (j = 1; j < MAX_OBJECTS; ++j)
                {
                    if (j == i || j == currentState.poid || currentState.objects[j].type != T_enemy)
                        continue;
                    // this should only be between two enemies
                    if (CheckCollisionBetweenObjects(*enemy, currentState.objects[j]))
                    {
                        // we need to tell the object had collied and
                        // the ammount of time we want him to be in a
                        // different trajectory than the default one
                        // (chasing the player)
                        enemy->isColliding = true;
                        enemy->timeVisible = timeNow + enemy->duration;

                        Vector2 normal =
                            Vector2Normalize(Vector2Subtract(enemy->position, currentState.objects[j].position));
                        Vector2 relativeVel = Vector2Subtract(enemy->vel, currentState.objects[j].vel);
                        Vector2 reflctVel = Vector2Reflect(relativeVel, normal);

                        enemy->vel = Vector2Scale(reflctVel, -ENEMY_COLLISION_REFLECT_SCALE);
                        currentState.objects[j].vel = Vector2Scale(reflctVel, ENEMY_COLLISION_REFLECT_SCALE);
                        break;
                    }
                }

                // when we are not colliding we make sure is being set
                // state to not colliding and also that the attackers
                // are chasing the player.
                if (!enemy->isColliding || !(enemy->timeVisible >= GetTime()))
                {
                    enemy->isColliding = false;
                    SetObjectDirAndSpeed(enemy, currentState.player->position);
                }

                MoveObject(enemy);

                // if the move out of the window show up in the opposite side.
                WrapObjectPosition(&currentState.objects[i]);
            }
            break;
        }
        case T_bullet:
            MoveObject(&currentState.objects[i]);
            WrapObjectPosition(&currentState.objects[i]);

            // destroy the bullet
            if (currentState.objects[i].timeVisible < timeNow)
            {
                DestroyObject(&currentState, i);
                continue;
            }

            for (j = 1; j < MAX_OBJECTS; ++j)
            {
                if (j == i || j == currentState.poid || currentState.objects[i].type == currentState.objects[j].type)
                    continue;
                if (CheckCollisionBetweenObjects(currentState.objects[i], currentState.objects[j]))
                {
                    DestroyObject(&currentState, i);
                    DestroyObject(&currentState, j);
                    ++currentState.score;
                    break;
                }
            }

            // TraceLog(LOG_INFO, "currentState.activeObjects %d",
            // currentState.activeObjects);w
            break;
        case T_player:
            MoveObject(currentState.player);
            for (j = 1; j < MAX_OBJECTS; ++j)
            {
                // bullest won't destroyed the player for now
                if (j == i || currentState.objects[j].type == T_bullet)
                    continue;
                if (CheckCollisionBetweenObjects(currentState.objects[j], currentState.objects[i]))
                    currentState.status = S_lose;
            }
            break;
        default:
            continue;
        }
    }

    if (currentState.totalEnemies == 0)
    {
        currentState.status = S_win;
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
    SetInitialGameState(&currentState);

    while (!WindowShouldClose())
    {
        UpdateAndDrawFrame();
    }

    CloseWindow();

    return 0;
}

/*
 * TODO(1): Add subtypes of enemies that will have different AI behavior:
 *             - Attacker -> follow the player at reduced speed, to stress
 *              the player aim.
 *             - Ambusher -> Follow the player very slowly (at least half
 *              the speed of the attacker) and accelerate randomly but
 *              keeping a minimum distance. To create a feeling of ambush.
 *             - Dumb -> Just do random movements. Not following any one.
 *
 * TODO(2): Maybe add different types of gameplay, eg; change the way of
 *          keeping the score, like time to eliminate all enemies, or
 *          survival mode if enemies spawn for ever.
 *
 * TODO(3): Make the game available for other platforms, thinking on
 *          Web and mobile. But I would start with web now.
 *
 * TODO(4): Add sound support.
 *
 * TODO(5): Define more clear modules and expose them in their
 *          respective header files, like, player.h, enemies.h, etc. and maybe
 *          implement the logic within the same file by using the stb single
 *          header strategy, this will allow me to define different constants
 *          for prototyping different gameplay, etc behind gates
 */
