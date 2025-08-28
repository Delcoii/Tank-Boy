// standard c library
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

// allegro5 library
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>

// local library
#include "game_system.h"


void* must_init(void* test, const char* description) {
    if (test) return test;

    printf("couldn't initialize %s\n", description);
    exit(1);
    return NULL;
}


int main(void) {
    srand((unsigned int)time(NULL));
	al_init();
    
    // Load configuration first
    GameSystem game_system;
    load_game_config(&game_system.config, "config.ini");
    
    // Create display with scaled buffer size
    int disp_w = (int)(game_system.config.buffer_width * game_system.config.display_scale);
    int disp_h = (int)(game_system.config.buffer_height * game_system.config.display_scale);
    ALLEGRO_DISPLAY* display = must_init(al_create_display(disp_w, disp_h), "display");
    ALLEGRO_EVENT_QUEUE* queue = must_init(al_create_event_queue(), "event queue");
    
    // Create timer for 60 FPS
    ALLEGRO_TIMER* timer = must_init(al_create_timer(1.0 / 60.0), "timer");
    al_register_event_source(queue, al_get_timer_event_source(timer));
    
    // Initialize game system
    init_game_system(display, queue, &game_system);
    
    ALLEGRO_EVENT event;
    bool redraw = true;
    
    al_start_timer(timer);
    
    while (game_system.running) {
        al_wait_for_event(queue, &event);
        
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            game_system.running = false;
        }
        // only update display on timer event
        else if (event.type == ALLEGRO_EVENT_TIMER) {
            redraw = true;
        }
        
        // Handle all events (input, timer, etc.)
        update_game_state(&event, &game_system);

        if (redraw && al_is_event_queue_empty(queue)) {
            redraw = false;
            render_game(&game_system);
        }
    }
    
    al_destroy_timer(timer);
    

    cleanup_game_system(&game_system, queue, display);
    return 0;
}