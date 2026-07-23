#ifndef H_MENU
#define H_MENU

#include <stdint.h>
#define CENTER(width) ((WINDOW_WIDTH / 2) - (((uint16_t)width * 6) / 2))

// I think this should be Menu 1-* MenuItems
typedef struct
{
    char *name;
    Vector2 position;
    void (*Action)(GameState*);
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
void MenuDrawVolMenu(Menu menu);

MenuItem gMenuItemSFXVolume = {"SFX Volume:", (Vector2){WINDOW_CENTRE_H, 100}, MenuItemSFXVolume};
MenuItem gMenuItemMusicVolume = {"Music Volume:", (Vector2){WINDOW_CENTRE_H, 140}, MenuItemMusicVolume};

void MenuItemSFXVolume(GameState *currentState) {
    static float sfxVol = 0.1f;
    sfxVol += 0.1f;
    for (int i = 0; i < SO_total; i++)
    {
        SetSoundVolume(currentState->sounds[i], sfxVol);
    }
    
    TraceLog(LOG_INFO, "SFX vol is: %d", sfxVol);
}

void MenuItemMusicVolume(GameState *currentState) {
    static float musicVol = 0.1f;
    musicVol += 0.1f;
    SetMusicVolume(currentState->music, musicVol);
    TraceLog(LOG_INFO, "Music vol is: %d", musicVol);
}

MenuItem *menuItemVolItems[] = {&gMenuItemSFXVolume, &gMenuItemMusicVolume};

Menu menuVolume = { "Set Volume", 0, 2,  menuItemVolItems, BLACK, WHITE, YELLOW};

void MenuDrawVolMenu(Menu menu)
{
    ClearBackground(menu.background);    
    DrawText(menu.title, WINDOW_CENTRE_H - MeasureText(menu.title, 40)/2.0, 30, 40, menu.foreground);

    for (int i = 0; i < menu.count; i++)
    {
        DrawText(menu.items[i]->name, (int)menu.items[i]->position.x - MeasureText(menu.items[i]->name, 30) / 2.0, (int)menu.items[i]->position.y, 30, menu.selected == i ? menu.activeForeground : menu.foreground);
    }

}
#endif
