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

void ProcessInputMenu(Menu *menu, GameState *currentState);

void ProcessInputMenu(Menu *menu, GameState *currentState)
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

    if (IsKeyPressed(KEY_M))
    {
        currentState->status = S_playing;
        PlaySound(currentState->sounds[SO_menu]);
    }
}

#endif
