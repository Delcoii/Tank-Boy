#include "mode_selection.h"
#include <stdio.h>

// Button initialization
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

// Draw button
void draw_button(Button* button, ALLEGRO_FONT* font) {
    ALLEGRO_COLOR bg_color, text_color;
    
    // Button color based on state
    if (button->clicked) {
        bg_color = al_map_rgb(100, 150, 200);
        text_color = al_map_rgb(255, 255, 255);
    }
    else if (button->hovered) {
        bg_color = al_map_rgb(80, 120, 180);
        text_color = al_map_rgb(255, 255, 255);
    }
    else {
        bg_color = al_map_rgb(60, 100, 160);
        text_color = al_map_rgb(255, 255, 255);
    }
    
    // Draw button background
    al_draw_filled_rounded_rectangle(button->x, button->y,
                                   button->x + button->width, button->y + button->height,
                                   5, 5, bg_color);
    
    // Draw button border
    al_draw_rounded_rectangle(button->x, button->y,
                            button->x + button->width, button->y + button->height,
                            5, 5, al_map_rgb(255, 255, 255), 2);
    
    // Draw button text
    al_draw_text(font, text_color,
                 button->x + button->width/2, button->y + button->height/2 - 8,
                 ALLEGRO_ALIGN_CENTER, button->text);
}

// Draw menu screen
void draw_menu(Button* start_button, Button* exit_button, ALLEGRO_FONT* font) {
    // Clear screen
    al_clear_to_color(al_map_rgb(20, 20, 40));
    
    // Draw title
    al_draw_text(font, al_map_rgb(255, 255, 255),
                 SCREEN_WIDTH/2, 100, ALLEGRO_ALIGN_CENTER,
                 "TANK BOY");
    
    // Draw subtitle
    al_draw_text(font, al_map_rgb(200, 200, 200),
                 SCREEN_WIDTH/2, 150, ALLEGRO_ALIGN_CENTER,
                 "Click button to start game");
    
    // Draw buttons
    draw_button(start_button, font);
    draw_button(exit_button, font);
}

// Draw game screen
void draw_game(ALLEGRO_FONT* font) {
    // Clear screen
    al_clear_to_color(al_map_rgb(0, 50, 0));
    
    // Draw game text
    al_draw_text(font, al_map_rgb(255, 255, 255),
                 SCREEN_WIDTH/2, SCREEN_HEIGHT/2, ALLEGRO_ALIGN_CENTER,
                 "Game is running!\nPress ESC to return to menu");
}
