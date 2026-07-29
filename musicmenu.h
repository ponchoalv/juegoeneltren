#ifndef H_MUSIC_MENU
#define H_MUSIC_MENU

#define H_MENU_IMPLEMENTATION // process input for the menu is already implemented in the header file
#include "menu.h"

void MenuItemSFXVolume(GameState *currentState);
void MenuItemMusicVolume(GameState *currentState);
void DrawSFXVolumeItem(const Font *, const MenuItem *, const GameState *, const Color);
void DrawMusicVolumeItem(const Font *, const MenuItem *, const GameState *, const Color);

#endif // H_MUSIC_MENU

#ifdef H_MUSIC_MENU_IMPLEMENTATION

// For now I would prefer to not use dynamic arrays or maps.
MenuItem gMenuItemSFXVolume = {"SFX Volume:   %3.0f%%", M_CENTRE, (Vector2){WINDOW_CENTRE_H, 100}, MenuItemSFXVolume,
                               DrawSFXVolumeItem};
MenuItem gMenuItemMusicVolume = {"Music Volume: %3.0f%%", M_CENTRE, (Vector2){WINDOW_CENTRE_H, 140},
                                 MenuItemMusicVolume, DrawMusicVolumeItem};
MenuItem *menuItemVolItems[] = {&gMenuItemSFXVolume, &gMenuItemMusicVolume};
Menu menuVolume = {"Set Volume", 0, 2, menuItemVolItems, (Font){0}, DARKBLUE, WHITE, YELLOW};

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

// helper function to prevent code duplication
void DrawVolumeItem(const Font *font, const MenuItem *menuItem, const Color foreground, const float vol)
{
    const char *buff = TextFormat(menuItem->name, vol * 100.0f);
    const Vector2 buffMeasure = MeasureTextEx(*font, buff, 32, 3);
    DrawTextEx(*font, buff, (Vector2){menuItem->position.x - buffMeasure.x / 2.0f, menuItem->position.y}, 32, 3, foreground);
    /* DrawText(buff, (int)menuItem->position.x - MeasureText(buff, 30) / 2, (int)menuItem->position.y, 30, foreground); */
}

void DrawSFXVolumeItem(const Font *font, const MenuItem *menuItem, const GameState *currentState, const Color foreground)
{
    // Subscribe to the SFX volume variable from the game state.
    DrawVolumeItem(font, menuItem, foreground, currentState->soundsVol);
}

void DrawMusicVolumeItem(const Font *font, const MenuItem *menuItem, const GameState *currentState, const Color foreground)
{
    // Subscribe to the Music volume variable from the game state.
    DrawVolumeItem(font, menuItem, foreground, currentState->musicVol);
}

#endif // H_MUSIC_MENU_IMPLEMENTATION
