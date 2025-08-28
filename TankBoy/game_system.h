#ifndef GAME_SYSTEM_H
#define GAME_SYSTEM_H


// standard C lib
#include <stdbool.h>

// allegro5 lib
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

// algorithm
#include "tank.h"
#include "bullet.h"
#include "map_generation.h"


// utils
#include "ini_parser.h"
#include "input_system.h"
#include "head_up_display.h"

// Display buffer settings are now loaded from config.ini
// No more hardcoded defines!

// Game configuration structure
typedef struct {
    // Buffer settings
    int buffer_width;
    int buffer_height;
    double display_scale;
    
    // Button settings
    int button_width;
    int button_height;
    int button_spacing;
    
    // Colors (RGB values)
    int menu_bg_r, menu_bg_g, menu_bg_b;
    int game_bg_r, game_bg_g, game_bg_b;
    int button_normal_r, button_normal_g, button_normal_b;
    int button_hover_r, button_hover_g, button_hover_b;
    int button_clicked_r, button_clicked_g, button_clicked_b;
    int text_r, text_g, text_b;
    
    // Game settings
    int game_speed;
    int max_lives;
} GameConfig;

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
    GameConfig config;
    
    // Display buffer
    ALLEGRO_BITMAP* buffer;
    
    // Game objects
    Tank player_tank;
    InputState input;

    // Bullet system
    Bullet bullets[MAX_BULLETS];  
    int max_bullets;
    
    // Camera
    double camera_x, camera_y;
    
    // Map system
    Map current_map;
    int current_stage;

    // Head_Up_Display
    Head_Up_Display_Data hud;

} GameSystem;


// Function declarations
void load_game_config(GameConfig* config, const char* config_file);
void init_game_system(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* queue, GameSystem* game_system);
void cleanup_game_system(GameSystem* game_system, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_DISPLAY* display);
void update_game_state(ALLEGRO_EVENT* event, GameSystem* game_system);
void render_game(GameSystem* game_system);

// Display buffer functions
void disp_pre_draw(GameSystem* game_system);
void disp_post_draw(GameSystem* game_system);

#endif // GAME_SYSTEM_H