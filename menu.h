#ifndef H_MENU
#define H_MENU

typedef struct Menu Menu;
typedef struct MenuItem MenuItem;

// I think this should be Menu 1-* MenuItems
struct MenuItem
{
    char *name;
    Vector2 position;
    void (*action)(GameState *);
    void (*drawItem)(const MenuItem *, const GameState *, const Color);
};

struct Menu
{
    char *title;
    int selected;
    int count;
    MenuItem **items;
    Color background;
    Color foreground;
    Color activeForeground;
};

// This one I provided a generic implementation because I think we could do it
void ProcessInputMenu(Menu *menu, GameState *currentState);

// This method is not implemented because I don't know how to draw it in a generic way
// TODO(menu.h) Maybe including more information in the menu items related to how we want to draw.
void DrawMenu(const Menu *menu, const GameState *currentState);

#endif // H_MENU

#ifdef H_MENU_IMPLEMENTATION

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

    if (IsKeyPressed(KEY_SPACE))
    {
        currentState->status = S_playing;
        PlaySound(currentState->sounds[SO_menu]);
    }
}

// First attempt to generic drawing
void DrawMenu(const Menu *menu, const GameState *currentState)
{
    const char *exitMessage = "Press [SPACE] to exit the menu.";

    ClearBackground(menu->background);
    DrawText(menu->title, WINDOW_CENTRE_H - MeasureText(menu->title, 40) / 2.0, 30, 40, menu->foreground);

    for (int i = 0; i < menu->count; i++)
    {
        menu->items[i]->drawItem(menu->items[i], currentState,
                                 menu->selected == i ? menu->activeForeground : menu->foreground);
    }

    DrawText(exitMessage, WINDOW_CENTRE_H - MeasureText(exitMessage, 20) / 2.0, WINDOW_HEIGHT - 60, 20,
             menu->foreground);
}

#endif // H_MENU_IMPLEMENTATION
