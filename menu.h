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

#endif // H_MENU_IMPLEMENTATION
