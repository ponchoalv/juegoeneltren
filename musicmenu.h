#ifndef H_MUSIC_MENU
#define H_MUSIC_MENU

#define H_MENU_IMPLEMENTATION // process input for the menu is already implemented in the header file
#include "menu.h"

void MenuItemSFXVolume(GameState *currentState);
void MenuItemMusicVolume(GameState *currentState);

#endif // H_MUSIC_MENU

#ifdef H_MUSIC_MENU_IMPLEMENTATION

// For now I would prefer to not use dynamic arrays or maps.
MenuItem gMenuItemSFXVolume = {"SFX Volume: %.0f%%", (Vector2){WINDOW_CENTRE_H, 100}, MenuItemSFXVolume};
MenuItem gMenuItemMusicVolume = {"Music Volume: %.0f%%", (Vector2){WINDOW_CENTRE_H, 140}, MenuItemMusicVolume};
MenuItem *menuItemVolItems[] = {&gMenuItemSFXVolume, &gMenuItemMusicVolume};
Menu menuVolume = {"Set Volume", 0, 2, menuItemVolItems, DARKBLUE, WHITE, YELLOW};

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

// we must always implement this one, I think is always related to what are you trying to do with the menu
// Maybe in the future I could find a way to describe it with data and create a generic draw.
// But I suspect that we would need to use dynamic arrays to / maps to store more information.
// Maybe we could add a callback function to every item to tell how it should draw.
// This way we could keep the generic menu render and each item is responsible for defining how the should render (similar to the action).
void DrawMenu(const Menu *menu, const GameState *currentState)
{
    const char * exitMessage = "Press [SPACE] to exit the menu.";
    
    ClearBackground(menu->background);
    DrawText(menu->title, WINDOW_CENTRE_H - MeasureText(menu->title, 40) / 2.0, 30, 40, menu->foreground);

    for (int i = 0; i < menu->count; i++)
    {
        const char *buff =
            TextFormat(menu->items[i]->name, (i == 0 ? currentState->soundsVol : currentState->musicVol) * 100.0f);
        DrawText(buff, (int)menu->items[i]->position.x - MeasureText(buff, 30) / 2, (int)menu->items[i]->position.y, 30,
                 menu->selected == i ? menu->activeForeground : menu->foreground);
    }

    DrawText(exitMessage, WINDOW_CENTRE_H - MeasureText(exitMessage, 20) / 2.0, WINDOW_HEIGHT - 60, 20, menu->foreground);
}

#endif // H_MUSIC_MENU_IMPLEMENTATION
