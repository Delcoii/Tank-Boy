#include "tank.h"
#include "map_generation.h"
#include "ini_parser.h"
#include "audio.h"
#include <math.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


typedef struct SPRITES_TANK {
    ALLEGRO_BITMAP* tank_base;
    ALLEGRO_BITMAP* _sheet;
     ALLEGRO_BITMAP* fliped_sheet;
    //ALLEGRO_BITMAP* tank_cannon;
} SPRITES_TANK;
SPRITES_TANK tank_sprites;
   
// Remove global tank size variables - now stored in Tank struct

// Initialize tank
void tank_init(Tank* tank, double x, double y) {
    // Load tank size from config at initialization
    IniParser* parser = ini_parser_create();
    ini_parser_load_file(parser, "TankBoy/config.ini");
    tank->width = ini_parser_get_int(parser, "Tank", "tank_width", 32);
    tank->height = ini_parser_get_int(parser, "Tank", "tank_height", 20);
    // Tank size loaded from config
    ini_parser_destroy(parser);
    
    tank->x = x;
    tank->y = y;
    tank->vx = 0;
    tank->vy = 0;
    tank->on_ground = true;
    tank->cannon_angle = M_PI / 4;
    tank->weapon = 0;

    // Initialize HP and invincibility
    tank->max_hp = 100;
    tank->hp = tank->max_hp;
    tank->invincible = 0.0;

    tank->charging = false;
    tank->cannon_power = 0;

    tank->mg_firing = false;
    tank->mg_fire_time = 0;
    tank->mg_shot_cooldown = 0;
    tank->mg_reloading = false;
    tank->mg_reload_time = 0;
}


// Update tank based on input
void tank_update(Tank* tank, InputState* input, double dt, Bullet* bullets, int max_bullets, const Map* map) {
    // Update invincibility timer
    if (tank->invincible > 0.0) {
        tank->invincible -= dt;
        if (tank->invincible < 0.0) tank->invincible = 0.0;
    }

    if (tank->vx > 0) tank->facing_right = true;
    else if (tank->vx < 0) tank->facing_right = false;
    
    // Load physics settings from config.ini
    IniParser* parser = ini_parser_create();
    ini_parser_load_file(parser, "TankBoy/config.ini");
    const double accel = ini_parser_get_double(parser, "Tank", "tank_acceleration", 0.5);
    const double maxspeed = ini_parser_get_double(parser, "Tank", "tank_max_speed", 5.0);
    const double friction = ini_parser_get_double(parser, "Tank", "tank_friction", 0.85);
    const double gravity = ini_parser_get_double(parser, "Tank", "tank_gravity", 0.3);
    const double jump_power = ini_parser_get_double(parser, "Tank", "tank_jump_power", 8.0);
    tank->width = ini_parser_get_int(parser, "Tank", "tank_width", 32);
    tank->height = ini_parser_get_int(parser, "Tank", "tank_height", 20);
    // Tank size updated from config
    const int tank_width = tank->width;
    const int tank_height = tank->height;
    const int max_step_height = ini_parser_get_int(parser, "Tank", "max_step_height", 10);
    const int max_escape_height = ini_parser_get_int(parser, "Tank", "max_escape_height", 10);
    const double escape_velocity = ini_parser_get_double(parser, "Tank", "escape_velocity", 2.0);
    const int map_height = map_get_map_height(); // Use function instead of hardcoded value
    ini_parser_destroy(parser);

    // Movement with collision detection
    if (input->left) tank->vx -= accel;
    if (input->right) tank->vx += accel;
    tank->vx *= friction;
    if (tank->vx > maxspeed) tank->vx = maxspeed;
    if (tank->vx < -maxspeed) tank->vx = -maxspeed;
    
    // Check horizontal collision before moving
    double new_x = tank->x + tank->vx;
    if (map && map_rect_collision(map, (int)new_x, (int)tank->y, tank_width, tank_height)) {
        // Try to escape from block gaps by pushing upward
        bool escaped = false;
        for (int push_up = 1; push_up <= max_escape_height && !escaped; push_up++) {
            double escape_y = tank->y - push_up;
            if (!map_rect_collision(map, (int)new_x, (int)escape_y, tank_width, tank_height)) {
                tank->x = new_x;
                tank->y = escape_y;
                tank->vy = -escape_velocity;  // Small upward velocity to continue escaping
                escaped = true;
            }
        }
        
        // Try auto step-up for small obstacles
        if (!escaped && tank->on_ground) {
            for (int step_up = 1; step_up <= max_step_height && !escaped; step_up += 1) {
                double step_y = tank->y - step_up;
                if (!map_rect_collision(map, (int)new_x, (int)step_y, tank_width, tank_height)) {
                    // Check if there's solid ground to stand on
                    double ground_check_y = step_y + 1;
                    if (map_rect_collision(map, (int)new_x, (int)ground_check_y, tank_width, tank_height)) {
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
        tank->vy = -jump_power; 
        tank->on_ground = false; 
    }
    tank->vy += gravity;
    
    // Check vertical collision before moving
    double new_y = tank->y + tank->vy;
    if (map && map_rect_collision(map, (int)tank->x, (int)new_y, tank_width, tank_height)) {
        if (tank->vy > 0) {  // Falling down, hit ground
            tank->vy = 0;
            tank->on_ground = true;
            // Improved ground alignment using tank's left edge for more stability
            int ground_level = map_get_ground_level(map, (int)tank->x, tank_width);
            tank->y = ground_level - tank_height;  // Tank height from config
        } else {  // Moving up, hit ceiling
            tank->vy = 0;
        }
    } else {
        tank->y = new_y;
        // Check if still on ground by testing a small area below tank
        if (map && map_rect_collision(map, (int)tank->x, (int)(tank->y + tank_height + 1), tank_width, 1)) {
            tank->on_ground = true;
        } else {
            tank->on_ground = false;  // In air if no collision below
        }
    }
    
    // Fallback ground collision (if no map or below map bounds)
    if (!map || tank->y > map_height - tank_height) {  // Map height - tank height
        tank->y = map_height - tank_height;
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
                    // Load cannon bullet size from config
                    IniParser* parser = ini_parser_create();
                    ini_parser_load_file(parser, "TankBoy/config.ini");
                    int cannon_width = ini_parser_get_int(parser, "Bullets", "cannon_bullet_width", 20);
                    int cannon_height = ini_parser_get_int(parser, "Bullets", "cannon_bullet_height", 20);
                    ini_parser_destroy(parser);
                    
                    bullets[i].alive = true;
                    bullets[i].x = tank->x + tank->width / 2;
                    bullets[i].y = tank->y + tank->height / 2;
                    bullets[i].weapon = 1;
                    bullets[i].vx = cos(tank->cannon_angle) * tank->cannon_power * 1.4;
                    bullets[i].vy = sin(tank->cannon_angle) * tank->cannon_power * 1.4;
                    bullets[i].width = cannon_width;
                    bullets[i].height = cannon_height;
                                         bullets[i].angle = tank->cannon_angle;
                     bullets[i].from_enemy = false;
                     // Debug output removed
                     
                     // Play cannon sound effect
                     play_cannon_sound();
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
                            // Load MG bullet size from config
                            IniParser* parser = ini_parser_create();
                            ini_parser_load_file(parser, "config.ini");
                            int mg_width = ini_parser_get_int(parser, "Bullets", "mg_bullet_width", 40);
                            int mg_height = ini_parser_get_int(parser, "Bullets", "mg_bullet_height", 10);
                            ini_parser_destroy(parser);
                            
                            bullets[i].alive = true;
                            bullets[i].x = tank->x + tank->width / 2;
                            bullets[i].y = tank->y + tank->height / 2;
                            bullets[i].weapon = 0;
                            bullets[i].vx = cos(tank->cannon_angle) * 8.0 * 1.5;
                            bullets[i].vy = sin(tank->cannon_angle) * 8.0 * 1.5;
                            bullets[i].width = mg_width;
                            bullets[i].height = mg_height;
                            bullets[i].angle = tank->cannon_angle;
                            bullets[i].from_enemy = false;
                            // Debug output removed
                            tank->mg_shot_cooldown = 0.1;
                            
                            // Play machine gun sound effect
                            play_machine_sound();
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
    // Tank drawing with dynamic size
    
    double sx = tank->x - camera_x;
    double sy = tank->y - camera_y;

    ALLEGRO_BITMAP* sprite = tank->facing_right ? tank_sprites.fliped_sheet : tank_sprites._sheet;
    al_draw_scaled_bitmap(sprite, 0, 0, 1024, 793, sx, sy, tank->width, tank->height, 0);
   // al_draw_scaled_bitmap(tank_sprites.tank_base, 0, 0, 1024, 793, sx, sy, tank->width, tank->height, 0);

    // Cannon
    double cx = sx + tank->width / 2;
    double cy = sy + tank->height / 2;
    double bx = cx + cos(tank->cannon_angle) * 18;
    double by = cy + sin(tank->cannon_angle) * 18;
    // al_draw_line(cx, cy, bx, by, al_map_rgb(200, 200, 0), 4);  // Commented out yellow cannon line

    // Cannon charge gauge (centered on tank, smaller size)
    if (tank->charging && tank->weapon == 1) {
        double bar_width = 100; // Reduced from 150 to 100
        double bar_x = cx - bar_width / 2; // Center on tank
        double bar_y = sy - 25; // Position above tank (tank height based)
        
        // Calculate gauge width with proper scaling and clamping
        double max_power = 15.0; // Maximum cannon power
        double gauge_ratio = tank->cannon_power / max_power;
        if (gauge_ratio > 1.0) gauge_ratio = 1.0; // Clamp to 100%
        double gauge_w = bar_width * gauge_ratio;
        
        al_draw_filled_rectangle(bar_x, bar_y, bar_x + gauge_w, bar_y + 8, al_map_rgb(255, 0, 0));
        al_draw_rectangle(bar_x, bar_y, bar_x + bar_width, bar_y + 8, al_map_rgb(255, 255, 255), 1);
    }

    // Machine gun reload gauge (centered on tank, smaller size) - only show when using MG
    if (tank->mg_reloading && tank->weapon == 0) {
        double total = 2.0;
        double filled = (total - tank->mg_reload_time) / total;
        if (filled < 0) filled = 0;
        if (filled > 1) filled = 1;
        double bar_width = 100; // Reduced from 150 to 100
        double bar_x = cx - bar_width / 2; // Center on tank
        double bar_y = sy - 40; // Position above tank (tank height based)
        double gw = bar_width * filled;
        al_draw_rectangle(bar_x, bar_y, bar_x + bar_width, bar_y + 8, al_map_rgb(255, 255, 255), 1);
        al_draw_filled_rectangle(bar_x + 1, bar_y + 1, bar_x + 1 + gw, bar_y + 7, al_map_rgb(0, 200, 255));
    }
}

// ===== Getter Functions =====

// Global tank instance (needed for getter functions)
static Tank* g_tank = NULL;
static double g_camera_x = 0.0;
static double g_camera_y = 0.0;

// Set global tank reference (called from tank_init)
void set_global_tank_ref(Tank* tank) {
    g_tank = tank;
}

// Set camera position (called from main game loop)
void set_camera_position(double x, double y) {
    g_camera_x = x;
    g_camera_y = y;
}

double get_tank_x(void) {
    return g_tank ? g_tank->x : 0.0;
}

double get_tank_y(void) {
    return g_tank ? g_tank->y : 0.0;
}

int get_tank_width(void) {
    return g_tank ? g_tank->width : 32; // Use global reference or default
}

int get_tank_height(void) {
    return g_tank ? g_tank->height : 20; // Use global reference or default
}

int get_tank_hp(void) {
    return g_tank ? g_tank->hp : 0;
}

int get_tank_max_hp(void) {
    return g_tank ? g_tank->max_hp : 0;
}

double get_tank_invincible(void) {
    return g_tank ? g_tank->invincible : 0.0;
}

void set_tank_hp(int hp) {
    if (g_tank) {
        g_tank->hp = hp;
        if (g_tank->hp < 0) g_tank->hp = 0;
        if (g_tank->hp > g_tank->max_hp) g_tank->hp = g_tank->max_hp;
    }
}

void set_tank_invincible(double time) {
    if (g_tank) {
        g_tank->invincible = time;
    }
}

void set_tank_velocity(double vx, double vy) {
    if (g_tank) {
        g_tank->vx = vx;
        g_tank->vy = vy;
    }
}

void set_tank_x(double x) {
    if (g_tank) {
        g_tank->x = x;
    }
}

double get_camera_x(void) {
    return g_camera_x;
}

double get_camera_y(void) {
    return g_camera_y;
}

ALLEGRO_BITMAP* tank_sprite_grab(int x, int y, int w, int h)
{
    ALLEGRO_BITMAP* sprite = al_create_sub_bitmap(tank_sprites._sheet, x, y, w, h);
    return sprite;
}

//tank sprite flip horizontal
ALLEGRO_BITMAP* flip_horizontal(ALLEGRO_BITMAP* bmp) {
    int w = al_get_bitmap_width(bmp);
    int h = al_get_bitmap_height(bmp);

    ALLEGRO_BITMAP* flipped = al_create_bitmap(w, h);
    if (!flipped) {
        printf("Failed to create flipped bitmap\n");
        return NULL;
    }

    ALLEGRO_BITMAP* old_target = al_get_target_bitmap(); // 현재 백버퍼 저장

    al_set_target_bitmap(flipped);
    al_clear_to_color(al_map_rgba(0, 0, 0, 0)); // 투명 배경

    // ✅ 정중앙(0, 0)에 반전하여 그리기
    al_draw_bitmap(bmp, 0, 0, ALLEGRO_FLIP_HORIZONTAL);

    al_set_target_bitmap(old_target); // 원래 백버퍼로 복귀

    return flipped;
}


//sprite tank init
void tank_sprite_init(const char* sprite_path){
    tank_sprites._sheet = al_load_bitmap(sprite_path);
    if (!tank_sprites._sheet){
    printf("Failed to load tank sprite sheet: %s\n", sprite_path);
        return;
    } 
    tank_sprites.tank_base = tank_sprites._sheet;
    tank_sprites.fliped_sheet = flip_horizontal(tank_sprites._sheet);
}

