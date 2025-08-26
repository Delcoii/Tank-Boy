#ifndef MODE_SELECTION_H
#define MODE_SELECTION_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <stdbool.h>

// Screen size
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Button structure
typedef struct {
    int x, y;
    int width, height;
    char* text;
    bool hovered;
    bool clicked;
} Button;

// Game states
typedef enum {
    STATE_MENU,
    STATE_GAME
} GameState;

// Function declarations
void init_button(Button* button, int x, int y, int width, int height, char* text);
bool is_point_in_button(int x, int y, Button* button);
void draw_button(Button* button, ALLEGRO_FONT* font);
void draw_menu(Button* start_button, Button* exit_button, ALLEGRO_FONT* font);
void draw_game(ALLEGRO_FONT* font);

#endif // MODE_SELECTION_H
