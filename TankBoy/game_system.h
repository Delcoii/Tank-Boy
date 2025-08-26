#ifndef GAME_SYSTEM_H
#define GAME_SYSTEM_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <stdbool.h>

// Screen size definitions
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

// Game state enumeration
typedef enum {
    STATE_MENU,
    STATE_RANKING,
    STATE_GAME,
    STATE_EXIT
} GameState;

// Game system structure
typedef struct {
    Button start_button;
    Button exit_button;
    ALLEGRO_FONT* font;
    GameState current_state;
    bool running;
} GameSystem;

// Function declarations
void init_game_system(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* queue, GameSystem* game_system);
void cleanup_game_system(GameSystem* game_system, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_DISPLAY* display);
void update_game_state(ALLEGRO_EVENT* event, GameSystem* game_system);
void render_game(GameSystem* game_system);

#endif // GAME_SYSTEM_H