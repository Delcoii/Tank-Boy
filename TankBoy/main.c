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
    ALLEGRO_DISPLAY* display = must_init(al_create_display(DISP_W, DISP_H), "display");
    ALLEGRO_EVENT_QUEUE* queue = must_init(al_create_event_queue(), "event queue");
    
    // Initialize game system
    init_game_system(display, queue, &game_system);
    
    ALLEGRO_EVENT event;
    
    while (game_system.running) {
        al_wait_for_event(queue, &event);
        
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            game_system.running = false;
        }
        
        update_game_state(&event, &game_system);
        render_game(&game_system);
        
        al_flip_display();
    }
    

    cleanup_game_system(&game_system, queue, display);
    return 0;
}