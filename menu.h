#ifndef H_MENU
#define H_MENU

#include <stdint.h>
#define CENTER(width) ((WINDOW_WIDTH / 2) - (((uint16_t)width * 6) / 2))

// I think this should be Menu 1-* MenuItems
typedef struct
{
    char *name;
    Vector2 position;
    void (*action)(GameState*);
} MenuItem;


typedef struct
{
    char *title;
    uint8_t selected;
    uint8_t count;
    MenuItem **items;
    Color background;
    Color foreground;
    Color activeForeground;
} Menu;

void MenuItemSFXVolume(GameState *currentState);
void MenuItemMusicVolume(GameState *currentState);
void MenuDrawVolMenu(const Menu *menu, const GameState *currentState);
void ProcessInputMenu(Menu *menu, GameState *currentState);

MenuItem gMenuItemSFXVolume = {"SFX Volume: %f", (Vector2){WINDOW_CENTRE_H, 100}, MenuItemSFXVolume};
MenuItem gMenuItemMusicVolume = {"Music Volume: %f", (Vector2){WINDOW_CENTRE_H, 140}, MenuItemMusicVolume};

void MenuItemSFXVolume(GameState *currentState) {
    currentState->soundsVol += 0.1f;
    for (int i = 0; i < SO_total; i++)
    {
        SetSoundVolume(currentState->sounds[i], currentState->soundsVol);
    }

    TraceLog(LOG_INFO, "SFX vol is: %.*f", currentState->soundsVol);
}

void MenuItemMusicVolume(GameState *currentState) {
    currentState->musicVol += 0.1f;
    SetMusicVolume(currentState->music, currentState->musicVol);
    TraceLog(LOG_INFO, "Music vol is: %.1f", currentState->musicVol);
}

MenuItem *menuItemVolItems[] = {&gMenuItemSFXVolume, &gMenuItemMusicVolume};

Menu menuVolume = { "Set Volume", 0, 2,  menuItemVolItems, BLACK, WHITE, YELLOW};

void MenuDrawVolMenu(const Menu *menu, const GameState *currentState)
{
    ClearBackground(menu->background);
    DrawText(menu->title, WINDOW_CENTRE_H - MeasureText(menu->title, 40)/2.0, 30, 40, menu->foreground);

    for (int i = 0; i < menu->count; i++)
    {
        const char *buff = TextFormat(menu->items[i]->name, currentState->soundsVol);
        DrawText(buff, (int)menu->items[i]->position.x - MeasureText(buff, 30) / 2.0, (int)menu->items[i]->position.y, 30, menu->selected == i ? menu->activeForeground : menu->foreground);
    }

}

void MenuVolProcessInput(Menu *menu, GameState *currentState)
{
    if (IsKeyPressed(KEY_DOWN))
    {
        menu->selected += 1;
        menu->selected %= menu->count;
    }

    if (IsKeyPressed(KEY_UP))
    {
        menu->selected -= 1;
        menu->selected %= menu->count;
    }


    if (IsKeyPressed(KEY_ENTER))
    {
        menu->items[menu->selected]->action(currentState);
    }
}

#endif
