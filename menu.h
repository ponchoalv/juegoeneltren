#ifndef H_MENU
#define H_MENU

typedef enum
{
    SP_FX,
    SP_music,
    SP_total
} SoundOpt;

typedef struct Menu
{
    char *title;
    char *volumeOptions[SP_total];
} Menu;

inline void DrawMenu()
{
    // static const char *optio
}

#endif