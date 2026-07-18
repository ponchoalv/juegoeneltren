#ifndef H_MENU
#define H_MENU

#include <stdint.h>

typedef struct
{
    const char *name;
    Vector2 position;
    void (*Action)(CurrentState);
} MenuItem;

typedef struct
{
    char *title;
    uint8_t selectedItem;
    uint8_t itemCount;
    MenuItem **items;
    Color background;
    Color foreground;
    Color activeForeground;
} Menu;

void ProcessMenuInput(Menu *menu);
void DrawMenu(Menu *menu);

void ProcessMenuInput(Menu *menu)
{
    if(menu->itemCount < 1) return;
}
#endif
