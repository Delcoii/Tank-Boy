#ifndef GAME_SYSTEM_H
#define GAME_SYSTEM_H

#include <stdbool.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include "tank.h"
#include "bullet.h"
#include "map_generation.h"
#include "ini_parser.h"
#include "input_system.h"
#include "head_up_display.h"

#define MAX_BULLETS 100

typedef struct {
    int buffer_width;
    int buffer_height;
    double display_scale;

    int button_width;
    int button_height;
    int button_spacing;

    int menu_bg_r, menu_bg_g, menu_bg_b;
    int game_bg_r, game_bg_g, game_bg_b;
    int button_normal_r, button_normal_g, button_normal_b;
    int button_hover_r, button_hover_g, button_hover_b;
    int button_clicked_r, button_clicked_g, button_clicked_b;
    int text_r, text_g, text_b;

    int game_speed;
    int max_lives;
} GameConfig;

typedef struct {
    int x, y;
    int width, height;
    char* text;
    bool hovered;
    bool clicked;
} Button;

typedef enum {
    STATE_MENU,
    STATE_RANKING,
    STATE_GAME,
    STATE_EXIT
} GameState;

typedef struct {
    // UI
    Button start_button;
    Button exit_button;
    ALLEGRO_FONT* font;
    GameState current_state;
    bool running;
    GameConfig config;
    ALLEGRO_BITMAP* buffer;

    // Player & Input
    Tank player_tank;
    InputState input;

    // Bullets
    Bullet bullets[MAX_BULLETS];
    int max_bullets;

    // Camera
    double camera_x, camera_y;

    // Map & Stage
    Map current_map;
    int current_stage;

    // HUD
    Head_Up_Display_Data hud;

    // Stage Clear & Score
    bool stage_clear;
    double stage_clear_timer;
    double stage_clear_scale;
    double score;            // 실제 점수
    double displayed_score;   // HUD에 표시되는 점수
} GameSystem;

// ================= Core Functions =================
void load_game_config(GameConfig* config, const char* config_file);
void init_game_system(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* queue, GameSystem* game_system);
void cleanup_game_system(GameSystem* game_system, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_DISPLAY* display);
void update_game_state(ALLEGRO_EVENT* event, GameSystem* game_system);
void render_game(GameSystem* game_system);

// ================= Buffer Handling =================
void disp_pre_draw(GameSystem* game_system);
void disp_post_draw(GameSystem* game_system);

#endif // GAME_SYSTEM_H
