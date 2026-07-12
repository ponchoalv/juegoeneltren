#include <raylib.h>
#include <raymath.h>

#include "gamestate.h"
#include "player.h"
#include "enemies.h"

#define ENEMY_COLLISION_REFLECT_SCALE 0.65f
#define NO_LOSE

#define SHOW_FPS

CurrentState currentState = {0};


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
    InitPlayer(currentState);
    InitEnemies(currentState);
}

void ProcessInput(void)
{
    CaptureMouseWithinWindow();

    switch (currentState.status)
    {
    case S_playing:
        ProcessPlayerInput(&currentState);
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

void UpdatePlayingGameState(void)
{
    int i = 1;
    double timeNow = GetTime();

    for (i = 1; i < MAX_OBJECTS; ++i)
    {
        UpdateStateWithCollisions(&currentState, i, timeNow);

        // TODO(gamestate): this should be moved to gamestate.h (along side with this methods)
        MoveObject(&currentState.objects[i]);
        WrapObjectPosition(&currentState.objects[i]);

        switch (currentState.objects[i].type)
        {
        case T_enemy: {
            // TODO(enemies): should move this to enemies.h I think
            Object *enemy = &currentState.objects[i];
            // Todo implement proper AI / logic to move and attack the player
            // this is not a good experience, we need to find a way to make it
            // feel more real, now is like converging all T_enemies attacker into one point
            enemy->rotation += GetRandomValue(-10, 10);

            // when we are not colliding we make sure is being set
            // state to not colliding and also that the attackers
            // are chasing the player.
            if (enemy->subType == OS_attacker && (!enemy->isColliding || !(enemy->timeVisible >= GetTime())))
            {
                enemy->isColliding = false;
                // WIP: testing if adding some random scalar to the player position would make it more fun
                SetObjectDirAndSpeed(enemy, Vector2Scale(currentState.player->position, GetRandomValue(1, 2)));
            }
            break;
        }
        case T_bullet:
            // TODO(player): we should move this to the player.h
            // destroy the bullet
            if (currentState.objects[i].timeVisible < timeNow)
            {
                DestroyObject(&currentState, i);
            }

            // TraceLog(LOG_INFO, "currentState.activeObjects %d",
            // currentState.activeObjects);w
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
