#include "game_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "enemy.h"
#include "collision.h"

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
    config->max_bullets = ini_parser_get_int(parser, "Game", "max_bullets", 100);

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

static void draw_menu(const GameSystem* game_system) {
    al_clear_to_color(al_map_rgb(game_system->config.menu_bg_r, game_system->config.menu_bg_g, game_system->config.menu_bg_b));
    al_draw_text(game_system->font, al_map_rgb(game_system->config.text_r, game_system->config.text_g, game_system->config.text_b),
        game_system->config.buffer_width / 2, 100, ALLEGRO_ALIGN_CENTER, "Tank-Boy Game");
    draw_button(&game_system->start_button, &game_system->config, game_system->font);
    draw_button(&game_system->exit_button, &game_system->config, game_system->font);
}

// =================== Coordinate Conversion ===================

static void display_to_buffer_coords(int display_x, int display_y, int* buffer_x, int* buffer_y, const GameConfig* cfg) {
    *buffer_x = (int)(display_x / cfg->display_scale);
    *buffer_y = (int)(display_y / cfg->display_scale);
}

// =================== Game Initialization ===================

void init_game_system(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* queue, GameSystem* game_system) {
    al_init_font_addon();
    al_init_image_addon();
    al_init_primitives_addon();
    al_install_mouse();
    al_install_keyboard();

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());

    game_system->buffer = al_create_bitmap(game_system->config.buffer_width, game_system->config.buffer_height);
    game_system->font = al_create_builtin_font();

    int bx = game_system->config.buffer_width / 2 - game_system->config.button_width / 2;
    int sy = game_system->config.buffer_height / 2 - game_system->config.button_spacing / 2;
    int ey = game_system->config.buffer_height / 2 + game_system->config.button_spacing / 2;
    init_button(&game_system->start_button, bx, sy, game_system->config.button_width, game_system->config.button_height, "Start Game");
    init_button(&game_system->exit_button, bx, ey, game_system->config.button_width, game_system->config.button_height, "Exit Game");
    
    // Initialize Next button for stage clear screen (position will be set when needed)
    int next_y = game_system->config.buffer_height / 2 + 120;
    init_button(&game_system->next_button, bx, next_y, game_system->config.button_width, game_system->config.button_height, "Next Stage");
    
    // Initialize Menu button for game end screen
    init_button(&game_system->menu_button, bx, next_y, game_system->config.button_width, 
        game_system->config.button_height, "Back to Menu");

    game_system->current_state = STATE_MENU;
    game_system->running = true;
    game_system->current_stage = 1; // Initialize current stage first

    input_system_init(&game_system->input);
    
    // Load spawn points and initialize tank position
    char spawn_file[256];
    snprintf(spawn_file, sizeof(spawn_file), "TankBoy/resources/stages/spawns%d.csv", game_system->current_stage);
    
    double tank_x = 100.0; // Default position
    double tank_y = 2000.0; // Default position
    
    if (spawn_points_load(&game_system->spawn_points, spawn_file)) {
        SpawnPoint* tank_spawn = spawn_points_get_tank_spawn(&game_system->spawn_points);
        if (tank_spawn) {
            tank_x = (double)tank_spawn->x;
            tank_y = (double)tank_spawn->y;
            printf("Tank spawn loaded from file: (%.0f, %.0f)\n", tank_x, tank_y);
        }
    } else {
        printf("Using default tank spawn position: (%.0f, %.0f)\n", tank_x, tank_y);
    }
    
    tank_init(&game_system->player_tank, tank_x, tank_y);

    game_system->max_bullets = game_system->config.max_bullets;
    game_system->bullets = malloc(sizeof(Bullet) * game_system->max_bullets);
    bullets_init(game_system->bullets, game_system->max_bullets);

    game_system->camera_x = 0;
    game_system->camera_y = 0;

    head_up_display_init("config.ini");

    // Initialize and load map
    char map_file[256];
    snprintf(map_file, sizeof(map_file), "TankBoy/resources/stages/stage%d.csv", game_system->current_stage);
    if (!map_load(&game_system->current_map, map_file))
        map_init(&game_system->current_map);
    printf("location : %s\n", map_file);


    char map_sprite_file[256];
    snprintf(map_sprite_file, sizeof(map_sprite_file), "TankBoy/resources/sprites/stage1_ground.png");

    printf("location : %s\n", map_sprite_file);
    map_sprites_init(map_sprite_file);

    // Initialize enemy system
    game_system->round_number = 1;
    game_system->enemies_spawned = false;

    // flying enemy sprite load
    char flying_enemy_sprite_file[256];
    snprintf(flying_enemy_sprite_file, sizeof(flying_enemy_sprite_file), "TankBoy/resources/sprites/helicopter.png");

    printf("location : %s\n", flying_enemy_sprite_file);
    flying_enemy_sprites_init(flying_enemy_sprite_file);

    // enemy sprite load
    char enemy_sprite_file[256];
    snprintf(enemy_sprite_file, sizeof(enemy_sprite_file), "TankBoy/resources/sprites/enemy.png");

    printf("location : %s\n", enemy_sprite_file);
    enemy_sprites_init(enemy_sprite_file);
    
    // Set global references for getter functions
    set_global_tank_ref(&game_system->player_tank);
    set_global_bullet_ref(game_system->bullets, game_system->max_bullets);
    set_global_game_system(game_system);

    game_system->stage_clear = false;
    game_system->stage_clear_timer = 0.0;
    game_system->stage_clear_scale = 1.0;
    game_system->score = 0.0;  // Initialize score
    
    // Initialize game over system
    game_system->game_over = false;
    game_system->game_over_timer = 0.0;
    game_system->game_over_scale = 1.0;
}

// =================== Cleanup ===================

void cleanup_game_system(GameSystem* game_system, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_DISPLAY* display) {
    map_free(&game_system->current_map);
    spawn_points_free(&game_system->spawn_points);
    free(game_system->bullets);
    al_destroy_bitmap(game_system->buffer);
    al_destroy_font(game_system->font);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    map_sprites_deinit();
}

// =================== Input Handling ===================

static void handle_keyboard_input(ALLEGRO_EVENT* event, GameSystem* game_system) {
    if (event->type != ALLEGRO_EVENT_KEY_DOWN) return;

    switch (event->keyboard.keycode) {
    case ALLEGRO_KEY_ESCAPE:
        if (game_system->current_state == STATE_GAME) game_system->current_state = STATE_MENU;
        else game_system->running = false;
        break;
    // Removed U key force clear - now auto clears when all enemies defeated
    }
}

static void handle_mouse_input(ALLEGRO_EVENT* event, GameSystem* game_system) {
    int bx, by;
    switch (event->type) {
    case ALLEGRO_EVENT_MOUSE_AXES:
        display_to_buffer_coords(event->mouse.x, event->mouse.y, &bx, &by, &game_system->config);
        if (game_system->current_state == STATE_MENU) {
            game_system->start_button.hovered = is_point_in_button(bx, by, &game_system->start_button);
            game_system->exit_button.hovered = is_point_in_button(bx, by, &game_system->exit_button);
        }
        else if (game_system->current_state == STATE_GAME) {
            double cx = game_system->player_tank.x - game_system->camera_x + get_tank_width() / 2;
            double cy = game_system->player_tank.y - game_system->camera_y + get_tank_height() / 2;
            game_system->player_tank.cannon_angle = atan2(by - cy, bx - cx);
        }
        break;

    case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
        if (event->mouse.button != 1) break;
        display_to_buffer_coords(event->mouse.x, event->mouse.y, &bx, &by, &game_system->config);
        if (game_system->current_state == STATE_MENU) {
            if (is_point_in_button(bx, by, &game_system->start_button)) {
                // Initialize for new game start
                game_system->score = 0;
                game_system->current_stage = 1;
                char map_file[256];
                snprintf(map_file, sizeof(map_file), "TankBoy/resources/stages/stage%d.csv", game_system->current_stage);
                if (!map_load(&game_system->current_map, map_file))
                    map_init(&game_system->current_map);
                
                // Load spawn points for the new stage
                char spawn_file[256];
                snprintf(spawn_file, sizeof(spawn_file), "TankBoy/resources/stages/spawns%d.csv", game_system->current_stage);
                
                double tank_x = 100.0; // Default position
                double tank_y = 2000.0; // Default position
                
                // Free previous spawn points
                spawn_points_free(&game_system->spawn_points);
                
                if (spawn_points_load(&game_system->spawn_points, spawn_file)) {
                    SpawnPoint* tank_spawn = spawn_points_get_tank_spawn(&game_system->spawn_points);
                    if (tank_spawn) {
                        tank_x = (double)tank_spawn->x;
                        tank_y = (double)tank_spawn->y;
                        printf("Tank spawn loaded for stage %d: (%.0f, %.0f)\n", game_system->current_stage, tank_x, tank_y);
                    }
                } else {
                    printf("Using default tank spawn position for stage %d: (%.0f, %.0f)\n", game_system->current_stage, tank_x, tank_y);
                }
                
                tank_init(&game_system->player_tank, tank_x, tank_y);
                
                // Reset enemies for new game
                enemies_init();
                flying_enemies_init();
                game_system->enemies_spawned = false;
                
                game_system->current_state = STATE_GAME;
            }
            else if (is_point_in_button(bx, by, &game_system->exit_button)) game_system->running = false;
        }
        else if (game_system->current_state == STATE_GAME && game_system->stage_clear && game_system->stage_clear_timer < 0) {
            // Handle button clicks on stage clear/end screen
            if (game_system->current_stage >= 3) {
                // Game end screen - handle menu button
                if (is_point_in_button(bx, by, &game_system->menu_button)) {
                    game_system->current_state = STATE_MENU;
                    game_system->stage_clear = false;
                }
            } else {
                // Stage clear screen - handle next button
                if (is_point_in_button(bx, by, &game_system->next_button)) {
                    // Move to next stage
                    game_system->stage_clear = false;
                    game_system->current_stage++;

                    char map_file[256];
                    snprintf(map_file, sizeof(map_file), "TankBoy/resources/stages/stage%d.csv", game_system->current_stage);
                    if (!map_load(&game_system->current_map, map_file))
                        map_init(&game_system->current_map);

                    // Load spawn points for the new stage
                    char spawn_file[256];
                    snprintf(spawn_file, sizeof(spawn_file), "TankBoy/resources/stages/spawns%d.csv", game_system->current_stage);
                    
                    double tank_x = 100.0; // Default position
                    double tank_y = 2000.0; // Default position
                    
                    // Free previous spawn points
                    spawn_points_free(&game_system->spawn_points);
                    
                    if (spawn_points_load(&game_system->spawn_points, spawn_file)) {
                        SpawnPoint* tank_spawn = spawn_points_get_tank_spawn(&game_system->spawn_points);
                        if (tank_spawn) {
                            tank_x = (double)tank_spawn->x;
                            tank_y = (double)tank_spawn->y;
                            printf("Tank spawn loaded for stage %d: (%.0f, %.0f)\n", game_system->current_stage, tank_x, tank_y);
                        }
                    } else {
                        printf("Using default tank spawn position for stage %d: (%.0f, %.0f)\n", game_system->current_stage, tank_x, tank_y);
                    }
                    
                    tank_init(&game_system->player_tank, tank_x, tank_y);
                    
                    // Reset enemies for new stage
                    enemies_init();
                    flying_enemies_init();
                    game_system->enemies_spawned = false;
                }
            }
        }
        else if (game_system->current_state == STATE_GAME_OVER) {
            // Handle button clicks on game over screen
            if (is_point_in_button(bx, by, &game_system->menu_button)) {
                game_system->current_state = STATE_MENU;
                game_system->game_over = false;
            }
        }
        break;

    case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
        if (event->mouse.button == 1) {
            game_system->start_button.clicked = false;
            game_system->exit_button.clicked = false;
            game_system->next_button.clicked = false;
            game_system->menu_button.clicked = false;
        }
        break;
    }
}

// =================== Game Update ===================

void update_game_state(ALLEGRO_EVENT* event, GameSystem* game_system) {
    handle_keyboard_input(event, game_system);
    handle_mouse_input(event, game_system);
    input_system_update(&game_system->input, event);

    if (game_system->current_state != STATE_GAME) return;
    if (event->type != ALLEGRO_EVENT_TIMER) return;



    tank_update(&game_system->player_tank, &game_system->input, 1.0 / 60.0,
        game_system->bullets, game_system->max_bullets, (const Map*)&game_system->current_map);
    bullets_update(game_system->bullets, game_system->max_bullets, (const Map*)&game_system->current_map);

    // Check for game over condition
    if (get_tank_hp() <= 0 && !game_system->game_over) {
        game_system->game_over = true;
        game_system->game_over_timer = 0.0;
        game_system->game_over_scale = 1.0;
        game_system->current_state = STATE_GAME_OVER;
    }

    // Update camera to follow tank
    game_system->camera_x = game_system->player_tank.x - game_system->config.buffer_width / 3.0;
    game_system->camera_y = game_system->player_tank.y - game_system->config.buffer_height / 2.0;
    
    // Set camera position for HP bar drawing
    set_camera_position(game_system->camera_x, game_system->camera_y);
    
    // Spawn enemies if not spawned yet (but not during stage clear)
    if (!game_system->enemies_spawned && !game_system->stage_clear) {
        load_enemies_from_csv_with_map(game_system->current_stage, (const Map*)&game_system->current_map);
        // spawn_flying_enemy(game_system->round_number); // Removed: CSV already contains all enemies
        game_system->enemies_spawned = true;
    }
    
    // Update enemy systems with map reference
    enemies_update_roi_with_map(1.0/60.0, game_system->camera_x, game_system->camera_y, 
                      game_system->config.buffer_width, game_system->config.buffer_height, (const Map*)&game_system->current_map);
    flying_enemies_update_roi(1.0/60.0, game_system->camera_x, game_system->camera_y, 
                             game_system->config.buffer_width, game_system->config.buffer_height);
    
    // Update collision detection (only when not game over)
    if (!game_system->game_over) {
        bullets_hit_enemies();
        bullets_hit_tank();
        tank_touch_ground_enemy();
        tank_touch_flying_enemy();
    }
    
    // Check if all enemies are cleared for next round (but not during stage clear)
    int total_alive_enemies = get_alive_enemy_count() + get_alive_flying_enemy_count();
    if (total_alive_enemies == 0 && !game_system->stage_clear) {
        game_system->round_number++;
        game_system->enemies_spawned = false;
    }
    
    // HUD update only when not in Stage Clear!
    if (!game_system->stage_clear) {
        game_system->hud = head_up_display_update(
            (int)game_system->score,
            game_system->player_tank.weapon,
            game_system->current_stage
        );
        
        // Update HUD with enemy counts and round
        game_system->hud.enemies_alive = get_alive_enemy_count();
        game_system->hud.flying_enemies_alive = get_alive_flying_enemy_count();
        game_system->hud.round = game_system->round_number;
        game_system->hud.player_hp = get_tank_hp();
        game_system->hud.player_max_hp = get_tank_max_hp();
        
        // Auto stage clear when all enemies are defeated
        int total_enemies = game_system->hud.enemies_alive + game_system->hud.flying_enemies_alive;
        if (total_enemies == 0) {
            // Add health bonus when clearing stage
            int current_hp = get_tank_hp();
            int health_bonus = current_hp * 100;
            game_system->score += health_bonus;
            
            game_system->stage_clear = true;
            if (game_system->current_stage >= 3) {
                game_system->stage_clear_timer = -1.0; // Infinite wait for click
            } else {
                game_system->stage_clear_timer = -1.0; // Infinite wait for click
            }
            game_system->stage_clear_scale = 1.0;
        }
    }

    // Handle Stage Clear
    if (game_system->stage_clear) {
        // Only update timer if it's not waiting for click (-1.0)
        if (game_system->stage_clear_timer > 0) {
            game_system->stage_clear_timer -= 1.0 / 60.0;
        }
        
        // Always animate scale for visual effect
        static double animation_time = 0.0;
        animation_time += 1.0 / 60.0;
        game_system->stage_clear_scale = 1.0 + 0.2 * sin(animation_time * 3.14);
        
        // Auto advance only if timer runs out (for old U-key triggers)
        if (game_system->stage_clear_timer > 0 && game_system->stage_clear_timer <= 0) {
            // Stage 3 clear -> Game End processing
            if (game_system->current_stage >= 3) {
                game_system->current_state = STATE_MENU;
                game_system->stage_clear = false;
                return;
            }

            // Move to next stage
            game_system->stage_clear = false;
            game_system->current_stage++;

            char map_file[256];
            snprintf(map_file, sizeof(map_file), "TankBoy/resources/stages/stage%d.csv", game_system->current_stage);
            if (!map_load(&game_system->current_map, map_file))
                map_init(&game_system->current_map);

            tank_init(&game_system->player_tank, 50.0, 480.0);
        }
        // For new auto-clear, wait for click (timer = -1.0)
    }

    // Handle Game Over animation
    if (game_system->game_over) {
        game_system->game_over_timer += 1.0 / 60.0;
        
        // Animate scale for visual effect
        static double game_over_animation_time = 0.0;
        game_over_animation_time += 1.0 / 60.0;
        game_system->game_over_scale = 1.0 + 0.3 * sin(game_over_animation_time * 4.0);
    }
}

// =================== Rendering ===================

void disp_pre_draw(GameSystem* game_system) {
    al_set_target_bitmap(game_system->buffer);
}

void disp_post_draw(GameSystem* game_system) {
    al_set_target_backbuffer(al_get_current_display());
    double w = game_system->config.buffer_width * game_system->config.display_scale;
    double h = game_system->config.buffer_height * game_system->config.display_scale;
    al_draw_scaled_bitmap(game_system->buffer, 0, 0, game_system->config.buffer_width, game_system->config.buffer_height, 0, 0, w, h, 0);
    al_flip_display();
}

static void draw_game(const GameSystem* game_system) {
    al_clear_to_color(al_map_rgb(game_system->config.game_bg_r, game_system->config.game_bg_g, game_system->config.game_bg_b));

    map_draw((const Map*)&game_system->current_map, game_system->camera_x, game_system->camera_y, game_system->config.buffer_width, game_system->config.buffer_height);
    
    // Only draw tank and game elements when not game over
    if (!game_system->game_over) {
        tank_draw(&game_system->player_tank, game_system->camera_x, game_system->camera_y);
        enemies_draw(game_system->camera_x, game_system->camera_y);
        flying_enemies_draw(game_system->camera_x, game_system->camera_y);
        bullets_draw(game_system->bullets, game_system->max_bullets, game_system->camera_x, game_system->camera_y);
    }
    
    
    
    
    // Draw enemy HP bars (only when not game over)
    if (!game_system->game_over) {
        draw_enemy_hp_bars();
        draw_flying_enemy_hp_bars();
    }

    if (game_system->game_over) {
        // Game Over screen - draw over everything
        int cx = game_system->config.buffer_width / 2;
        int cy = game_system->config.buffer_height / 2;
        
        // Semi-transparent overlay
        al_draw_filled_rectangle(0, 0, game_system->config.buffer_width, game_system->config.buffer_height, 
                                al_map_rgba(0, 0, 0, 128));
        
        // Game Over text with animation
        al_draw_text(game_system->font, al_map_rgb(255, 0, 0), cx, cy - 60, ALLEGRO_ALIGN_CENTER, "GAME OVER");
        
        // Final score display
        char score_text[64];
        snprintf(score_text, sizeof(score_text), "Final Score: %d", (int)game_system->score);
        al_draw_text(game_system->font, al_map_rgb(255, 255, 255), cx, cy - 20, ALLEGRO_ALIGN_CENTER, score_text);
        
        // Stage reached
        char stage_text[64];
        snprintf(stage_text, sizeof(stage_text), "Stage Reached: %d", game_system->current_stage);
        al_draw_text(game_system->font, al_map_rgb(255, 255, 255), cx, cy + 20, ALLEGRO_ALIGN_CENTER, stage_text);
        
        // Back to Menu button
        draw_button(&game_system->menu_button, &game_system->config, game_system->font);
    }
    else if (!game_system->stage_clear) {
        head_up_display_draw(&game_system->hud);
    }
    else {
        int cx = game_system->config.buffer_width / 2;
        int cy = game_system->config.buffer_height / 2;

        if (game_system->current_stage >= 3) {  // Ending when Stage 3 is cleared
            al_draw_text(game_system->font, al_map_rgb(255, 0, 0), cx, cy - 20, ALLEGRO_ALIGN_CENTER, "Congratulations! You won the game!");
            
            // Show final score
            char score_text[64];
            snprintf(score_text, sizeof(score_text), "Final Score: %d", (int)game_system->score);
            al_draw_text(game_system->font, al_map_rgb(255, 255, 255), cx, cy + 20, ALLEGRO_ALIGN_CENTER, score_text);
            
            // Show health bonus info
            char health_bonus_text[64];
            int current_hp = get_tank_hp();
            int health_bonus = current_hp * 1000;
            snprintf(health_bonus_text, sizeof(health_bonus_text), "Health Bonus: +%d", health_bonus);
            al_draw_text(game_system->font, al_map_rgb(0, 255, 0), cx, cy + 50, ALLEGRO_ALIGN_CENTER, health_bonus_text);
            
            // Show Back to Menu button for game end
            if (game_system->stage_clear_timer < 0) {
                draw_button(&game_system->next_button, &game_system->config, game_system->font);
            }
        }
        else {
            al_draw_text(game_system->font, al_map_rgb(255, 255, 0), cx, cy, ALLEGRO_ALIGN_CENTER, "Stage Clear");
            
            // Show current score
            char score_text[64];
            snprintf(score_text, sizeof(score_text), "Score: %d", (int)game_system->score);
            al_draw_text(game_system->font, al_map_rgb(255, 255, 255), cx, cy + 40, ALLEGRO_ALIGN_CENTER, score_text);
            
            // Show health bonus info
            char health_bonus_text[64];
            int current_hp = get_tank_hp();
            int health_bonus = current_hp * 1000;
            snprintf(health_bonus_text, sizeof(health_bonus_text), "Health Bonus: +%d", health_bonus);
            al_draw_text(game_system->font, al_map_rgb(0, 255, 0), cx, cy + 70, ALLEGRO_ALIGN_CENTER, health_bonus_text);
            
            // Show Next button if waiting for click
            if (game_system->stage_clear_timer < 0) {
                draw_button(&game_system->next_button, &game_system->config, game_system->font);
            }
        }
    }
}

void render_game(GameSystem* game_system) {
    disp_pre_draw(game_system);
    if (game_system->current_state == STATE_MENU) draw_menu(game_system);
    else if (game_system->current_state == STATE_GAME || game_system->current_state == STATE_GAME_OVER) draw_game(game_system);
    disp_post_draw(game_system);
}

// ================= Score System =================

static GameSystem* global_game_system = NULL; // For enemy kill scoring

void set_global_game_system(GameSystem* gs) {
    global_game_system = gs;
}

void add_score_for_enemy_kill(int difficulty) {
    if (!global_game_system) return;
    
    int score_points = 0;
    switch (difficulty) {
        case 1: score_points = 500; break;
        case 2: score_points = 1000; break;
        case 3: score_points = 1500; break;
        default: score_points = 500; break; // Default to level 1 score
    }
    
    global_game_system->score += score_points;
}
