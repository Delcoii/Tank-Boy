#include "tank.h"
#include <math.h>


// Initialize tank
void tank_init(Tank* tank, float x, float y, ALLEGRO_COLOR color) {
    tank->x = x;
    tank->y = y;
    tank->angle = 0.0f;  // Facing right
    tank->speed = 100.0f;  // pixels per second
    tank->turn_speed = 2.0f;  // radians per second
    tank->width = 40;
    tank->height = 25;
    tank->color = color;
}


// Update tank based on input
void tank_update(Tank* tank, InputState* input, float map_width, float map_height) {
    float dt = 1.0f / 60.0f;  // Assuming 60 FPS
    
    // Rotation
    if (input->key_left) {
        tank->angle -= tank->turn_speed * dt;
    }
    if (input->key_right) {
        tank->angle += tank->turn_speed * dt;
    }
    
    // Movement
    float dx = 0, dy = 0;
    if (input->key_up) {
        dx = cosf(tank->angle) * tank->speed * dt;
        dy = sinf(tank->angle) * tank->speed * dt;
    }
    if (input->key_down) {
        dx = -cosf(tank->angle) * tank->speed * dt;
        dy = -sinf(tank->angle) * tank->speed * dt;
    }
    
    // Update position with boundary checking
    tank->x += dx;
    tank->y += dy;
    
    // Keep tank within map bounds
    float half_width = tank->width / 2.0f;
    float half_height = tank->height / 2.0f;
    
    if (tank->x - half_width < 0) tank->x = half_width;
    if (tank->x + half_width > map_width) tank->x = map_width - half_width;
    if (tank->y - half_height < 0) tank->y = half_height;
    if (tank->y + half_height > map_height) tank->y = map_height - half_height;
}

// Draw tank
void tank_draw(Tank* tank) {
    float half_width = tank->width / 2.0f;
    float half_height = tank->height / 2.0f;
    
    // Save current transform
    ALLEGRO_TRANSFORM transform, identity;
    al_copy_transform(&transform, al_get_current_transform());
    al_identity_transform(&identity);
    al_use_transform(&identity);
    
    // Translate to tank position and rotate
    al_translate_transform(&identity, tank->x, tank->y);
    al_rotate_transform(&identity, tank->angle);
    al_use_transform(&identity);
    
    // Draw tank body (rectangle)
    al_draw_filled_rectangle(-half_width, -half_height, half_width, half_height, tank->color);
    al_draw_rectangle(-half_width, -half_height, half_width, half_height, al_map_rgb(0, 0, 0), 2);
    
    // Draw tank cannon (line pointing forward)
    al_draw_line(0, 0, half_width + 10, 0, al_map_rgb(50, 50, 50), 4);
    
    // Draw direction indicator (small triangle at front)
    al_draw_filled_triangle(
        half_width, 0,
        half_width - 8, -4,
        half_width - 8, 4,
        al_map_rgb(200, 200, 0)
    );
    
    // Restore transform
    al_use_transform(&transform);
}
