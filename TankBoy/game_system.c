#include "game_system.h"
#include <stdio.h>

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

// Check if point is inside button
bool is_point_in_button(int x, int y, Button* button) {
    return (x >= button->x && x <= button->x + button->width &&
            y >= button->y && y <= button->y + button->height);
}

// Draw a button
void draw_button(Button* button, ALLEGRO_FONT* font) {
    ALLEGRO_COLOR bg_color, text_color;
    
    if (button->clicked) {
        bg_color = al_map_rgb(100, 100, 100);
        text_color = al_map_rgb(200, 200, 200);
    } else if (button->hovered) {
        bg_color = al_map_rgb(150, 150, 150);
        text_color = al_map_rgb(255, 255, 255);
    } else {
        bg_color = al_map_rgb(200, 200, 200);
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
void draw_menu(Button* start_button, Button* exit_button, ALLEGRO_FONT* font) {
    al_clear_to_color(al_map_rgb(50, 50, 100));
    
    al_draw_text(font, al_map_rgb(255, 255, 255),
                 SCREEN_WIDTH/2, 100, ALLEGRO_ALIGN_CENTER,
                 "Tank-Boy Game");
    
    draw_button(start_button, font);
    draw_button(exit_button, font);
}

// Draw game screen
void draw_game(ALLEGRO_FONT* font) {
    al_clear_to_color(al_map_rgb(0, 100, 0));
    
    al_draw_text(font, al_map_rgb(255, 255, 255),
                 SCREEN_WIDTH/2, SCREEN_HEIGHT/2, ALLEGRO_ALIGN_CENTER,
                 "Game is running!\nPress ESC to return to menu");
}

// Handle keyboard input
bool handle_keyboard_input(ALLEGRO_EVENT* event, GameSystem* game_system) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        if (event->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            if (game_system->current_state == STATE_GAME) {
                game_system->current_state = STATE_MENU;
                printf("Returning to menu.\n");
            } else {
                game_system->running = false;
            }
            return true; // Event was handled
        }
    }
    return false; // Event was not handled
}

// Handle mouse input
void handle_mouse_input(ALLEGRO_EVENT* event, GameSystem* game_system) {
    switch (event->type) {
        case ALLEGRO_EVENT_MOUSE_AXES:
            if (game_system->current_state == STATE_MENU) {
                // Update button hover states
                game_system->start_button.hovered = is_point_in_button(event->mouse.x, event->mouse.y, &game_system->start_button);
                game_system->exit_button.hovered = is_point_in_button(event->mouse.x, event->mouse.y, &game_system->exit_button);
            }
            break;
            
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == 1 && game_system->current_state == STATE_MENU) { // Left mouse button
                if (is_point_in_button(event->mouse.x, event->mouse.y, &game_system->start_button)) {
                    game_system->start_button.clicked = true;
                    game_system->current_state = STATE_GAME;
                    printf("Start Game button clicked!\n");
                }
                else if (is_point_in_button(event->mouse.x, event->mouse.y, &game_system->exit_button)) {
                    game_system->exit_button.clicked = true;
                    game_system->running = false;
                    printf("Exit Game button clicked!\n");
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
    
    // Initialize font
    game_system->font = al_create_builtin_font();
    
    // Initialize buttons
    init_button(&game_system->start_button, SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 50, 200, 50, "Start Game");
    init_button(&game_system->exit_button, SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 20, 200, 50, "Exit Game");
    
    // Initialize game state
    game_system->current_state = STATE_MENU;
    game_system->running = true;
    
    printf("Game system initialized!\n");
}

// Cleanup game system
void cleanup_game_system(GameSystem* game_system, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_DISPLAY* display) {
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
}

// Render game based on current state
void render_game(GameSystem* game_system) {
    if (game_system->current_state == STATE_MENU) {
        draw_menu(&game_system->start_button, &game_system->exit_button, game_system->font);
    } else if (game_system->current_state == STATE_GAME) {
        draw_game(game_system->font);
    }
}