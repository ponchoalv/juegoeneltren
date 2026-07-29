#ifndef H_MENU
#define H_MENU

typedef struct Menu Menu;
typedef struct MenuItem MenuItem;

typedef enum {
    M_LEFT,
    M_CENTRE,
    M_RIGHT,
    M_NONE
} MenuItemAlignment;

// I think this should be Menu 1-* MenuItems
struct MenuItem
{
    char *name;
    MenuItemAlignment alignment;
    Vector2 position; // change this to LEFT, CENTRE, RIGHT
    void (*action)(GameState *);
    void (*drawItem)(const Font*, const MenuItem *, const GameState *, const Color);
};

// How to add a font to the menu, or should be a global font used for everything?
// Maybe all the menus should have the same
struct Menu
{
    char *title;
    int selected;
    int count;
    MenuItem **items;
    Font font;
    Color background;
    Color foreground;
    Color activeForeground;
};

// This one I provided a generic implementation because I think we could do it
void MenuProcessInput(Menu *menu, GameState *currentState);

// This method is not implemented because I don't know how to draw it in a generic way
// TODO(menu.h) Maybe including more information in the menu items related to how we want to draw.
void MenuDraw(const Menu *menu, const GameState *currentState);

// Load Font into the GPU & RAM
void MenuLoadFont(Menu *menu, const char *filename);

// Unload Font from the GPU & RAM
void MenuUnloadFont(Menu *menu);

#endif // H_MENU

#ifdef H_MENU_IMPLEMENTATION

void MenuProcessInput(Menu *menu, GameState *currentState)
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
void MenuDraw(const Menu *menu, const GameState *currentState)
{
    const char *exitMessage = "Press [SPACE] to exit the menu.";
    const float screenCentreH = GetScreenCentre(currentState).x;

    const Vector2 exitMessageMeasure = MeasureTextEx(menu->font, exitMessage, 32, 3); // Measure string size for Font
    const Vector2 titleMeasure = MeasureTextEx(menu->font, menu->title, 32, 3); // Measure string size for Font

    ClearBackground(menu->background);

    DrawTextEx(menu->font, menu->title, (Vector2){screenCentreH - titleMeasure.x / 2.0f, 30}, 32, 3, menu->foreground);
    // DrawText(menu->title, (int)screenCentreH - MeasureText(menu->title, 40) / 2, 30, 40, menu->foreground);

    for (int i = 0; i < menu->count; i++)
    {
        menu->items[i]->position.x = screenCentreH;
        menu->items[i]->drawItem(&menu->font, menu->items[i], currentState,
                                 menu->selected == i ? menu->activeForeground : menu->foreground);
    }

    // DrawTextEx(menu->font, menu->title, (Vector2){screenCentreH - titleMeasure.x / 2.0f, 30.0f}, 40, 4, menu->foreground);
    DrawTextEx(menu->font, exitMessage, (Vector2){screenCentreH - exitMessageMeasure.x / 2.0f, currentState->screenRes.y - 60.0f}, 32, 3, menu->foreground);
}

void MenuLoadFont(Menu *menu, const char *filename)
{
    menu->font = LoadFont(filename);
}

void MenuUnloadFont(Menu *menu)
{
    UnloadFont(menu->font);
}

#endif // H_MENU_IMPLEMENTATION
