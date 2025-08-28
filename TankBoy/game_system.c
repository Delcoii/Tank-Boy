#include "game_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// =================== Config Loading ===================

void load_game_config(GameConfig* config, const char* config_file) {
    IniParser* parser = ini_parser_create();
    if (!parser) { printf("Error: INI parser create failed\n"); return; }

    char full_path[512];
    ini_parser_resolve_path(__FILE__, config_file, full_path, sizeof(full_path));
    if (!ini_parser_load_file(parser, full_path))
        printf("Warning: Config file '%s' not loaded. Using defaults.\n", full_path);

    // Buffer
    config->buffer_width = ini_parser_get_int(parser, "Buffer", "buffer_width", 320);
    config->buffer_height = ini_parser_get_int(parser, "Buffer", "buffer_height", 240);
    config->display_scale = ini_parser_get_double(parser, "Buffer", "display_scale", 3.0);

    // Buttons
    config->button_width = ini_parser_get_int(parser, "Buttons", "button_width", 200);
    config->button_height = ini_parser_get_int(parser, "Buttons", "button_height", 50);
    config->button_spacing = ini_parser_get_int(parser, "Buttons", "button_spacing", 70);

    // Colors
    config->menu_bg_r = ini_parser_get_int(parser, "Colors", "menu_bg_r", 50);
    config->menu_bg_g = ini_parser_get_int(parser, "Colors", "menu_bg_g", 50);
    config->menu_bg_b = ini_parser_get_int(parser, "Colors", "menu_bg_b", 100);

    config->game_bg_r = ini_parser_get_int(parser, "Colors", "game_bg_r", 0);
    config->game_bg_g = ini_parser_get_int(parser, "Colors", "game_bg_g", 100);
    config->game_bg_b = ini_parser_get_int(parser, "Colors", "game_bg_b", 0);

    config->button_normal_r = ini_parser_get_int(parser, "Colors", "button_normal_r", 200);
    config->button_normal_g = ini_parser_get_int(parser, "Colors", "button_normal_g", 200);
    config->button_normal_b = ini_parser_get_int(parser, "Colors", "button_normal_b", 200);

    config->button_hover_r = ini_parser_get_int(parser, "Colors", "button_hover_r", 150);
    config->button_hover_g = ini_parser_get_int(parser, "Colors", "button_hover_g", 150);
    config->button_hover_b = ini_parser_get_int(parser, "Colors", "button_hover_b", 150);

    config->button_clicked_r = ini_parser_get_int(parser, "Colors", "button_clicked_r", 100);
    config->button_clicked_g = ini_parser_get_int(parser, "Colors", "button_clicked_g", 100);
    config->button_clicked_b = ini_parser_get_int(parser, "Colors", "button_clicked_b", 100);

    config->text_r = ini_parser_get_int(parser, "Colors", "text_r", 255);
    config->text_g = ini_parser_get_int(parser, "Colors", "text_g", 255);
    config->text_b = ini_parser_get_int(parser, "Colors", "text_b", 255);

    // Game
    config->game_speed = ini_parser_get_int(parser, "Game", "game_speed", 60);
    config->max_lives = ini_parser_get_int(parser, "Game", "max_lives", 3);

    ini_parser_destroy(parser);
}

// =================== Button Helpers ===================

static void init_button(Button* btn, int x, int y, int w, int h, char* text) {
    btn->x = x; btn->y = y; btn->width = w; btn->height = h;
    btn->text = text; btn->hovered = false; btn->clicked = false;
}

static bool is_point_in_button(int x, int y, const Button* btn) {
    return x >= btn->x && x <= btn->x + btn->width &&
           y >= btn->y && y <= btn->y + btn->height;
}

static void draw_button(const Button* btn, const GameConfig* cfg, ALLEGRO_FONT* font) {
    ALLEGRO_COLOR bg, fg;
    if (btn->clicked)
        bg = al_map_rgb(cfg->button_clicked_r, cfg->button_clicked_g, cfg->button_clicked_b),
        fg = al_map_rgb(cfg->text_r, cfg->text_g, cfg->text_b);
    else if (btn->hovered)
        bg = al_map_rgb(cfg->button_hover_r, cfg->button_hover_g, cfg->button_hover_b),
        fg = al_map_rgb(cfg->text_r, cfg->text_g, cfg->text_b);
    else
        bg = al_map_rgb(cfg->button_normal_r, cfg->button_normal_g, cfg->button_normal_b),
        fg = al_map_rgb(0, 0, 0);

    al_draw_filled_rectangle(btn->x, btn->y, btn->x + btn->width, btn->y + btn->height, bg);
    al_draw_rectangle(btn->x, btn->y, btn->x + btn->width, btn->y + btn->height, al_map_rgb(0, 0, 0), 2);
    al_draw_text(font, fg, btn->x + btn->width / 2, btn->y + btn->height / 2 - 8, ALLEGRO_ALIGN_CENTER, btn->text);
}

static void draw_menu(const GameSystem* gs) {
    al_clear_to_color(al_map_rgb(gs->config.menu_bg_r, gs->config.menu_bg_g, gs->config.menu_bg_b));
    al_draw_text(gs->font, al_map_rgb(gs->config.text_r, gs->config.text_g, gs->config.text_b),
                 gs->config.buffer_width / 2, 100, ALLEGRO_ALIGN_CENTER, "Tank-Boy Game");
    draw_button(&gs->start_button, &gs->config, gs->font);
    draw_button(&gs->exit_button, &gs->config, gs->font);
}

// =================== Coordinate Conversion ===================

static void display_to_buffer_coords(int display_x, int display_y, int* buffer_x, int* buffer_y, const GameConfig* cfg) {
    *buffer_x = (int)(display_x / cfg->display_scale);
    *buffer_y = (int)(display_y / cfg->display_scale);
}

// =================== Game Initialization ===================

void init_game_system(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* queue, GameSystem* gs) {
    al_init_font_addon();
    al_init_primitives_addon();
    al_install_mouse();
    al_install_keyboard();

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());

    gs->buffer = al_create_bitmap(gs->config.buffer_width, gs->config.buffer_height);
    gs->font = al_create_builtin_font();

    int bx = gs->config.buffer_width / 2 - gs->config.button_width / 2;
    int sy = gs->config.buffer_height / 2 - gs->config.button_spacing / 2;
    int ey = gs->config.buffer_height / 2 + gs->config.button_spacing / 2;
    init_button(&gs->start_button, bx, sy, gs->config.button_width, gs->config.button_height, "Start Game");
    init_button(&gs->exit_button, bx, ey, gs->config.button_width, gs->config.button_height, "Exit Game");

    gs->current_state = STATE_MENU;
    gs->running = true;

    input_system_init(&gs->input);
    tank_init(&gs->player_tank, 50.0, 480.0);

    gs->max_bullets = MAX_BULLETS;
    bullets_init(gs->bullets, gs->max_bullets);

    gs->camera_x = 0;
    gs->camera_y = 0;

    head_up_display_init("config.ini");

    gs->current_stage = 1;
    char map_file[256];
    snprintf(map_file, sizeof(map_file), "TankBoy/resources/stages/stage%d.csv", gs->current_stage);
    if (!map_load(&gs->current_map, map_file))
        map_init(&gs->current_map);

    gs->stage_clear = false;
    gs->stage_clear_timer = 0.0;
    gs->stage_clear_scale = 1.0;
    gs->score = 0.0;  // 점수 초기화
}

// =================== Cleanup ===================

void cleanup_game_system(GameSystem* gs, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_DISPLAY* display) {
    map_free(&gs->current_map);
    al_destroy_bitmap(gs->buffer);
    al_destroy_font(gs->font);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
}

// =================== Input Handling ===================

static void handle_keyboard_input(ALLEGRO_EVENT* event, GameSystem* gs) {
    if (event->type != ALLEGRO_EVENT_KEY_DOWN) return;

    switch (event->keyboard.keycode) {
        case ALLEGRO_KEY_ESCAPE:
            if (gs->current_state == STATE_GAME) gs->current_state = STATE_MENU;
            else gs->running = false;
            break;
        case ALLEGRO_KEY_U:
            if (gs->current_state == STATE_GAME) {
                gs->stage_clear = true;
                gs->stage_clear_timer = 2.0;
                gs->stage_clear_scale = 1.0;
            }
            break;
    }
}

static void handle_mouse_input(ALLEGRO_EVENT* event, GameSystem* gs) {
    int bx, by;
    switch (event->type) {
        case ALLEGRO_EVENT_MOUSE_AXES:
            display_to_buffer_coords(event->mouse.x, event->mouse.y, &bx, &by, &gs->config);
            if (gs->current_state == STATE_MENU) {
                gs->start_button.hovered = is_point_in_button(bx, by, &gs->start_button);
                gs->exit_button.hovered = is_point_in_button(bx, by, &gs->exit_button);
            } else if (gs->current_state == STATE_GAME) {
                double cx = gs->player_tank.x - gs->camera_x + 16;
                double cy = gs->player_tank.y - gs->camera_y + 10;
                gs->player_tank.cannon_angle = atan2(by - cy, bx - cx);
            }
            break;

        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button != 1) break;
            display_to_buffer_coords(event->mouse.x, event->mouse.y, &bx, &by, &gs->config);
            if (gs->current_state == STATE_MENU) {
                if (is_point_in_button(bx, by, &gs->start_button)) gs->current_state = STATE_GAME;
                else if (is_point_in_button(bx, by, &gs->exit_button)) gs->running = false;
            }
            break;

        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            if (event->mouse.button == 1) {
                gs->start_button.clicked = false;
                gs->exit_button.clicked = false;
            }
            break;
    }
}

// =================== Game Update ===================

void update_game_state(ALLEGRO_EVENT* event, GameSystem* gs) {
    handle_keyboard_input(event, gs);
    handle_mouse_input(event, gs);
    input_system_update(&gs->input, event);

    if (gs->current_state != STATE_GAME) return;
    if (event->type != ALLEGRO_EVENT_TIMER) return;

    tank_update(&gs->player_tank, &gs->input, 1.0 / 60.0, gs->bullets, gs->max_bullets, &gs->current_map);
    bullets_update(gs->bullets, gs->max_bullets, &gs->current_map);

    // 점수: 1/10 단위로 증가
    gs->score += 1.0 / 60.0;

    // HUD 업데이트 (정수)
    gs->hud = head_up_display_update((int)(gs->score * 10), gs->player_tank.weapon, gs->current_stage);

    // 카메라
    gs->camera_x = gs->player_tank.x - gs->config.buffer_width / 3.0;
    gs->camera_y = gs->player_tank.y - gs->config.buffer_height / 2.0;

    // Stage Clear 처리
    if (gs->stage_clear) {
        gs->stage_clear_timer -= 1.0 / 60.0;
        gs->stage_clear_scale = 1.0 + 0.5 * sin((2.0 - gs->stage_clear_timer) * 3.14);

        if (gs->stage_clear_timer <= 0) {
            gs->stage_clear = false;
            gs->current_stage++;
            char map_file[256];
            snprintf(map_file, sizeof(map_file), "TankBoy/resources/stages/stage%d.csv", gs->current_stage);
            if (!map_load(&gs->current_map, map_file))
                map_init(&gs->current_map);
            tank_init(&gs->player_tank, 50.0, 480.0);
        }
    }
}

// =================== Rendering ===================

void disp_pre_draw(GameSystem* gs) {
    al_set_target_bitmap(gs->buffer);
}

void disp_post_draw(GameSystem* gs) {
    al_set_target_backbuffer(al_get_current_display());
    double w = gs->config.buffer_width * gs->config.display_scale;
    double h = gs->config.buffer_height * gs->config.display_scale;
    al_draw_scaled_bitmap(gs->buffer, 0, 0, gs->config.buffer_width, gs->config.buffer_height, 0, 0, w, h, 0);
    al_flip_display();
}

static void draw_game(const GameSystem* gs) {
    al_clear_to_color(al_map_rgb(gs->config.game_bg_r, gs->config.game_bg_g, gs->config.game_bg_b));

    map_draw(&gs->current_map, gs->camera_x, gs->camera_y, gs->config.buffer_width, gs->config.buffer_height);
    tank_draw(&gs->player_tank, gs->camera_x, gs->camera_y);
    bullets_draw(gs->bullets, gs->max_bullets, gs->camera_x, gs->camera_y);

    if (!gs->stage_clear) {
        head_up_display_draw(&gs->hud);
    } else {
        int cx = gs->config.buffer_width / 2;
        int cy = gs->config.buffer_height / 2;
        al_draw_text(gs->font, al_map_rgb(255, 255, 0), cx, cy, ALLEGRO_ALIGN_CENTER, "Stage Clear");

        char score_text[64];
        snprintf(score_text, sizeof(score_text), "Score: %d", (int)(gs->score * 10));
        al_draw_text(gs->font, al_map_rgb(255, 255, 255), cx, cy + 40, ALLEGRO_ALIGN_CENTER, score_text);
    }
}

void render_game(GameSystem* gs) {
    disp_pre_draw(gs);
    if (gs->current_state == STATE_MENU) draw_menu(gs);
    else if (gs->current_state == STATE_GAME) draw_game(gs);
    disp_post_draw(gs);
}
