#include "head_up_display.h"
#include "ini_parser.h"
#include "enemy.h"
#include "tank.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>

// HUD font
static ALLEGRO_FONT* hud_font = NULL;

// Internal state
static int current_hp = 100;
static int current_score = 0;
static double last_time = 0;

// HUD
static ALLEGRO_COLOR hud_text_color;
static ALLEGRO_COLOR hud_hp_color;
static ALLEGRO_COLOR hud_border_color;

hud_sprites_t hud_sprites;
hud_settings_t hud_settings = {20, 20, 32, 32}; // Default HUD settings

// HUD initialization (reads config.ini)
void head_up_display_init(const char* config_file) {
    al_init_font_addon();
    al_init_ttf_addon();

    // Load HUD from config.ini
    IniParser* parser = ini_parser_create();
    ini_parser_load_file(parser, "TankBoy/config.ini");

    // Load font from config.ini [Font] section
    const char* font_file = ini_parser_get_string(parser, "Font", "font_file", "TankBoy/resources/fonts/pressstart.ttf");
    int font_size = ini_parser_get_int(parser, "Font", "font_size", 15);
    bool fallback_to_builtin = ini_parser_get_bool(parser, "Font", "fallback_to_builtin", true);
    
    // Try to load TTF font first
    hud_font = al_load_ttf_font(font_file, font_size, 0);
    if (!hud_font) {
        if (fallback_to_builtin) {
            // Fallback to builtin font if TTF fails
            hud_font = al_create_builtin_font();
            printf("HUD: Using builtin font (TTF font not found: %s)\\n", font_file);
        } else {
            printf("HUD: Failed to load font: %s\\n", font_file);
            return;
        }
    } else {
        printf("HUD: Loaded TTF font: %s (size: %d)\\n", font_file, font_size);
    }

    // Load HUD colors
    int head_up_display_text_r = ini_parser_get_int(parser, "HUD", "hud_text_r", 255);
    int head_up_display_text_g = ini_parser_get_int(parser, "HUD", "hud_text_g", 255);
    int head_up_display_text_b = ini_parser_get_int(parser, "HUD", "hud_text_b", 255);
    hud_text_color = al_map_rgb(head_up_display_text_r, head_up_display_text_g, head_up_display_text_b);

    int head_up_display_hp_r = ini_parser_get_int(parser, "HUD", "hud_hp_r", 255);
    int head_up_display_hp_g = ini_parser_get_int(parser, "HUD", "hud_hp_g", 0);
    int head_up_display_hp_b = ini_parser_get_int(parser, "HUD", "hud_hp_b", 0);
    hud_hp_color = al_map_rgb(head_up_display_hp_r, head_up_display_hp_g, head_up_display_hp_b);

    int head_up_display_border_r = ini_parser_get_int(parser, "HUD", "hud_border_r", 255);
    int head_up_display_border_g = ini_parser_get_int(parser, "HUD", "hud_border_g", 255);
    int head_up_display_border_b = ini_parser_get_int(parser, "HUD", "hud_border_b", 255);
    hud_border_color = al_map_rgb(head_up_display_border_r, head_up_display_border_g, head_up_display_border_b);

    // Load HUD positions and sizes
    hud_settings.hud_weapon_x = ini_parser_get_int(parser, "HUD", "hud_weapon_x", 1100);
    hud_settings.hud_weapon_y = ini_parser_get_int(parser, "HUD", "hud_weapon_y", 20);
    hud_settings.hud_weapon_width = ini_parser_get_int(parser, "HUD", "hud_weapon_width", 100);
    hud_settings.hud_weapon_height = ini_parser_get_int(parser, "HUD", "hud_weapon_height", 30);
    
    hud_settings.hud_text_r = head_up_display_text_r;
    hud_settings.hud_text_g = head_up_display_text_g;
    hud_settings.hud_text_b = head_up_display_text_b;
    hud_settings.hud_hp_r = head_up_display_hp_r;
    hud_settings.hud_hp_g = head_up_display_hp_g;
    hud_settings.hud_hp_b = head_up_display_hp_b;
    hud_settings.hud_border_r = head_up_display_border_r;
    hud_settings.hud_border_g = head_up_display_border_g;
    hud_settings.hud_border_b = head_up_display_border_b;
    
    hud_settings.hud_score_x = ini_parser_get_int(parser, "HUD", "hud_score_x", 20);
    hud_settings.hud_score_y = ini_parser_get_int(parser, "HUD", "hud_score_y", 20);
    hud_settings.hud_stage_x = ini_parser_get_int(parser, "HUD", "hud_stage_x", 20);
    hud_settings.hud_stage_y = ini_parser_get_int(parser, "HUD", "hud_stage_y", 50);
    hud_settings.hud_hp_x = ini_parser_get_int(parser, "HUD", "hud_hp_x", 20);
    hud_settings.hud_hp_y = ini_parser_get_int(parser, "HUD", "hud_hp_y", 80);
    hud_settings.hud_enemies_x = ini_parser_get_int(parser, "HUD", "hud_enemies_x", 20);
    hud_settings.hud_enemies_y = ini_parser_get_int(parser, "HUD", "hud_enemies_y", 110);
    hud_settings.hud_round_x = ini_parser_get_int(parser, "HUD", "hud_round_x", 20);
    hud_settings.hud_round_y = ini_parser_get_int(parser, "HUD", "hud_round_y", 140);

    if (parser) {
        ini_parser_destroy(parser);
    }
    
   
    current_hp = 100;
    current_score = 0;
    last_time = al_get_time();   

}

// HUD update
Head_Up_Display_Data head_up_display_update(int score, int weapon, int stage) {
    current_score = score; // Use the score passed from game system

    Head_Up_Display_Data hud;
    hud.player_hp = current_hp;
    hud.score = current_score;
    hud.weapon = weapon;
    hud.stage = stage;

    return hud;
}

// HUD draw
void head_up_display_draw(const Head_Up_Display_Data* hud) {
    if (!hud) {
        return;
    }
    
    if (!hud_font) {
        return;
    }

    // Weapon name
    const char* weapon_name = "Unknown";
    switch (hud->weapon) {
    case 0: weapon_name = "Machine Gun"; break;
    case 1: weapon_name = "Cannon"; break;
    }

    // HUD text using config positions
    al_draw_textf(hud_font, hud_text_color, hud_settings.hud_score_x, hud_settings.hud_score_y, 0,
        "Score  : %d", hud->score);
    al_draw_textf(hud_font, hud_text_color, hud_settings.hud_stage_x, hud_settings.hud_stage_y, 0,
        "Stage: %d", hud->stage);

    // Health bar using config positions
    int bar_x = hud_settings.hud_hp_x, bar_y = hud_settings.hud_hp_y, bar_w = 200, bar_h = 20;
    double ratio = hud->player_hp / 100.0;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    // Outline
    al_draw_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h,
        hud_border_color, 2);

    // Fill
    if (hud->player_hp > 0) {
        al_draw_filled_rectangle(bar_x + 1, bar_y + 1,
            bar_x + (int)(bar_w * ratio) - 1,
            bar_y + bar_h - 1,
            hud_hp_color);
    }

    // Health text in the center of the bar
    char hp_text[32];
    snprintf(hp_text, sizeof(hp_text), "%d / %d", hud->player_hp, hud->player_max_hp);
    int text_width = al_get_text_width(hud_font, hp_text);
    int text_x = bar_x + (bar_w - text_width) / 2;
    int text_y = bar_y + (bar_h - al_get_font_line_height(hud_font)) / 2;
    
    // Draw text with black outline for better visibility
    al_draw_text(hud_font, al_map_rgb(0, 0, 0), text_x - 1, text_y, 0, hp_text);
    al_draw_text(hud_font, al_map_rgb(0, 0, 0), text_x + 1, text_y, 0, hp_text);
    al_draw_text(hud_font, al_map_rgb(0, 0, 0), text_x, text_y - 1, 0, hp_text);
    al_draw_text(hud_font, al_map_rgb(0, 0, 0), text_x, text_y + 1, 0, hp_text);
    
    // Draw the main text
    al_draw_text(hud_font, hud_text_color, text_x, text_y, 0, hp_text);
        
    // Enemy count display - total enemies using config positions
    int total_enemies = hud->enemies_alive + hud->flying_enemies_alive;
    al_draw_textf(hud_font, hud_text_color, hud_settings.hud_enemies_x, hud_settings.hud_enemies_y, 0,
        "Enemies: %d", total_enemies);
    
    // Round display using config positions
    al_draw_textf(hud_font, hud_text_color, hud_settings.hud_round_x, hud_settings.hud_round_y, 0,
        "Round  : %d", hud->round);
    
    // draw sprites    
    if (weapon_name == "Machine Gun") {
        int width = al_get_bitmap_width(hud_sprites.tank_bullet_sheet);
        int height = al_get_bitmap_height(hud_sprites.tank_bullet_sheet);
        al_draw_scaled_bitmap(hud_sprites.tank_bullet_sheet, 0, 0, width, height,  
                            hud_settings.hud_weapon_x, hud_settings.hud_weapon_y,  // draw position 
                            hud_settings.hud_weapon_width, hud_settings.hud_weapon_height, 
                            0);
    } else if (weapon_name == "Cannon") {
        int width = al_get_bitmap_width(hud_sprites.cannon_bullet_sheet);
        int height = al_get_bitmap_height(hud_sprites.cannon_bullet_sheet);
        al_draw_scaled_bitmap(hud_sprites.cannon_bullet_sheet, 0, 0,      // draw start position
                            width, height,                              // original size to draw
                            hud_settings.hud_weapon_x + (hud_settings.hud_weapon_width/2), hud_settings.hud_weapon_y,  // draw position
                            hud_settings.hud_weapon_height*1.5, hud_settings.hud_weapon_height*1.5,  // draw size   
                            0);
    }   
}

// ===== Enemy HP Display Functions =====

void draw_enemy_hp_bars(void) {
    Enemy* enemies = get_enemies();
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;
        
        // Draw HP bar above enemy (using dynamic size)
        double hp_bar_width = e->width * 1.2; // HP bar width scales with enemy width
        if (hp_bar_width < 30.0) hp_bar_width = 30.0; // Minimum width
        if (hp_bar_width > 60.0) hp_bar_width = 60.0; // Maximum width
        double hp_bar_x = e->x + (e->width - hp_bar_width) / 2; // Center HP bar above enemy
        draw_hp_bar_world(hp_bar_x, e->y + e->height + 4, e->hp, e->max_hp, hp_bar_width);
    }
}

void draw_flying_enemy_hp_bars(void) {
    FlyingEnemy* f_enemies = get_flying_enemies();
    
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;
        
        // Draw HP bar above flying enemy (using dynamic size)
        double hp_bar_width = fe->width * 1.2; // HP bar width scales with enemy width
        if (hp_bar_width < 30.0) hp_bar_width = 30.0; // Minimum width
        if (hp_bar_width > 60.0) hp_bar_width = 60.0; // Maximum width
        double hp_bar_x = fe->x + (fe->width - hp_bar_width) / 2; // Center HP bar above enemy
        draw_hp_bar_world(hp_bar_x, fe->y + fe->height + 4, fe->hp, fe->max_hp, hp_bar_width);
    }
}

// ===== World-space HP Bar Drawing =====

void draw_hp_bar_world(double wx, double wy, int hp, int hp_max, double bar_w) {
    if (hp_max <= 0) return;
    if (hp < 0) hp = 0;
    if (hp > hp_max) hp = hp_max;
    double ratio = (double)hp / (double)hp_max;

    // Get camera position for screen coordinates
    double camera_x = get_camera_x();
    double camera_y = get_camera_y();

    double sx = wx - camera_x;
    double sy = wy - camera_y;

    // Background bar
    al_draw_filled_rectangle(sx, sy, sx + bar_w, sy + 5, al_map_rgb(35, 35, 35));
    
    // HP bar
    al_draw_filled_rectangle(sx, sy, sx + bar_w * ratio, sy + 5, al_map_rgb(220, 40, 40));
    
    // Border
    al_draw_rectangle(sx, sy, sx + bar_w, sy + 5, al_map_rgb(255, 255, 255), 1);
}

// ===== HUD Update Functions =====

void update_tank_hp_display(int new_hp) {
    current_hp = new_hp;
}

void update_enemy_count_display(int ground_count, int flying_count) {
    // This function can be used to update enemy counts in real-time
    // Currently the counts are updated in the main update loop
}

void hud_sprites_init() {
    hud_sprites.button_sheet = al_load_bitmap("TankBoy/resources/sprites/button_sheet.png");
    hud_sprites.tank_bullet_sheet = al_load_bitmap("TankBoy/resources/sprites/tank_bullet.png");
    hud_sprites.cannon_bullet_sheet = al_load_bitmap("TankBoy/resources/sprites/cannon_bullet.png");

    if (hud_sprites.button_sheet == NULL || hud_sprites.tank_bullet_sheet == NULL || hud_sprites.cannon_bullet_sheet == NULL) {
        printf("wrong location of hud sprite!!\n");
    }
}