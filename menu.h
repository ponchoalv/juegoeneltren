#ifndef H_MENU
#define H_MENU

typedef struct Menu Menu;
typedef struct MenuItem MenuItem;

typedef enum
{
    M_LEFT,
    M_CENTRE,
    M_RIGHT,
    M_NONE
} MenuItemAlignment;

typedef enum
{
    M_F_TITLE,
    M_F_EXIT,
    M_F_ITEM,
    M_F_TOTAL
} MenuFont;

// I think this should be Menu 1-* MenuItems
struct MenuItem
{
    char *name;
    MenuItemAlignment alignment;
    Vector2 position; // change this to LEFT, CENTRE, RIGHT
    void (*action)(GameState *);
    void (*drawItem)(const MenuItem *, const GameState *, const Color, const Font *, const float);
};

// How to add a font to the menu, or should be a global font used for everything?
// I don't like how this font thing looks, we will need to load the font texture for the different font sizes we want to
// support I think it would be better to leave to the menuItem to define their own font (and we call their font
// initializer from LoadFont)
// TODO(menu.h) I need to rethinl this font thing
// Maybe all the menus should have the same
struct Menu
{
    char *title;
    int selected;
    int count;
    MenuItem **items;
    float fontSize[M_F_TOTAL];
    Font font[M_F_TOTAL];
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

    const Vector2 exitMessageMeasure =
        MeasureTextEx(menu->font[M_F_EXIT], exitMessage, menu->fontSize[M_F_EXIT], 3); // Measure string size for Font
    const Vector2 titleMeasure =
        MeasureTextEx(menu->font[M_F_TITLE], menu->title, menu->fontSize[M_F_TITLE], 3); // Measure string size for Font

    ClearBackground(menu->background);

    DrawTextEx(menu->font[M_F_TITLE], menu->title, (Vector2){screenCentreH - titleMeasure.x / 2.0f, 30},
               menu->fontSize[M_F_TITLE], 3, menu->foreground);
    // DrawText(menu->title, (int)screenCentreH - MeasureText(menu->title, 40) / 2, 30, 40, menu->foreground);

    for (int i = 0; i < menu->count; i++)
    {
        menu->items[i]->position.x = screenCentreH;
        menu->items[i]->drawItem(menu->items[i], currentState,
                                 menu->selected == i ? menu->activeForeground : menu->foreground, &menu->font[M_F_ITEM],
                                 menu->fontSize[M_F_ITEM]);
    }

    // DrawTextEx(menu->font, menu->title, (Vector2){screenCentreH - titleMeasure.x / 2.0f, 30.0f}, 40, 4,
    // menu->foreground);
    DrawTextEx(menu->font[M_F_EXIT], exitMessage,
               (Vector2){screenCentreH - exitMessageMeasure.x / 2.0f, currentState->screenRes.y - 60.0f},
               menu->fontSize[M_F_EXIT], 3, menu->foreground);
}

void MenuLoadFont(Menu *menu, const char *filename)
{
    int i = 0;
    for (i = 0; i < M_F_TOTAL; i++)
    {
        menu->font[i] = LoadFontEx(filename, (int)menu->fontSize[i], NULL, 0);
    }
}

void MenuUnloadFont(Menu *menu)
{
    int i = 0;
    for (i = 0; i < M_F_TOTAL; i++)
    {
        UnloadFont(menu->font[i]);
    }
}

#endif // H_MENU_IMPLEMENTATION
