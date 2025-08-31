#ifndef GAME_SYSTEM_H
#define GAME_SYSTEM_H

#include <stdbool.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

// Game system components
#include "tank.h"           // Tank player character
#include "bullet.h"         // Bullet system for shooting
#include "map_generation.h" // Map and collision system
#include "ini_parser.h"     // Configuration file parser
#include "input_system.h"   // Keyboard and mouse input handling
#include "head_up_display.h" // HUD and UI display system

// MAX_BULLETS is now loaded from config.ini

typedef struct {
    // Display buffer settings
    int buffer_width;
    int buffer_height;
    double display_scale;

    // Button UI settings
    int button_width;
    int button_height;
    int button_spacing;

    // Color settings
    int menu_bg_r, menu_bg_g, menu_bg_b;
    int game_bg_r, game_bg_g, game_bg_b;
    int button_normal_r, button_normal_g, button_normal_b;
    int button_hover_r, button_hover_g, button_hover_b;
    int button_clicked_r, button_clicked_g, button_clicked_b;
    int text_r, text_g, text_b;

    // Game settings
    int game_speed;
    int max_lives;
    int max_bullets;
    
    // Font settings
    char font_file[256];
    int font_size;
    bool fallback_to_builtin;
} GameConfig;

typedef struct {
    // Button position and size
    int x, y;
    int width, height;
    char* text;
    // Button state
    bool hovered;
    bool clicked;
    // Button sprites
    ALLEGRO_BITMAP* button_sprite;
} Button;

typedef enum {
    STATE_MENU,    // Main menu state
    STATE_RANKING, // Ranking display state
    STATE_GAME,    // Active gameplay state
    STATE_GAME_OVER, // Game over state
    STATE_NAME_INPUT, // Name input state for ranking
    STATE_STAGE_COMPLETE, // Stage complete screen state
    STATE_EXIT     // Exit game state
} GameState;

typedef struct {
    // UI Components
    Button start_button;      // Start game button
    Button exit_button;       // Exit game button
    Button ranking_button;    // Ranking button
    Button next_button;       // Next stage button (for stage clear screen)
    Button menu_button;       // Back to menu button (for game end screen)
    Button ranking_page_button; // Ranking page button (for stage complete screen)
    ALLEGRO_FONT* font;      // Font for text rendering
    GameState current_state;  // Current game state
    bool running;             // Game loop running flag
    GameConfig config;        // Game configuration settings
    ALLEGRO_BITMAP* buffer;  // Off-screen rendering buffer

    // Player & Input
    Tank player_tank;         // Player tank character
    InputState input;         // Input system state
    TextInput name_input;     // Text input for player name

    // Bullet System
    Bullet* bullets;          // Dynamic bullet array
    int max_bullets;          // Maximum number of bullets

    // Camera System
    double camera_x, camera_y; // Camera position for viewport

    // Map & Stage System
    Map current_map;          // Current stage map
    SpawnPoints spawn_points; // Current stage spawn points
    int current_stage;        // Current stage number

    // HUD System
    Head_Up_Display_Data hud; // Heads-up display data
    
    // Background System
    ALLEGRO_BITMAP* bg_green;   // Stage 1 background
    ALLEGRO_BITMAP* bg_volcano; // Stage 2 background
    ALLEGRO_BITMAP* bg_snow;    // Stage 3 background
    ALLEGRO_BITMAP* intro_bg;   // Intro screen background
    
    // Fonts
    ALLEGRO_FONT* title_font;   // Large font for titles
    
    // Enemy System
    int round_number;         // Current round number
    bool enemies_spawned;     // Enemy spawn flag

    // Stage Clear & Score System
    bool stage_clear;         // Stage clear flag
    double stage_clear_timer; // Stage clear animation timer
    double stage_clear_scale; // Stage clear animation scale
    double score;             // Current score
    double displayed_score;   // Score displayed in HUD
    
    // Game Over System
    bool game_over;           // Game over flag
    double game_over_timer;   // Game over animation timer
    double game_over_scale;   // Game over animation scale
} GameSystem;

// ================= Core Functions =================
void load_game_config(GameConfig* config, const char* config_file);                    // Load game configuration from INI file
void init_game_system(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* queue, GameSystem* game_system); // Initialize game system
void cleanup_game_system(GameSystem* game_system, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_DISPLAY* display); // Cleanup game system
void update_game_state(ALLEGRO_EVENT* event, GameSystem* game_system);                // Update game state based on events
void render_game(GameSystem* game_system);                                            // Render current game state

// ================= Buffer Handling =================
void disp_pre_draw(GameSystem* game_system);                                          // Set up off-screen buffer for rendering
void disp_post_draw(GameSystem* game_system);                                         // Display rendered buffer and flip screen

// ================= Score System =================
void set_global_game_system(GameSystem* gs);                                         // Set global game system reference for scoring
void add_score_for_enemy_kill(int difficulty);                                       // Add score when enemy is killed


#endif // GAME_SYSTEM_H
