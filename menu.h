#ifndef H_MENU
#define H_MENU

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
    int selected;
    int count;
    MenuItem **items;
    Color background;
    Color foreground;
    Color activeForeground;
} Menu;

void MenuItemSFXVolume(GameState *currentState);
void MenuItemMusicVolume(GameState *currentState);
void MenuDrawVolMenu(const Menu *menu, const GameState *currentState);
void ProcessInputMenu(Menu *menu, GameState *currentState);

MenuItem gMenuItemSFXVolume = {"SFX Volume: %.0f%%", (Vector2){WINDOW_CENTRE_H, 100}, MenuItemSFXVolume};
MenuItem gMenuItemMusicVolume = {"Music Volume: %.0f%%", (Vector2){WINDOW_CENTRE_H, 140}, MenuItemMusicVolume};

void MenuItemSFXVolume(GameState *currentState) {
    currentState->soundsVol += 0.1f;
    currentState->soundsVol = currentState->soundsVol > 1.01f ? 0.0f : currentState->soundsVol;

    for (int i = 0; i < SO_total; i++)
    {
        SetSoundVolume(currentState->sounds[i], currentState->soundsVol);
    }

    TraceLog(LOG_INFO, "SFX vol is: %.1f", currentState->soundsVol);
}

void MenuItemMusicVolume(GameState *currentState) {
    currentState->musicVol += 0.1f;
    currentState->musicVol = currentState->musicVol > 1.01f ? 0.0f : currentState->musicVol;
    SetMusicVolume(currentState->music, currentState->musicVol);
    TraceLog(LOG_INFO, "Music vol is: %.1f", currentState->musicVol);
}

MenuItem *menuItemVolItems[] = {&gMenuItemSFXVolume, &gMenuItemMusicVolume};

Menu menuVolume = { "Set Volume", 0, 2,  menuItemVolItems, DARKBLUE, WHITE, YELLOW};

void MenuDrawVolMenu(const Menu *menu, const GameState *currentState)
{
    ClearBackground(menu->background);
    DrawText(menu->title, WINDOW_CENTRE_H - MeasureText(menu->title, 40)/2.0, 30, 40, menu->foreground);

    for (int i = 0; i < menu->count; i++)
    {
        const char *buff = TextFormat(menu->items[i]->name, (i == 0 ? currentState->soundsVol : currentState->musicVol) * 100.0f);
        DrawText(buff, (int)menu->items[i]->position.x - MeasureText(buff, 30) / 2.0, (int)menu->items[i]->position.y, 30, menu->selected == i ? menu->activeForeground : menu->foreground);
    }
}

void MenuVolProcessInput(Menu *menu, GameState *currentState)
{
    if (IsKeyPressed(KEY_DOWN))
    {
        menu->selected += 1;
        menu->selected %= menu->count;
        PlaySound(currentState->sounds[SO_menu]);
    }

    if (IsKeyPressed(KEY_UP))
    {
        menu->selected -= 1;
        if (menu->selected < 0)
        {
            menu->selected = menu->count - 1;
        }
        PlaySound(currentState->sounds[SO_menu]);
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        menu->items[menu->selected]->action(currentState);
        PlaySound(currentState->sounds[SO_menu]);
    }
}

#endif
