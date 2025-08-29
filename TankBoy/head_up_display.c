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

// HUD Colors
static ALLEGRO_COLOR hud_text_color;
static ALLEGRO_COLOR hud_hp_color;
static ALLEGRO_COLOR hud_border_color;

// HUD initialization (reads config.ini)
void head_up_display_init(const char* config_file) {
    al_init_font_addon();
    al_init_ttf_addon();

    hud_font = al_create_builtin_font();
    if (!hud_font) {
        // Continue even if font creation fails
        return;
    }

    // Load HUD colors from config.ini
    IniParser* parser = ini_parser_create();
    if (parser && ini_parser_load_file(parser, config_file)) {
        int head_up_display_text_r = ini_parser_get_int(parser, "HUD Colors", "hud_text_r", 255);
        int head_up_display_text_g = ini_parser_get_int(parser, "HUD Colors", "hud_text_g", 255);
        int head_up_display_text_b = ini_parser_get_int(parser, "HUD Colors", "hud_text_b", 255);
        hud_text_color = al_map_rgb(head_up_display_text_r, head_up_display_text_g, head_up_display_text_b);

        int head_up_display_hp_r = ini_parser_get_int(parser, "HUD Colors", "hud_hp_r", 255);
        int head_up_display_hp_g = ini_parser_get_int(parser, "HUD Colors", "hud_hp_g", 0);
        int head_up_display_hp_b = ini_parser_get_int(parser, "HUD Colors", "hud_hp_b", 0);
        hud_hp_color = al_map_rgb(head_up_display_hp_r, head_up_display_hp_g, head_up_display_hp_b);

        int head_up_display_border_r = ini_parser_get_int(parser, "HUD Colors", "hud_border_r", 255);
        int head_up_display_border_g = ini_parser_get_int(parser, "HUD Colors", "hud_border_g", 255);
        int head_up_display_border_b = ini_parser_get_int(parser, "HUD Colors", "hud_border_b", 255);
        hud_border_color = al_map_rgb(head_up_display_border_r, head_up_display_border_g, head_up_display_border_b);
        

    } else {
        // Use default colors
        hud_text_color = al_map_rgb(255, 255, 255);
        hud_hp_color = al_map_rgb(255, 0, 0);
        hud_border_color = al_map_rgb(255, 255, 255);
    }
   
    if (parser) {
        ini_parser_destroy(parser);
    }
   
    current_hp = 100;
    current_score = 0;
    last_time = al_get_time();
    

}

// HUD update
Head_Up_Display_Data head_up_display_update(int damage, int weapon, int stage) {
    current_hp -= damage;
    if (current_hp < 0) current_hp = 0;

    double now = al_get_time();
    double elapsed = now - last_time;
    if (elapsed >= 1.0) {
        current_score += (int)elapsed;
        last_time = now;
    }

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

    // HUD text
    al_draw_textf(hud_font, hud_text_color, 10, 10, 0,
        "Weapon: %s", weapon_name);
    al_draw_textf(hud_font, hud_text_color, 10, 40, 0,
        "Score: %d", hud->score);
    al_draw_textf(hud_font, hud_text_color, 10, 70, 0,
        "Stage: %d", hud->stage);

    // Health bar
    int bar_x = 10, bar_y = 100, bar_w = 200, bar_h = 20;
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

    // Health text
    al_draw_textf(hud_font, hud_text_color,
        bar_x + bar_w + 10, bar_y, 0,
        "%d / %d", hud->player_hp, hud->player_max_hp);
        
    // Enemy count display
    al_draw_textf(hud_font, hud_text_color, 10, 140, 0,
        "Enemies: %d", hud->enemies_alive);
    al_draw_textf(hud_font, hud_text_color, 10, 170, 0,
        "Flying Enemies: %d", hud->flying_enemies_alive);
    
    // Round display
    al_draw_textf(hud_font, hud_text_color, 10, 200, 0,
        "Round: %d", hud->round);

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

/* Usage:

#include "head_up_display.h"

head_up_display_init("config.ini");

Head_Up_Display_Data hud = head_up_display_update(int damage, int(enum)tank.weapon, (int) stage);

head_up_display_draw(&hud);

// Draw enemy HP bars
draw_enemy_hp_bars();
draw_flying_enemy_hp_bars();

*/