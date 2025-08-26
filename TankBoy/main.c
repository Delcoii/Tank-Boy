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

void must_init(bool test, const char* description) {
    if (test) return;

    printf("couldn't initialize %s\n", description);
    exit(1);
}


int main(void) {
    srand((unsigned int)time(NULL));
	must_init(al_init(), "allegro");
    

    ALLEGRO_DISPLAY* display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_EVENT event;


    GameSystem game_system;
    init_game_system(display, queue, &game_system);
    
    
    while (game_system.running) {
        al_wait_for_event(queue, &event);
        
        // Handle display close
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