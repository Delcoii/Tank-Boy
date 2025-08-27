#include "game_system.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Load game configuration from INI file
void load_game_config(GameConfig* config, const char* config_file) {
    IniParser* parser = ini_parser_create();
    
    if (!parser) {
        printf("Error: Could not create INI parser\n");
        return;
    }
    
    // Create full path based on source file location
    char full_path[512];
    const char* source_file = __FILE__;  // This file's path
    
    // Find the last directory separator
    const char* last_slash = strrchr(source_file, '\\');
    if (!last_slash) last_slash = strrchr(source_file, '/');
    
    if (last_slash) {
        // Copy directory part
        size_t dir_len = last_slash - source_file + 1;
        if (dir_len < sizeof(full_path)) {
#pragma warning(push)
#pragma warning(disable: 4996)
            strncpy(full_path, source_file, dir_len);
            full_path[dir_len] = '\0';
            
            // Append config filename
            strcat(full_path, config_file);
#pragma warning(pop)
        } else {
            strcpy_s(full_path, sizeof(full_path), config_file);
        }
    } else {
        // No directory found, use relative path
        strcpy_s(full_path, sizeof(full_path), config_file);
    }
    
    // Load with default values as fallback
    if (ini_parser_load_file(parser, full_path)) {
        printf("Configuration loaded from '%s'\n", full_path);
    } else {
        printf("Warning: Could not load config file '%s', using defaults\n", full_path);
    }
    
    // Load all values with defaults (whether file exists or not)
    config->display_width = ini_parser_get_int(parser, "Display", "display_width", 800);
    config->display_height = ini_parser_get_int(parser, "Display", "display_height", 600);
    config->button_width = ini_parser_get_int(parser, "Buttons", "button_width", 200);
    config->button_height = ini_parser_get_int(parser, "Buttons", "button_height", 50);
    config->button_spacing = ini_parser_get_int(parser, "Buttons", "button_spacing", 70);
    
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
    
    config->game_speed = ini_parser_get_int(parser, "Game", "game_speed", 60);
    config->max_lives = ini_parser_get_int(parser, "Game", "max_lives", 3);
    
    ini_parser_destroy(parser);
}

// Initialize button
void init_button(Button* button, int x, int y, int width, int height, char* text) {
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->text = text;
    button->hovered = false;
    button->clicked = false;
}

// Convert display coordinates to buffer coordinates
void display_to_buffer_coords(int display_x, int display_y, int* buffer_x, int* buffer_y) {
    *buffer_x = (display_x * BUFFER_W) / DISP_W;
    *buffer_y = (display_y * BUFFER_H) / DISP_H;
}

// Check if point is inside button
bool is_point_in_button(int x, int y, Button* button) {
    return (x >= button->x && x <= button->x + button->width &&
            y >= button->y && y <= button->y + button->height);
}

// Draw a button
void draw_button(Button* button, ALLEGRO_FONT* font, GameConfig* config) {
    ALLEGRO_COLOR bg_color, text_color;
    
    if (button->clicked) {
        bg_color = al_map_rgb(config->button_clicked_r, config->button_clicked_g, config->button_clicked_b);
        text_color = al_map_rgb(config->text_r, config->text_g, config->text_b);
    } else if (button->hovered) {
        bg_color = al_map_rgb(config->button_hover_r, config->button_hover_g, config->button_hover_b);
        text_color = al_map_rgb(config->text_r, config->text_g, config->text_b);
    } else {
        bg_color = al_map_rgb(config->button_normal_r, config->button_normal_g, config->button_normal_b);
        text_color = al_map_rgb(0, 0, 0);
    }
    
    al_draw_filled_rectangle(button->x, button->y, 
                            button->x + button->width, button->y + button->height, bg_color);
    al_draw_rectangle(button->x, button->y, 
                     button->x + button->width, button->y + button->height, 
                     al_map_rgb(0, 0, 0), 2);
    
    al_draw_text(font, text_color,
                 button->x + button->width/2, button->y + button->height/2 - 8,
                 ALLEGRO_ALIGN_CENTER, button->text);
}

// Draw main menu screen
void draw_menu(Button* start_button, Button* exit_button, ALLEGRO_FONT* font, GameConfig* config) {
    al_clear_to_color(al_map_rgb(config->menu_bg_r, config->menu_bg_g, config->menu_bg_b));
    
    al_draw_text(font, al_map_rgb(config->text_r, config->text_g, config->text_b),
                 BUFFER_W/2, 100, ALLEGRO_ALIGN_CENTER,
                 "Tank-Boy Game");
    
    draw_button(start_button, font, config);
    draw_button(exit_button, font, config);
}

// Draw game screen
void draw_game(ALLEGRO_FONT* font, GameConfig* config, Tank* player_tank) {
    al_clear_to_color(al_map_rgb(config->game_bg_r, config->game_bg_g, config->game_bg_b));
    
    // Draw tank
    tank_draw(player_tank);
    
    // Draw UI
    al_draw_text(font, al_map_rgb(config->text_r, config->text_g, config->text_b),
                 10, 10, ALLEGRO_ALIGN_LEFT,
                 "Use WASD or Arrow Keys to move, ESC to return to menu");
}

// Handle keyboard input
bool handle_keyboard_input(ALLEGRO_EVENT* event, GameSystem* game_system) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {

        // when user press ESC key
        if (event->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            // Return to menu
            if (game_system->current_state == STATE_GAME) {
                game_system->current_state = STATE_MENU;
            } else {
                // Exit game
                game_system->running = false;
            }
            return true;
        }
    }
    return false;
}

// Handle mouse input
void handle_mouse_input(ALLEGRO_EVENT* event, GameSystem* game_system) {
    int buffer_x, buffer_y;
    
    switch (event->type) {
        case ALLEGRO_EVENT_MOUSE_AXES:
            if (game_system->current_state == STATE_MENU) {
                // Convert display coordinates to buffer coordinates
                display_to_buffer_coords(event->mouse.x, event->mouse.y, &buffer_x, &buffer_y);
                
                // Update button hover states
                game_system->start_button.hovered = is_point_in_button(buffer_x, buffer_y, &game_system->start_button);
                game_system->exit_button.hovered = is_point_in_button(buffer_x, buffer_y, &game_system->exit_button);
            }
            break;
            
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == 1 && game_system->current_state == STATE_MENU) { // Left mouse button
                // Convert display coordinates to buffer coordinates
                display_to_buffer_coords(event->mouse.x, event->mouse.y, &buffer_x, &buffer_y);
                
                if (is_point_in_button(buffer_x, buffer_y, &game_system->start_button)) {
                    game_system->start_button.clicked = true;
                    game_system->current_state = STATE_GAME;
                    printf("Start Game button clicked! (display: %d,%d -> buffer: %d,%d)\n", 
                           event->mouse.x, event->mouse.y, buffer_x, buffer_y);
                }
                else if (is_point_in_button(buffer_x, buffer_y, &game_system->exit_button)) {
                    game_system->exit_button.clicked = true;
                    game_system->running = false;
                    printf("Exit Game button clicked! (display: %d,%d -> buffer: %d,%d)\n", 
                           event->mouse.x, event->mouse.y, buffer_x, buffer_y);
                }
            }
            break;
            
        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            if (event->mouse.button == 1) {
                game_system->start_button.clicked = false;
                game_system->exit_button.clicked = false;
            }
            break;
    }
}

// Initialize game system
void init_game_system(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* queue, GameSystem* game_system) {
    // Initialize addons
    al_init_font_addon();
    al_init_primitives_addon();
    al_install_mouse();
    al_install_keyboard();
    
    // Register event sources
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());
    
    // Initialize display buffer
    game_system->buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
    if (!game_system->buffer) {
        printf("Error: Could not create display buffer\n");
        exit(1);
    }
    
    // Initialize font
    game_system->font = al_create_builtin_font();
    
    // Initialize buttons using buffer dimensions
    int button_x = BUFFER_W/2 - game_system->config.button_width/2;
    int start_y = BUFFER_H/2 - game_system->config.button_spacing/2;
    int exit_y = BUFFER_H/2 + game_system->config.button_spacing/2;
    
    init_button(&game_system->start_button, button_x, start_y, 
                game_system->config.button_width, game_system->config.button_height, "Start Game");
    init_button(&game_system->exit_button, button_x, exit_y, 
                game_system->config.button_width, game_system->config.button_height, "Exit Game");
    
    // Initialize game state
    game_system->current_state = STATE_MENU;
    game_system->running = true;
    
    // Initialize input system
    input_system_init(&game_system->input);
    
    // Initialize player tank at center of buffer
    tank_init(&game_system->player_tank, 
              BUFFER_W / 2.0f, 
              BUFFER_H / 2.0f,
              al_map_rgb(0, 150, 0));  // Green tank
    
    printf("Game system initialized with double buffering!\n");
}

// Cleanup game system
void cleanup_game_system(GameSystem* game_system, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_DISPLAY* display) {
    al_destroy_bitmap(game_system->buffer);
    al_destroy_font(game_system->font);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    printf("Game system cleaned up.\n");
}


// Update game state based on events
void update_game_state(ALLEGRO_EVENT* event, GameSystem* game_system) {
    // Handle keyboard input
    handle_keyboard_input(event, game_system);
    
    // Handle mouse input
    handle_mouse_input(event, game_system);
    
    // Update input system
    input_system_update(&game_system->input, event);
    
    // Update game objects if in game state
    if (game_system->current_state == STATE_GAME) {
        tank_update(&game_system->player_tank, &game_system->input, 
                   BUFFER_W, BUFFER_H);
    }
}

// Set target to buffer for drawing
void disp_pre_draw(GameSystem* game_system) {
    al_set_target_bitmap(game_system->buffer);
}

// Scale buffer to display and flip
void disp_post_draw(GameSystem* game_system) {
    al_set_target_backbuffer(al_get_current_display());
    al_draw_scaled_bitmap(game_system->buffer, 0, 0, BUFFER_W, BUFFER_H, 
                          0, 0, DISP_W, DISP_H, 0);
    al_flip_display();
}

// Render game based on current state
void render_game(GameSystem* game_system) {
    disp_pre_draw(game_system);
    
    if (game_system->current_state == STATE_MENU) {
        draw_menu(&game_system->start_button, &game_system->exit_button, game_system->font, &game_system->config);
    } else if (game_system->current_state == STATE_GAME) {
        draw_game(game_system->font, &game_system->config, &game_system->player_tank);
    }
    
    disp_post_draw(game_system);
}