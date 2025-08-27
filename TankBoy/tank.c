#if 0
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
#endif

#include "tank.h"
#include <math.h>
#include <allegro5/allegro_primitives.h>
#define M_PI 3.14159265358979323846

// Initialize tank
void tank_init(Tank* tank, float x, float y, ALLEGRO_COLOR color) {
    tank->x = x;
    tank->y = y;
    tank->cannon_angle = M_PI / 4;
    tank->on_ground = true;
    tank->angle = 0.0f;  // Facing right
    tank->speed = 100.0f;  // pixels per second
    tank->turn_speed = 2.0f;  // radians per second
    tank->width = 40;
    tank->height = 25;
    tank->color = color;
    tank->weapon = 0;

    tank->charging = false;
    tank->cannon_power = 0;

    tank->mg_firing = false;
    tank->mg_fire_time = 0;
    tank->mg_shot_cooldown = 0;
    tank->mg_reloading = false;
    tank->mg_reload_time = 0;
}


// Update tank based on input
void tank_update(Tank* tank, InputState* input, float map_width, float map_height) {
    float dt = 1.0f / 60.0f;  // Assuming 60 FPS
    const double friction = 0.85;
    const double gravity = 0.5;

    // Rotation
    if (input->left) {
        tank->angle -= tank->turn_speed * dt; // if you press a, you move to -x position
    }
    if (input->right) {
        tank->angle += tank->turn_speed * dt; // if you press d, you move to +x position
    }

    // Movement
    float dx = 0, dy = 0;
    if (input->jump && tank->on_ground) {
        dx = cosf(tank->angle) * tank->speed * dt;
        dy = sinf(tank->angle) * tank->speed * dt;
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

    // change weappon
    if (input->change_weapon) { tank->weapon = 1 - tank->weapon; input->change_weapon = false; }

    //canon charge
    if (tank->weapon == 1 && tank->charging) {
        tank->cannon_power += 0.2;
        if (tank->cannon_power > 15) tank->cannon_power = 15;
    }

    // m60 
    if (tank->weapon == 0) {
        if (tank->mg_shot_cooldown > 0) tank->mg_shot_cooldown -= dt;

        if (tank->mg_reloading) {
            tank->mg_reload_time -= dt;
            if (tank->mg_reload_time <= 0) {
                tank->mg_reloading = false;
                tank->mg_fire_time = 0;
                tank->mg_shot_cooldown = 0;
            }
        }
        else if (tank->mg_firing) {
            tank->mg_fire_time += dt;
            if (tank->mg_shot_cooldown <= 0) {
                // 발사는 main에서 bullet에 구현
                tank->mg_shot_cooldown = 0.1;
            }
            if (tank->mg_fire_time >= 3.0) {
                tank->mg_reloading = true;
                tank->mg_reload_time = 2.0;
                tank->mg_firing = false;
            }
        }
        else {
            tank->mg_fire_time = 0;
        }
    }
}

// Draw tank
void tank_draw(Tank* tank, double camera_x, double camera_y, Bullet* bullets, int max_bullets) {
    float half_width = tank->width / 2.0f;
    float half_height = tank->height / 2.0f;
    double sx = tank->x - camera_x;
    double sy = tank->y - camera_y;


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
    double cx = half_width + 16;
    double cy = half_height + 10;
    double bx = cx + cos(tank->cannon_angle) * 18;
    double by = cy + sin(tank->cannon_angle) * 18;
    al_draw_line(cx, cy, bx, by, al_map_rgb(50, 50, 50), 4);

    // Draw direction indicator (small triangle at front)
    al_draw_filled_triangle(
        half_width, 0,
        half_width - 8, -4,
        half_width - 8, 4,
        al_map_rgb(200, 200, 0)
    );


    // canon charge gauge
    if (tank->charging && tank->weapon == 1) {
        double gauge_w = tank->cannon_power * 10;
        al_draw_filled_rectangle(half_width, half_width, half_width + gauge_w, half_height - 10, al_map_rgb(255, 0, 0));
        al_draw_rectangle(half_width, half_height - 20, half_width + 150, half_height - 10, al_map_rgb(255, 255, 255), 2);
    }

    // m60 relaoding 
    if (tank->mg_reloading) {
        double total = 2.0;
        double filled = (total - tank->mg_reload_time) / total;
        if (filled < 0) filled = 0;
        if (filled > 1) filled = 1;
        double full_w = 150;
        double gw = full_w * filled;
        al_draw_rectangle(half_width, half_height - 35, half_width + full_w, half_height - 20, al_map_rgb(255, 255, 255), 2);
        al_draw_filled_rectangle(half_width + 1, half_height - 34, half_width + 1 + gw, half_height - 21, al_map_rgb(0, 200, 255));
    }
    // Restore transform
    al_use_transform(&transform);
}
