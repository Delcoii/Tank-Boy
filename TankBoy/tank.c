#include "tank.h"
#include "map_generation.h"
#include <math.h>
#include <allegro5/allegro_primitives.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Initialize tank
void tank_init(Tank* tank, double x, double y) {
    tank->x = x;
    tank->y = y;
    tank->vx = 0;
    tank->vy = 0;
    tank->on_ground = true;
    tank->cannon_angle = M_PI / 4;
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
void tank_update(Tank* tank, InputState* input, double dt, Bullet* bullets, int max_bullets, const struct Map* map) {
    const double accel = 0.4;
    const double maxspeed = 3.0;
    const double friction = 0.85;
    const double gravity = 0.5;

    // Movement with collision detection
    if (input->left) tank->vx -= accel;
    if (input->right) tank->vx += accel;
    tank->vx *= friction;
    if (tank->vx > maxspeed) tank->vx = maxspeed;
    if (tank->vx < -maxspeed) tank->vx = -maxspeed;
    
    // Check horizontal collision before moving
    double new_x = tank->x + tank->vx;
    if (map && map_rect_collision(map, (int)new_x, (int)tank->y, 32, 20)) {
        // Try to escape from block gaps by pushing upward
        bool escaped = false;
        for (int push_up = 1; push_up <= 10 && !escaped; push_up++) {
            double escape_y = tank->y - push_up;
            if (!map_rect_collision(map, (int)new_x, (int)escape_y, 32, 20)) {
                tank->x = new_x;
                tank->y = escape_y;
                tank->vy = -2;  // Small upward velocity to continue escaping
                escaped = true;
            }
        }
        
        // Try auto step-up for small obstacles (max 10px)
        if (!escaped && tank->on_ground) {
            for (int step_up = 1; step_up <= 10 && !escaped; step_up += 1) {
                double step_y = tank->y - step_up;
                if (!map_rect_collision(map, (int)new_x, (int)step_y, 32, 20)) {
                    // Check if there's solid ground to stand on
                    double ground_check_y = step_y + 1;
                    if (map_rect_collision(map, (int)new_x, (int)ground_check_y, 32, 20)) {
                        tank->x = new_x;
                        tank->y = step_y;
                        escaped = true;
                    }
                }
            }
        }
        
        if (!escaped) {
            tank->vx = 0;  // Stop horizontal movement if can't escape
        }
    } else {
        tank->x = new_x;
    }

    // Jump
    if (input->jump && tank->on_ground) { 
        tank->vy = -8; 
        tank->on_ground = false; 
    }
    tank->vy += gravity;
    
    // Check vertical collision before moving
    double new_y = tank->y + tank->vy;
    if (map && map_rect_collision(map, (int)tank->x, (int)new_y, 32, 20)) {
        if (tank->vy > 0) {  // Falling down, hit ground
            tank->vy = 0;
            tank->on_ground = true;
            // Improved ground alignment using tank's left edge for more stability
            int ground_level = map_get_ground_level(map, (int)tank->x);
            tank->y = ground_level - 20;  // Tank height is 20
        } else {  // Moving up, hit ceiling
            tank->vy = 0;
        }
    } else {
        tank->y = new_y;
        // Check if still on ground by testing a small area below tank
        if (map && map_rect_collision(map, (int)tank->x, (int)(tank->y + 21), 32, 1)) {
            tank->on_ground = true;
        } else {
            tank->on_ground = false;  // In air if no collision below
        }
    }
    
    // Fallback ground collision (if no map or below map bounds)
    if (!map || tank->y > 2140) {  // Map height - tank height
        tank->y = 2140;
        tank->vy = 0;
        tank->on_ground = true;
    }

    // Weapon change
    if (input->change_weapon) { 
        tank->weapon = 1 - tank->weapon; 
        input->change_weapon = false; 
    }
    
    // Fire input handling
    if (input->fire) {
        if (tank->weapon == 1) {
            tank->charging = true;
        } else if (!tank->mg_reloading) {
            tank->mg_firing = true;
        }
    } else {
        if (tank->weapon == 1 && tank->charging) {
            // Fire cannon
            for (int i = 0; i < max_bullets; i++) {
                if (!bullets[i].alive) {
                    bullets[i].alive = true;
                    bullets[i].x = tank->x + 16;
                    bullets[i].y = tank->y + 10;
                    bullets[i].weapon = 1;
                    bullets[i].vx = cos(tank->cannon_angle) * tank->cannon_power * 0.7;
                    bullets[i].vy = sin(tank->cannon_angle) * tank->cannon_power * 0.7;
                    break;
                }
            }
            tank->charging = false;
            tank->cannon_power = 0;
        } else if (tank->weapon == 0) {
            tank->mg_firing = false;
            tank->mg_fire_time = 0;
        }
    }

    // Cannon charging
    if (tank->weapon == 1 && tank->charging) {
        tank->cannon_power += 0.2;
        if (tank->cannon_power > 15) tank->cannon_power = 15;
    }

    // Machine gun firing
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
        else {
            if (tank->mg_firing) {
                tank->mg_fire_time += dt;
                if (tank->mg_shot_cooldown <= 0) {
                    // Fire bullet
                    for (int i = 0; i < max_bullets; i++) {
                        if (!bullets[i].alive) {
                            bullets[i].alive = true;
                            bullets[i].x = tank->x + 16;
                            bullets[i].y = tank->y + 10;
                            bullets[i].weapon = 0;
                            bullets[i].vx = cos(tank->cannon_angle) * 8.0 * 1.5;
                            bullets[i].vy = sin(tank->cannon_angle) * 8.0 * 1.5;
                            tank->mg_shot_cooldown = 0.1;
                            break;
                        }
                    }
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
}

// Draw tank
void tank_draw(Tank* tank, double camera_x, double camera_y) {
    double sx = tank->x - camera_x;
    double sy = tank->y - camera_y;

    // Tank body
    al_draw_filled_rectangle(sx, sy, sx + 32, sy + 20, al_map_rgb(60, 120, 180));

    // Cannon
    double cx = sx + 16;
    double cy = sy + 10;
    double bx = cx + cos(tank->cannon_angle) * 18;
    double by = cy + sin(tank->cannon_angle) * 18;
    al_draw_line(cx, cy, bx, by, al_map_rgb(200, 200, 0), 4);

    // Cannon charge gauge
    if (tank->charging && tank->weapon == 1) {
        double gauge_w = tank->cannon_power * 10;
        al_draw_filled_rectangle(sx, sy - 20, sx + gauge_w, sy - 10, al_map_rgb(255, 0, 0));
        al_draw_rectangle(sx, sy - 20, sx + 150, sy - 10, al_map_rgb(255, 255, 255), 2);
    }

    // Machine gun reload gauge
    if (tank->mg_reloading) {
        double total = 2.0;
        double filled = (total - tank->mg_reload_time) / total;
        if (filled < 0) filled = 0;
        if (filled > 1) filled = 1;
        double full_w = 150;
        double gw = full_w * filled;
        al_draw_rectangle(sx, sy - 35, sx + full_w, sy - 20, al_map_rgb(255, 255, 255), 2);
        al_draw_filled_rectangle(sx + 1, sy - 34, sx + 1 + gw, sy - 21, al_map_rgb(0, 200, 255));
    }
}