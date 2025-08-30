#include "enemy.h"
#include "map_generation.h"
#include "tank.h"
#include "bullet.h"
#include "ini_parser.h"
#include "game_system.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ===== Globals =====
static Enemy enemies[MAX_ENEMIES];
static FlyingEnemy f_enemies[MAX_FLY_ENEMIES];

// Jump timing parameters loaded from config.ini
double enemy_jump_interval_min = 1.8;  // Default values
double enemy_jump_interval_max = 2.2;

// Enemy physics parameters loaded from config.ini
double enemy_base_speed = 0.1;  // Default values
double enemy_speed_per_difficulty = 0.5;

// Flying enemy bullet parameters loaded from config.ini
int flying_enemy_burst_count = 10;  // Default values
double flying_enemy_shot_interval = 0.05;
double flying_enemy_rest_time = 2.0;
double flying_enemy_bullet_speed = 8.0;
int flying_enemy_bullet_width = 6;
int flying_enemy_bullet_height = 3;
double roi_multiplier = 1.5;
double max_shooting_distance = 800.0;

//align enemies
static int enemy_align_x = 20;
static int flying_enemy_align_x = 10;

// sprite

enemy_sprites_t enemy_sprites;


// ===== Enemy Initialization =====

void enemies_init(void) {
    // Load enemy parameters from config.ini
    const MapConfig* config = map_get_config();
    if (config) {
        enemy_jump_interval_min = config->enemy_jump_interval_min;
        enemy_jump_interval_max = config->enemy_jump_interval_max;
        enemy_base_speed = config->enemy_base_speed;
        enemy_speed_per_difficulty = config->enemy_speed_per_difficulty;
        
        printf("Loaded enemy parameters:\n");
        printf("  Jump timing: %.1f - %.1f seconds\n", enemy_jump_interval_min, enemy_jump_interval_max);
        printf("  Base speed: %.1f, Speed per difficulty: %.1f\n", enemy_base_speed, enemy_speed_per_difficulty);
    }
    
    // Load flying enemy bullet parameters from config.ini
    IniParser* bullet_parser = ini_parser_create();
    ini_parser_load_file(bullet_parser, "TankBoy/config.ini");
    flying_enemy_burst_count = ini_parser_get_int(bullet_parser, "EnemyBullets", "flying_enemy_burst_count", 10);
    flying_enemy_shot_interval = ini_parser_get_double(bullet_parser, "EnemyBullets", "flying_enemy_shot_interval", 0.05);
    flying_enemy_rest_time = ini_parser_get_double(bullet_parser, "EnemyBullets", "flying_enemy_rest_time", 2.0);
    flying_enemy_bullet_speed = ini_parser_get_double(bullet_parser, "EnemyBullets", "flying_enemy_bullet_speed", 8.0);
    flying_enemy_bullet_width = ini_parser_get_int(bullet_parser, "EnemyBullets", "flying_enemy_bullet_width", 6);
    flying_enemy_bullet_height = ini_parser_get_int(bullet_parser, "EnemyBullets", "flying_enemy_bullet_height", 3);
    roi_multiplier = ini_parser_get_double(bullet_parser, "EnemyBullets", "roi_multiplier", 1.5);
    max_shooting_distance = ini_parser_get_double(bullet_parser, "EnemyBullets", "max_shooting_distance", 800.0);
    
    ini_parser_destroy(bullet_parser);
    
    printf("Loaded flying enemy bullet parameters:\n");
    printf("  Burst count: %d, Shot interval: %.3f seconds\n", flying_enemy_burst_count, flying_enemy_shot_interval);
    printf("  Rest time: %.1f seconds, Bullet speed: %.1f\n", flying_enemy_rest_time, flying_enemy_bullet_speed);
    printf("  Bullet size: %dx%d\n", flying_enemy_bullet_width, flying_enemy_bullet_height);
    
    // Load enemy dimensions from config
    IniParser* parser = ini_parser_create();
    ini_parser_load_file(parser, "TankBoy/config.ini");
    int enemy_width = ini_parser_get_int(parser, "Enemy", "enemy_width", 25);
    int enemy_height = ini_parser_get_int(parser, "Enemy", "enemy_height", 15);
    ini_parser_destroy(parser);
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].alive = false;
        enemies[i].x = 0.0;
        enemies[i].y = 0.0;
        enemies[i].vx = 0.0;
        enemies[i].vy = 0.0;
        enemies[i].on_ground = true;
        enemies[i].hp = 0;
        enemies[i].max_hp = 0;
        enemies[i].last_x = 0.0;
        enemies[i].stuck_time = 0.0;
        enemies[i].speed = 0.0;
        enemies[i].jump_timer = 0.0;
        
        // Set dimensions from config
        enemies[i].width = enemy_width;
        enemies[i].height = enemy_height;
    }
}


void enemy_sprites_init() {
    
    enemy_sprites.land_enemy_sheet = NULL; // not using total combined sheet
    
    enemy_sprites.land_enemy_sprites = malloc(3 * sizeof(ALLEGRO_BITMAP*));
    for (int i = 0; i < 3; i++) {
        char enemy_sprite_file_path[256];
        snprintf(enemy_sprite_file_path, sizeof(enemy_sprite_file_path), "TankBoy/resources/sprites/enemy%d.png", i+1);
        enemy_sprites.land_enemy_sprites[i] = al_load_bitmap(enemy_sprite_file_path);
        if (enemy_sprites.land_enemy_sprites[i] == NULL) {
            printf("wrong location of enemy sprite!!\n");
        }
    }
}


void flying_enemy_sprites_init() {

    char* flying_enemy_sprite_file = "TankBoy/resources/sprites/helicopters.png";
    enemy_sprites.flying_enemy_sheet = al_load_bitmap(flying_enemy_sprite_file);
    if (enemy_sprites.flying_enemy_sheet == NULL) {
        printf("wrong location of flying enemy sprite!!\n");
    }
    
    enemy_sprites.flying_enemy_sprites = malloc(3 * sizeof(ALLEGRO_BITMAP*));
    int width = al_get_bitmap_width(enemy_sprites.flying_enemy_sheet) / 3;
    int height = al_get_bitmap_height(enemy_sprites.flying_enemy_sheet);
    for (int i = 0; i < 3; i++) {
        enemy_sprites.flying_enemy_sprites[i] = al_create_sub_bitmap(enemy_sprites.flying_enemy_sheet, i*width, 0, width, height);
    }
}

void flying_enemies_init(void) {
    // Load flying enemy dimensions from config
    IniParser* parser = ini_parser_create();
    ini_parser_load_file(parser, "TankBoy/config.ini");
    int flying_enemy_width = ini_parser_get_int(parser, "Enemy", "flying_enemy_width", 30);
    int flying_enemy_height = ini_parser_get_int(parser, "Enemy", "flying_enemy_height", 20);
    ini_parser_destroy(parser);
    
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        f_enemies[i].alive = false;
        f_enemies[i].x = 0.0;
        f_enemies[i].y = 0.0;
        f_enemies[i].vx = 0.0;
        f_enemies[i].base_y = 0.0;
        f_enemies[i].spawn_x = 0.0;
        f_enemies[i].angle = 0.0;
        f_enemies[i].x_angle = 0.0;
        f_enemies[i].in_burst = false;
        f_enemies[i].burst_shots_left = 0;
        f_enemies[i].shot_interval = flying_enemy_shot_interval;
        f_enemies[i].shot_timer = 0.0;
        f_enemies[i].rest_timer = flying_enemy_rest_time;
        f_enemies[i].hp = 0;
        f_enemies[i].max_hp = 0;
        
        // Set dimensions from config
        f_enemies[i].width = flying_enemy_width;
        f_enemies[i].height = flying_enemy_height;
    }
}



// ===== Enemy Spawning =====

void load_enemies_from_csv_with_map(int stage_number, const Map* map) {
    char csv_path[256];
    snprintf(csv_path, sizeof(csv_path), "TankBoy/resources/stages/enemies%d.csv", stage_number);
    
    FILE* file = fopen(csv_path, "r");
    if (!file) {
        printf("Warning: Could not open enemy CSV file: %s\n", csv_path);
        return;
    }
    
    char line[256];
    int line_count = 0;
    
    // Skip header line
    if (fgets(line, sizeof(line), file)) {
        line_count++;
    }
    
    int enemy_index = 0;
    while (fgets(line, sizeof(line), file) && enemy_index < MAX_ENEMIES) {
        line_count++;
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        char* token = strtok(line, ",");
        if (!token) continue;
        
        double x = atof(token);
        
        token = strtok(NULL, ",");
        if (!token) continue;
        double y = atof(token);
        
        token = strtok(NULL, ",");
        if (!token) continue;
        char enemy_type[32];
        strncpy(enemy_type, token, sizeof(enemy_type) - 1);
        enemy_type[sizeof(enemy_type) - 1] = 0;
        
        token = strtok(NULL, ",");
        int difficulty = token ? atoi(token) : 1;
        
        // Find available enemy slot
        while (enemy_index < MAX_ENEMIES && enemies[enemy_index].alive) {
            enemy_index++;
        }
        
        if (enemy_index >= MAX_ENEMIES) break;
        
        // Initialize enemy based on type
        if (strcmp(enemy_type, "tank") == 0) {
            // Load enemy dimensions from config
            IniParser* parser = ini_parser_create();
            ini_parser_load_file(parser, "TankBoy/config.ini");
            int enemy_width = ini_parser_get_int(parser, "Enemy", "enemy_width", 25);
            int enemy_height = ini_parser_get_int(parser, "Enemy", "enemy_height", 15);
            ini_parser_destroy(parser);
            
            enemies[enemy_index].alive = true;
            enemies[enemy_index].x = x;
            
            // Use actual map ground level if map is available
            if (map) {
                int ground_level = map_get_ground_level(map, (int)x, enemy_width);
                enemies[enemy_index].y = ground_level - enemy_height;
            } else {
                enemies[enemy_index].y = y;
            }
            
            enemies[enemy_index].vx = 0.0;
            enemies[enemy_index].vy = 0.0;
            enemies[enemy_index].on_ground = true;
            enemies[enemy_index].max_hp = ENEMY_BASE_HP + difficulty * 50;
            enemies[enemy_index].hp = enemies[enemy_index].max_hp;
            enemies[enemy_index].last_x = x;
            enemies[enemy_index].stuck_time = 0.0;
            enemies[enemy_index].speed = enemy_base_speed + difficulty * enemy_speed_per_difficulty;
            enemies[enemy_index].jump_timer = enemy_jump_interval_min + (rand() % (int)((enemy_jump_interval_max - enemy_jump_interval_min) * 10)) / 10.0;  // Random initial timer
            
            // Set dimensions from config
            enemies[enemy_index].width = enemy_width;
            enemies[enemy_index].height = enemy_height;
            
            // Store difficulty for scoring
            enemies[enemy_index].difficulty = difficulty;
            
            printf("Spawned tank enemy at (%f, %f) with difficulty %d\n", x, enemies[enemy_index].y, difficulty);
        }
        else if (strcmp(enemy_type, "helicopter") == 0) {
            // Find available flying enemy slot
            int fly_index = 0;
            while (fly_index < MAX_FLY_ENEMIES && f_enemies[fly_index].alive) {
                fly_index++;
            }
            
            if (fly_index < MAX_FLY_ENEMIES) {
                // Load flying enemy dimensions from config
                IniParser* parser = ini_parser_create();
                ini_parser_load_file(parser, "TankBoy/config.ini");
                int flying_enemy_width = ini_parser_get_int(parser, "Enemy", "flying_enemy_width", 30);
                int flying_enemy_height = ini_parser_get_int(parser, "Enemy", "flying_enemy_height", 20);
                ini_parser_destroy(parser);
                
                f_enemies[fly_index].alive = true;
                f_enemies[fly_index].x = x;
                f_enemies[fly_index].y = y;
                f_enemies[fly_index].base_y = y;
                f_enemies[fly_index].spawn_x = x;  // Store spawn position
                f_enemies[fly_index].vx = (rand() % 2 ? 1.0 : -1.0) * (1.0 + difficulty * 0.2);
                f_enemies[fly_index].angle = 0.0;
                f_enemies[fly_index].x_angle = 0.0;
                f_enemies[fly_index].in_burst = false;
                f_enemies[fly_index].burst_shots_left = 0;
                f_enemies[fly_index].shot_interval = 0.05;
                f_enemies[fly_index].shot_timer = 0.0;
                f_enemies[fly_index].rest_timer = 0.5 + (rand() % 50) / 100.0;
                f_enemies[fly_index].max_hp = FLY_BASE_HP + FLY_HP_PER_ROUND * difficulty;
                f_enemies[fly_index].hp = f_enemies[fly_index].max_hp;
                
                // Set dimensions from config
                f_enemies[fly_index].width = flying_enemy_width;
                f_enemies[fly_index].height = flying_enemy_height;
                
                // Store difficulty for scoring
                f_enemies[fly_index].difficulty = difficulty;
                
                printf("Spawned helicopter enemy at (%f, %f) with difficulty %d\n", x, y, difficulty);
            }
        }
        
        enemy_index++;
    }
    
    fclose(file);
    printf("Loaded %d enemies from CSV\n", enemy_index);
}

void spawn_enemies(int round_number) {
    int count = round_number + 2;
    int map_width = map_get_map_width();
    
    for (int i = 0; i < MAX_ENEMIES && count > 0; i++) {
        if (!enemies[i].alive) {
            // Load enemy dimensions from config
            IniParser* parser = ini_parser_create();
            ini_parser_load_file(parser, "TankBoy/config.ini");
            int enemy_width = ini_parser_get_int(parser, "Enemy", "enemy_width", 25);
            int enemy_height = ini_parser_get_int(parser, "Enemy", "enemy_height", 15);
            ini_parser_destroy(parser);
            
            enemies[i].alive = true;

            // Spawn enemies at map edges, but ensure they're within bounds
            if (rand() % 2) {
                enemies[i].x = 50 + rand() % 100; // Left side
            } else {
                enemies[i].x = map_width - 150 + rand() % 100; // Right side
            }

            // Get ground level at spawn position and place enemy on ground
            int ground_level = map_get_ground_level(NULL, (int)enemies[i].x, enemy_width);
            enemies[i].y = ground_level - enemy_height;
            
            enemies[i].vx = 0.0;
            enemies[i].vy = 0.0;
            enemies[i].on_ground = true;

            enemies[i].max_hp = ENEMY_BASE_HP + ENEMY_HP_PER_ROUND * round_number;
            enemies[i].hp = enemies[i].max_hp;

            enemies[i].last_x = enemies[i].x;
            enemies[i].stuck_time = 0.0;

            enemies[i].speed = enemy_base_speed + round_number * enemy_speed_per_difficulty;
            enemies[i].jump_timer = enemy_jump_interval_min + (rand() % (int)((enemy_jump_interval_max - enemy_jump_interval_min) * 10)) / 10.0;  // Random initial timer

            // Set dimensions from config
            enemies[i].width = enemy_width;
            enemies[i].height = enemy_height;

            count--;
        }
    }
}

void spawn_flying_enemy(int round_number) {
    int map_width = map_get_map_width();
    
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        if (!f_enemies[i].alive) {
            // Load flying enemy dimensions from config
            IniParser* parser = ini_parser_create();
            ini_parser_load_file(parser, "TankBoy/config.ini");
            int flying_enemy_width = ini_parser_get_int(parser, "Enemy", "flying_enemy_width", 30);
            int flying_enemy_height = ini_parser_get_int(parser, "Enemy", "flying_enemy_height", 20);
            ini_parser_destroy(parser);
            
            f_enemies[i].alive = true;
            f_enemies[i].x = rand() % map_width;
            f_enemies[i].base_y = 100 + rand() % 100;
            f_enemies[i].y = f_enemies[i].base_y;
            f_enemies[i].spawn_x = f_enemies[i].x;  // 스폰 위치 저장
            f_enemies[i].vx = (rand() % 2 ? 1.0 : -1.0) * (1.0 + round_number * 0.2);
            f_enemies[i].angle = 0.0;
            f_enemies[i].x_angle = 0.0;

            f_enemies[i].in_burst = false;
            f_enemies[i].burst_shots_left = 0;
            f_enemies[i].shot_interval = 0.05;
            f_enemies[i].shot_timer = 0.0;
            f_enemies[i].rest_timer = 0.5 + (rand() % 50) / 100.0;

            f_enemies[i].max_hp = FLY_BASE_HP + FLY_HP_PER_ROUND * round_number;
            f_enemies[i].hp = f_enemies[i].max_hp;
            
            // Set dimensions from config
            f_enemies[i].width = flying_enemy_width;
            f_enemies[i].height = flying_enemy_height;
            
            break;
        }
    }
}

// ===== Enemy Updates =====

void enemies_update_roi_with_map(double dt, double camera_x, double camera_y, int buffer_width, int buffer_height, const Map* map) {
    const double gravity = 0.5;
    const double jump_power = -8.5;
    const double stuck_threshold = 1.0;
    const double stuck_jump_time = 2.0;
    int map_width = map_get_map_width();
    int map_height = map_get_map_height();
    
    // Calculate ROI (Region of Interest) - 2x buffer size around camera
    double roi_left = camera_x - buffer_width;
    double roi_right = camera_x + buffer_width * 2;
    double roi_top = camera_y - buffer_height;
    double roi_bottom = camera_y + buffer_height * 2;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;
        
        // Skip enemies outside ROI
        if (e->x < roi_left || e->x > roi_right || e->y < roi_top || e->y > roi_bottom) {
            continue;
        }

        // Get tank position for AI
        double tank_x = get_tank_x();
        double dir = (tank_x > e->x) ? 1.0 : -1.0;
        
        // Set constant speed based on direction
        e->vx = dir * e->speed;

        e->vy += gravity;

        // Fixed interval jump logic
        if (e->on_ground && e->vy >= 0) {  // On ground and not moving up
            if (e->jump_timer <= 0.0) {
                // Always jump when timer expires
                e->vy = jump_power;
                e->on_ground = false;
                // Reset timer with fixed interval from config
                e->jump_timer = enemy_jump_interval_min + (rand() % (int)((enemy_jump_interval_max - enemy_jump_interval_min) * 10)) / 10.0;
            } else {
                e->jump_timer -= dt;
            }
        }

        // Store old position for collision detection
        double old_x = e->x;
        double old_y = e->y;

        // Update position based on velocity (same as tank.c)
        double new_x = e->x + e->vx;
        
        // Simple horizontal collision check (no auto step-up)
        if (map && map_rect_collision(map, (int)new_x, (int)e->y, e->width, e->height)) {
            // Just stop horizontal movement if collision
            e->vx = 0;
        } else {
            e->x = new_x;
        }

        // Check vertical collision before moving (like tank)
        double new_y = e->y + e->vy;
        if (map && map_rect_collision(map, (int)e->x, (int)new_y, e->width, e->height)) {
            if (e->vy > 0) {  // Falling down, hit ground
                e->vy = 0;
                e->on_ground = true;
                // Get actual ground level from map
                int ground_level = map_get_ground_level(map, (int)e->x, e->width);
                e->y = ground_level - e->height;
            } else {  // Moving up, hit ceiling
                e->vy = 0;
            }
        } else {
            e->y = new_y;
            // Check if still on ground by testing a small area below enemy
            if (map && map_rect_collision(map, (int)e->x, (int)(e->y + e->height + 1), e->width, 1)) {
                e->on_ground = true;
            } else {
                e->on_ground = false;  // In air if no collision below
            }
        }

        // Map boundary collision
        if (e->x < 0) { 
            e->x = 0; 
            e->vx = fabs(e->vx); 
        }
        if (e->x > map_width - e->width) {
            e->x = map_width - e->width; 
            e->vx = -fabs(e->vx); 
        }

        // Vertical boundary check
        if (e->y < 0) {
            e->y = 0;
            e->vy = 0.0;
        }
        if (e->y > map_height - 20) { // ENEMY_H = 20
            e->y = map_height - 20;
            e->vy = 0.0;
            e->on_ground = true;
        }

        // stuck detection
        if (fabs(e->x - e->last_x) <= stuck_threshold) {
            e->stuck_time += dt;
        }
        else {
            e->stuck_time = 0.0;
            e->last_x = e->x;
        }

        if (e->stuck_time >= stuck_jump_time && e->on_ground) {
            e->vy = jump_power;
            e->vx += dir * 1.5; // small horizontal boost
            e->stuck_time = 0.0;
        }
    }
}

void enemies_update_with_map(double dt, const Map* map) {
    const double gravity = 0.5;
    const double jump_power = -8.5;
    const double stuck_threshold = 1.0;
    const double stuck_jump_time = 2.0;
    int map_width = map_get_map_width();
    int map_height = map_get_map_height();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;

        // Get tank position for AI
        double tank_x = get_tank_x();
        double dir = (tank_x > e->x) ? 1.0 : -1.0;
        
        // Set constant speed based on direction
        e->vx = dir * e->speed;

        e->vy += gravity;

        // Fixed interval jump logic
        if (e->on_ground && e->vy >= 0) {  // On ground and not moving up
            if (e->jump_timer <= 0.0) {
                // Always jump when timer expires
                e->vy = jump_power;
                e->on_ground = false;
                // Reset timer with fixed interval from config
                e->jump_timer = enemy_jump_interval_min + (rand() % (int)((enemy_jump_interval_max - enemy_jump_interval_min) * 10)) / 10.0;
            } else {
                e->jump_timer -= dt;
            }
        }

        // Update position based on velocity (same as tank.c)
        double new_x = e->x + e->vx;
        
        // Simple horizontal collision check (no auto step-up)
        if (map && map_rect_collision(map, (int)new_x, (int)e->y, e->width, e->height)) {
            // Just stop horizontal movement if collision
            e->vx = 0;
        } else {
            e->x = new_x;
        }

        // Store old position for collision detection
        double old_x = e->x;
        double old_y = e->y;

        // Check vertical collision before moving (like tank)
        double new_y = e->y + e->vy;
        if (map && map_rect_collision(map, (int)e->x, (int)new_y, e->width, e->height)) {
            if (e->vy > 0) {  // Falling down, hit ground
                e->vy = 0;
                e->on_ground = true;
                // Get actual ground level from map
                int ground_level = map_get_ground_level(map, (int)e->x, 32);
                e->y = ground_level - 20;  // ENEMY_H = 20
            } else {  // Moving up, hit ceiling
                e->vy = 0;
            }
        } else {
            e->y = new_y;
            // Check if still on ground by testing a small area below enemy
            if (map && map_rect_collision(map, (int)e->x, (int)(e->y + 20 + 1), 32, 1)) {
                e->on_ground = true;
            } else {
                e->on_ground = false;  // In air if no collision below
            }
        }

        // Map boundary collision
        if (e->x < 0) { 
            e->x = 0; 
            e->vx = fabs(e->vx); 
        }
        if (e->x > map_width - 32) { // ENEMY_W = 32
            e->x = map_width - 32; 
            e->vx = -fabs(e->vx); 
        }

        // Vertical boundary check
        if (e->y < 0) {
            e->y = 0;
            e->vy = 0.0;
        }
        if (e->y > map_height - 20) { // ENEMY_H = 20
            e->y = map_height - 20;
            e->vy = 0.0;
            e->on_ground = true;
        }

        // stuck detection
        if (fabs(e->x - e->last_x) <= stuck_threshold) {
            e->stuck_time += dt;
        }
        else {
            e->stuck_time = 0.0;
            e->last_x = e->x;
        }

        if (e->stuck_time >= stuck_jump_time && e->on_ground) {
            e->vy = jump_power;
            e->vx += dir * 1.5; // small horizontal boost
            e->stuck_time = 0.0;
        }
    }
}

void enemies_update(double dt) {
    enemies_update_with_map(dt, NULL);
}

void flying_enemies_update_roi(double dt, double camera_x, double camera_y, int buffer_width, int buffer_height) {
    int map_width = map_get_map_width();
    int map_height = map_get_map_height();
    
    // Calculate ROI (Region of Interest) - symmetric around camera with configurable multiplier
    double roi_half_width = buffer_width * roi_multiplier;
    double roi_half_height = buffer_height * roi_multiplier;
    double roi_left = camera_x - roi_half_width;
    double roi_right = camera_x + roi_half_width;
    double roi_top = camera_y - roi_half_height;
    double roi_bottom = camera_y + roi_half_height;

    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;
        
        // Skip enemies outside ROI
        if (fe->x < roi_left || fe->x > roi_right || fe->y < roi_top || fe->y > roi_bottom) {
            continue;
        }

        // Y-axis: maintain existing trigonometric movement (up-down oscillation)
        fe->angle += dt * 2.0;
        fe->y = fe->base_y + sin(fe->angle) * 30.0;
        
        // X-axis: left-right movement based on trigonometry centered on spawn position
        fe->x_angle += dt * 1.5;  // x-axis movement speed (adjustable)
        double x_offset = sin(fe->x_angle) * 150.0;  // ±150 pixel range from spawn position (adjustable)
        fe->x = fe->spawn_x + x_offset;

        // 맵 경계 체크 (삼각함수 기반이므로 bounce 대신 경계 제한)
        if (fe->x < 0) { 
            fe->x = 0; 
        }
        if (fe->x > map_width - fe->width) {
            fe->x = map_width - fe->width; 
        }

        // Vertical boundary check - 헬리콥터는 땅과 상호작용하지 않음
        if (fe->y < 50) { // Keep helicopters above ground
            fe->y = 50;
        }
        if (fe->y > map_height - fe->height) {
            fe->y = map_height - fe->height;
        }

        if (fe->in_burst) {
            fe->shot_timer -= dt;

            while (fe->shot_timer <= 0.0 && fe->burst_shots_left > 0) {
                // Check distance to player before shooting
                double tank_x = get_tank_x();
                double tank_y = get_tank_y();
                double dx = tank_x - fe->x;
                double dy = tank_y - fe->y;
                double distance_to_player = sqrt(dx * dx + dy * dy);
                
                // Only shoot if player is within shooting range
                if (distance_to_player <= max_shooting_distance) {
                    // Create enemy bullet - shoot towards player tank
                    for (int j = 0; j < MAX_BULLETS; j++) {
                        Bullet* bullets = get_bullets();
                        if (bullets && !bullets[j].alive) {
                            bullets[j].alive = true;
                            bullets[j].x = fe->x + fe->width / 2.0;
                            bullets[j].y = fe->y + fe->height / 2.0;
                            bullets[j].weapon = 0;      // MG round
                            bullets[j].from_enemy = true;
                            bullets[j].width = flying_enemy_bullet_width;
                            bullets[j].height = flying_enemy_bullet_height;
                            
                            // Calculate bullet direction towards player tank
                            double ang = atan2(dy, dx);
                            
                            // Set bullet angle for visual orientation
                            bullets[j].angle = ang;

                            // Set bullet velocity
                            bullets[j].vx = cos(ang) * flying_enemy_bullet_speed;
                            bullets[j].vy = sin(ang) * flying_enemy_bullet_speed;
                            break;
                        }
                    }
                }

                fe->burst_shots_left--;
                fe->shot_timer += flying_enemy_shot_interval;

                if (fe->burst_shots_left <= 0) {
                    fe->in_burst = false;
                    fe->rest_timer = flying_enemy_rest_time;
                }
            }
        }
        else {
            fe->rest_timer -= dt;
            if (fe->rest_timer <= 0.0) {
                fe->in_burst = true;
                fe->burst_shots_left = flying_enemy_burst_count;
                fe->shot_timer = 0.0; // fire immediately
            }
        }
    }
}

void flying_enemies_update(double dt) {
    int map_width = map_get_map_width();
    int map_height = map_get_map_height();

    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;

        // Y-axis: maintain existing trigonometric movement (up-down oscillation)
        fe->angle += dt * 2.0;
        fe->y = fe->base_y + sin(fe->angle) * 30.0;
        
        // X-axis: left-right movement based on trigonometry centered on spawn position
        fe->x_angle += dt * 1.5;  // x-axis movement speed (adjustable)
        double x_offset = sin(fe->x_angle) * 150.0;  // ±150 pixel range from spawn position (adjustable)
        fe->x = fe->spawn_x + x_offset;

        // 맵 경계 체크 (삼각함수 기반이므로 bounce 대신 경계 제한)
        if (fe->x < 0) { 
            fe->x = 0; 
        }
        if (fe->x > map_width - fe->width) {
            fe->x = map_width - fe->width; 
        }

        // Vertical boundary check - 헬리콥터는 땅과 상호작용하지 않음
        if (fe->y < 50) { // Keep helicopters above ground
            fe->y = 50;
        }
        if (fe->y > map_height - fe->height) {
            fe->y = map_height - fe->height;
        }

        if (fe->in_burst) {
            fe->shot_timer -= dt;

            while (fe->shot_timer <= 0.0 && fe->burst_shots_left > 0) {
                // Check distance to player before shooting
                double tank_x = get_tank_x();
                double tank_y = get_tank_y();
                double dx = tank_x - fe->x;
                double dy = tank_y - fe->y;
                double distance_to_player = sqrt(dx * dx + dy * dy);
                
                // Only shoot if player is within shooting range
                if (distance_to_player <= max_shooting_distance) {
                    // Create enemy bullet - shoot towards player tank
                    for (int j = 0; j < MAX_BULLETS; j++) {
                        Bullet* bullets = get_bullets();
                        if (bullets && !bullets[j].alive) {
                            bullets[j].alive = true;
                            bullets[j].x = fe->x + fe->width / 2.0;
                            bullets[j].y = fe->y + fe->height / 2.0;
                            bullets[j].weapon = 0;      // MG round
                            bullets[j].from_enemy = true;
                            bullets[j].width = flying_enemy_bullet_width;
                            bullets[j].height = flying_enemy_bullet_height;
                            
                            // Calculate bullet direction towards player tank
                            double ang = atan2(dy, dx);
                            
                            // Set bullet angle for visual orientation
                            bullets[j].angle = ang;

                            // Set bullet velocity
                            bullets[j].vx = cos(ang) * flying_enemy_bullet_speed;
                            bullets[j].vy = sin(ang) * flying_enemy_bullet_speed;
                            break;
                        }
                    }
                }

                fe->burst_shots_left--;
                fe->shot_timer += flying_enemy_shot_interval;

                if (fe->burst_shots_left <= 0) {
                    fe->in_burst = false;
                    fe->rest_timer = flying_enemy_rest_time;
                }
            }
        }
        else {
            fe->rest_timer -= dt;
            if (fe->rest_timer <= 0.0) {
                fe->in_burst = true;
                fe->burst_shots_left = flying_enemy_burst_count;
                fe->shot_timer = 0.0; // fire immediately
            }
        }
    }
}

// ===== Enemy Rendering =====

void enemies_draw(double camera_x, double camera_y) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;
        
        // Convert world coordinates to screen coordinates
        double sx = e->x - camera_x;
        double sy = e->y - camera_y;
        
        // Draw enemy (basic rectangle for now)
        al_draw_filled_rectangle(sx, sy, sx + e->width, sy + e->height, al_map_rgb(200, 50, 50));
        
        // draw enemy sprite
        int width = al_get_bitmap_width(enemy_sprites.land_enemy_sprites[e->difficulty-1]);
        int height = al_get_bitmap_height(enemy_sprites.land_enemy_sprites[e->difficulty-1]);
        
        al_draw_scaled_bitmap(enemy_sprites.land_enemy_sprites[e->difficulty-1],
            0, 0,
            width, height,
            e->x - camera_x, e->y - camera_y,
            e->width, e->height,
            0);

        // HP bar would be drawn by HUD system
    }
}

void flying_enemies_draw(double camera_x, double camera_y) {
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;


        // Convert world coordinates to screen coordinates
        al_draw_filled_rectangle(fe->x - camera_x, fe->y - camera_y, 
            fe->x - camera_x + fe->width, fe->y - camera_y + fe->height, 
            al_map_rgb(180, 0, 180));

        int width = al_get_bitmap_width(enemy_sprites.flying_enemy_sprites[fe->difficulty-1]);
        int height = al_get_bitmap_height(enemy_sprites.flying_enemy_sprites[fe->difficulty-1]);
        
        al_draw_scaled_bitmap(enemy_sprites.flying_enemy_sprites[fe->difficulty-1],
            0, 0,
            width, height,
            fe->x - camera_x, fe->y - camera_y,
            fe->width, fe->height,
            0);
    }
}

// ===== Enemy Utilities =====

bool any_ground_enemies_alive(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i)
        if (enemies[i].alive) return true;
    return false;
}

bool any_flying_enemies_alive(void) {
    for (int i = 0; i < MAX_FLY_ENEMIES; ++i)
        if (f_enemies[i].alive) return true;
    return false;
}

int get_alive_enemy_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) count++;
    }
    return count;
}

int get_alive_flying_enemy_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        if (f_enemies[i].alive) count++;
    }
    return count;
}

// ===== Enemy Damage and Effects =====

void damage_enemy(Enemy* enemy, int damage) {
    if (!enemy || !enemy->alive) return;
    
    enemy->hp -= damage;
    if (enemy->hp <= 0) {
        enemy->alive = false;
        // Add score based on difficulty when enemy is killed
        add_score_for_enemy_kill(enemy->difficulty);
    }
}

void damage_flying_enemy(FlyingEnemy* fe, int damage) {
    if (!fe || !fe->alive) return;
    
    fe->hp -= damage;
    if (fe->hp <= 0) {
        fe->alive = false;
        // Add score based on difficulty when flying enemy is killed
        add_score_for_enemy_kill(fe->difficulty);
    }
}

void apply_cannon_explosion(double ex, double ey, double radius) {
    // ground enemies
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;
        double cx = e->x + e->width * 0.5;
        double cy = e->y + e->height * 0.5;
        double dx = cx - ex, dy = cy - ey;
        double dist = sqrt(dx * dx + dy * dy);
        int dmg = (int)(DMG_CANNON * (1.0 - (dist / radius)));
        if (dmg > 0) {
            damage_enemy(e, dmg);
            if (e->alive && dist < 1.0) dist = 1.0;
            if (e->alive && dist < radius) {
                double nx = dx / dist, ny = dy / dist;
                e->vx += nx * CANNON_SPLASH_KB;
                e->vy -= fabs(ny) * KNOCKBACK_ENEMY_VY;
            }
        }
    }

    // flying enemies: apply damage only
    for (int i = 0; i < MAX_FLY_ENEMIES; ++i) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;
        double cx = fe->x + fe->width * 0.5;
        double cy = fe->y + fe->height * 0.5;
        double dx = cx - ex, dy = cy - ey;
        double dist = sqrt(dx * dx + dy * dy);
        int dmg = (int)(DMG_CANNON * (1.0 - (dist / radius)));
        if (dmg > 0) {
            damage_flying_enemy(fe, dmg);
        }
    }
}

// ===== Enemy Movement Helpers =====

double get_enemy_ground_y(double x) {
    // Get map dimensions from config
    const MapConfig* config = map_get_config();
    if (!config) return 500.0; // Fallback
    
    int map_width = config->map_width;
    int map_height = config->map_height;
    
    // Create a simple terrain variation based on x position
    // This simulates hills and valleys
    double terrain_variation = sin(x * 0.01) * 50.0; // 50 pixel amplitude
    double base_ground = (double)(map_height - 150); // Base ground level
    
    // Add some random variation for more interesting terrain
    double random_offset = (sin(x * 0.005) + cos(x * 0.003)) * 20.0;
    
    double ground_level = base_ground + terrain_variation + random_offset;
    
    // Ensure ground level is within reasonable bounds
    if (ground_level < map_height - 300) ground_level = map_height - 300;
    if (ground_level > map_height - 50) ground_level = map_height - 50;
    
    return ground_level;
}

void handle_enemy_stuck_jump(Enemy* enemy, double dt) {
    if (!enemy) return;
    
    const double stuck_threshold = 1.0;
    const double stuck_jump_time = 2.0;
    const double jump_power = -8.5;
    
    if (fabs(enemy->x - enemy->last_x) <= stuck_threshold) {
        enemy->stuck_time += dt;
    }
    else {
        enemy->stuck_time = 0.0;
        enemy->last_x = enemy->x;
    }

    if (enemy->stuck_time >= stuck_jump_time && enemy->on_ground) {
        enemy->vy = jump_power;
        enemy->stuck_time = 0.0;
    }
}
// ===== Getter functions for external access =====

Enemy* get_enemies(void) {
    return enemies;
}

FlyingEnemy* get_flying_enemies(void) {
    return f_enemies;
}

void flying_enemy_sprites_deinit()
{
    al_destroy_bitmap(enemy_sprites.land_enemy_sheet);
    al_destroy_bitmap(enemy_sprites.flying_enemy_sheet);

    al_destroy_bitmap(enemy_sprites.land_enemy_sprites[0]);
    al_destroy_bitmap(enemy_sprites.land_enemy_sprites[1]);
    al_destroy_bitmap(enemy_sprites.land_enemy_sprites[2]);

    al_destroy_bitmap(enemy_sprites.flying_enemy_sprites[0]);
    al_destroy_bitmap(enemy_sprites.flying_enemy_sprites[1]);
    al_destroy_bitmap(enemy_sprites.flying_enemy_sprites[2]);

}