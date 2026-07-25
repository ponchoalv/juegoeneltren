#ifndef H_MUSIC_MENU
#define H_MUSIC_MENU

#include "menu.h"

void MenuItemSFXVolume(GameState *currentState);
void MenuItemMusicVolume(GameState *currentState);
void MenuDrawVolMenu(const Menu *menu, const GameState *currentState);

MenuItem gMenuItemSFXVolume = {"SFX Volume: %.0f%%", (Vector2){WINDOW_CENTRE_H, 100}, MenuItemSFXVolume};
MenuItem gMenuItemMusicVolume = {"Music Volume: %.0f%%", (Vector2){WINDOW_CENTRE_H, 140}, MenuItemMusicVolume};

void MenuItemSFXVolume(GameState *currentState)
{
    currentState->soundsVol += 0.1f;
    currentState->soundsVol = currentState->soundsVol > 1.01f ? 0.0f : currentState->soundsVol;

    for (int i = 0; i < SO_total; i++)
    {
        SetSoundVolume(currentState->sounds[i], currentState->soundsVol);
    }

#ifdef N_DEBUG
    TraceLog(LOG_INFO, "SFX vol is: %.1f", currentState->soundsVol);
#endif
}

void MenuItemMusicVolume(GameState *currentState)
{
    currentState->musicVol += 0.1f;
    currentState->musicVol = currentState->musicVol > 1.01f ? 0.0f : currentState->musicVol;
    SetMusicVolume(currentState->music, currentState->musicVol);
#ifdef N_DEBUG
    TraceLog(LOG_INFO, "Music vol is: %.1f", currentState->musicVol);
#endif
}

MenuItem *menuItemVolItems[] = {&gMenuItemSFXVolume, &gMenuItemMusicVolume};

Menu menuVolume = {"Set Volume", 0, 2, menuItemVolItems, DARKBLUE, WHITE, YELLOW};

void MenuDrawVolMenu(const Menu *menu, const GameState *currentState)
{
    ClearBackground(menu->background);
    DrawText(menu->title, WINDOW_CENTRE_H - MeasureText(menu->title, 40) / 2.0, 30, 40, menu->foreground);

    for (int i = 0; i < menu->count; i++)
    {
        const char *buff =
            TextFormat(menu->items[i]->name, (i == 0 ? currentState->soundsVol : currentState->musicVol) * 100.0f);
        DrawText(buff, (int)menu->items[i]->position.x - MeasureText(buff, 30) / 2, (int)menu->items[i]->position.y, 30,
                 menu->selected == i ? menu->activeForeground : menu->foreground);
    }
}
#endif
