#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include "mode_selection.h"

// Global variables
Button start_button;
Button exit_button;
ALLEGRO_FONT* font;
GameState current_state = STATE_MENU;



int main(void) {
    // Initialize Allegro
    if (!al_init()) {
        fprintf(stderr, "Allegro initialization failed\n");
        return -1;
    }
    
    // Initialize addons
    al_init_font_addon();
    al_init_primitives_addon();
    al_install_mouse();
    al_install_keyboard();
    
    // Create display
    ALLEGRO_DISPLAY* display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!display) {
        fprintf(stderr, "Display creation failed\n");
        return -1;
    }
    
    // Create event queue
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());
    
    // Initialize font
    font = al_create_builtin_font();
    
    // Initialize buttons
    init_button(&start_button, SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 50, 200, 50, "Start Game");
    init_button(&exit_button, SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 20, 200, 50, "Exit Game");
    
    printf("Tank-Boy game started!\n");
    printf("Click buttons with mouse to start game.\n");
    
    bool running = true;
    ALLEGRO_EVENT event;
    
    while (running) {
        al_wait_for_event(queue, &event);
        
        switch (event.type) {
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                running = false;
                break;
                
            case ALLEGRO_EVENT_KEY_DOWN:
                if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    if (current_state == STATE_GAME) {
                        current_state = STATE_MENU;
                        printf("Returning to menu.\n");
                    } else {
                        running = false;
                    }
                }
                break;
                
            case ALLEGRO_EVENT_MOUSE_AXES:
                if (current_state == STATE_MENU) {
                    // Update button hover states
                    start_button.hovered = is_point_in_button(event.mouse.x, event.mouse.y, &start_button);
                    exit_button.hovered = is_point_in_button(event.mouse.x, event.mouse.y, &exit_button);
                }
                break;
                
            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                if (event.mouse.button == 1 && current_state == STATE_MENU) { // Left mouse button
                    if (is_point_in_button(event.mouse.x, event.mouse.y, &start_button)) {
                        start_button.clicked = true;
                        current_state = STATE_GAME;
                        printf("Start Game button clicked!\n");
                    }
                    else if (is_point_in_button(event.mouse.x, event.mouse.y, &exit_button)) {
                        exit_button.clicked = true;
                        running = false;
                        printf("Exit Game button clicked!\n");
                    }
                }
                break;
                
            case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                if (event.mouse.button == 1) {
                    start_button.clicked = false;
                    exit_button.clicked = false;
                }
                break;
        }
        
        // Draw based on current state
        if (current_state == STATE_MENU) {
            draw_menu(&start_button, &exit_button, font);
        } else if (current_state == STATE_GAME) {
            draw_game(font);
        }
        
        al_flip_display();
    }
    
    // Cleanup
    al_destroy_font(font);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    
    printf("Game ended.\n");
    return 0;
}