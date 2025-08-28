#include "enemy.h"
#include "map_generation.h"
#include "tank.h"
#include "bullet.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ===== Globals ===== */
static Enemy enemies[MAX_ENEMIES];
static FlyingEnemy f_enemies[MAX_FLY_ENEMIES];

/* ===== Enemy Initialization ===== */

void enemies_init(void) {
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
        enemies[i].accel = 0.0;
        enemies[i].friction = 0.0;
    }
}

void flying_enemies_init(void) {
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        f_enemies[i].alive = false;
        f_enemies[i].x = 0.0;
        f_enemies[i].y = 0.0;
        f_enemies[i].vx = 0.0;
        f_enemies[i].base_y = 0.0;
        f_enemies[i].angle = 0.0;
        f_enemies[i].in_burst = false;
        f_enemies[i].burst_shots_left = 0;
        f_enemies[i].shot_interval = 0.05;
        f_enemies[i].shot_timer = 0.0;
        f_enemies[i].rest_timer = 2.0;
        f_enemies[i].hp = 0;
        f_enemies[i].max_hp = 0;
    }
}

/* ===== Enemy Spawning ===== */

void load_enemies_from_csv(int stage_number) {
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
            enemies[enemy_index].alive = true;
            enemies[enemy_index].x = x;
            enemies[enemy_index].y = y;
            enemies[enemy_index].vx = 0.0;
            enemies[enemy_index].vy = 0.0;
            enemies[enemy_index].on_ground = true;
            enemies[enemy_index].max_hp = ENEMY_BASE_HP + ENEMY_HP_PER_ROUND * difficulty;
            enemies[enemy_index].hp = enemies[enemy_index].max_hp;
            enemies[enemy_index].last_x = x;
            enemies[enemy_index].stuck_time = 0.0;
            enemies[enemy_index].speed = 1.2 + difficulty * 0.15;
            enemies[enemy_index].accel = 0.15;
            enemies[enemy_index].friction = 0.90;
            
            printf("Spawned tank enemy at (%f, %f) with difficulty %d\n", x, y, difficulty);
        }
        else if (strcmp(enemy_type, "helicopter") == 0) {
            // Find available flying enemy slot
            int fly_index = 0;
            while (fly_index < MAX_FLY_ENEMIES && f_enemies[fly_index].alive) {
                fly_index++;
            }
            
            if (fly_index < MAX_FLY_ENEMIES) {
                f_enemies[fly_index].alive = true;
                f_enemies[fly_index].x = x;
                f_enemies[fly_index].y = y;
                f_enemies[fly_index].base_y = y;
                f_enemies[fly_index].vx = (rand() % 2 ? 1.0 : -1.0) * (1.0 + difficulty * 0.2);
                f_enemies[fly_index].angle = 0.0;
                f_enemies[fly_index].in_burst = false;
                f_enemies[fly_index].burst_shots_left = 0;
                f_enemies[fly_index].shot_interval = 0.05;
                f_enemies[fly_index].shot_timer = 0.0;
                f_enemies[fly_index].rest_timer = 0.5 + (rand() % 50) / 100.0;
                f_enemies[fly_index].max_hp = FLY_BASE_HP + FLY_HP_PER_ROUND * difficulty;
                f_enemies[fly_index].hp = f_enemies[fly_index].max_hp;
                
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
            enemies[i].alive = true;

            if (rand() % 2) enemies[i].x = -50 + rand() % 30;
            else enemies[i].x = map_width + (rand() % 30);

            enemies[i].y = get_enemy_ground_y(enemies[i].x) - 20; // ENEMY_H
            enemies[i].vx = 0.0;
            enemies[i].vy = 0.0;
            enemies[i].on_ground = true;

            enemies[i].max_hp = ENEMY_BASE_HP + ENEMY_HP_PER_ROUND * round_number;
            enemies[i].hp = enemies[i].max_hp;

            enemies[i].last_x = enemies[i].x;
            enemies[i].stuck_time = 0.0;

            enemies[i].speed = 1.2 + round_number * 0.15;
            enemies[i].accel = 0.15;
            enemies[i].friction = 0.90;

            count--;
        }
    }
}

void spawn_flying_enemy(int round_number) {
    int map_width = map_get_map_width();
    
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        if (!f_enemies[i].alive) {
            f_enemies[i].alive = true;
            f_enemies[i].x = rand() % map_width;
            f_enemies[i].base_y = 100 + rand() % 100;
            f_enemies[i].y = f_enemies[i].base_y;
            f_enemies[i].vx = (rand() % 2 ? 1.0 : -1.0) * (1.0 + round_number * 0.2);
            f_enemies[i].angle = 0.0;

            f_enemies[i].in_burst = false;
            f_enemies[i].burst_shots_left = 0;
            f_enemies[i].shot_interval = 0.05;
            f_enemies[i].shot_timer = 0.0;
            f_enemies[i].rest_timer = 0.5 + (rand() % 50) / 100.0;

            f_enemies[i].max_hp = FLY_BASE_HP + FLY_HP_PER_ROUND * round_number;
            f_enemies[i].hp = f_enemies[i].max_hp;
            break;
        }
    }
}

/* ===== Enemy Updates ===== */

void enemies_update(double dt) {
    const double gravity = 0.5;
    const double jump_power = -8.5;
    const double stuck_threshold = 1.0;
    const double stuck_jump_time = 2.0;
    int map_width = map_get_map_width();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;

        // Get tank position for AI
        double tank_x = get_tank_x();
        double dir = (tank_x > e->x) ? 1.0 : -1.0;
        double target_vx = dir * e->speed;

        double dv = target_vx - e->vx;
        if (dv > e->accel) dv = e->accel;
        if (dv < -e->accel) dv = -e->accel;
        e->vx += dv;
        e->vx *= e->friction;

        e->vy += gravity;

        e->x += e->vx;
        e->y += e->vy;

        double ground = get_enemy_ground_y(e->x);
        if (e->y > ground - 20) { // ENEMY_H
            e->y = ground - 20;
            e->vy = 0.0;
            e->on_ground = true;
        }
        else {
            e->on_ground = false;
        }

        if (e->x < 0) { e->x = 0; e->vx = fabs(e->vx); }
        if (e->x > map_width) { e->x = map_width; e->vx = -fabs(e->vx); }

        /* stuck detection */
        if (fabs(e->x - e->last_x) <= stuck_threshold) {
            e->stuck_time += dt;
        }
        else {
            e->stuck_time = 0.0;
            e->last_x = e->x;
        }

        if (e->stuck_time >= stuck_jump_time && e->on_ground) {
            e->vy = jump_power;
            e->vx += dir * 1.5; /* small horizontal boost */
            e->stuck_time = 0.0;
        }
    }
}

void flying_enemies_update(double dt) {
    int map_width = map_get_map_width();

    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;

        fe->angle += dt * 2.0;
        fe->y = fe->base_y + sin(fe->angle) * 30.0;
        fe->x += fe->vx;

        if (fe->x < 0) { fe->x = 0; fe->vx *= -1; }
        if (fe->x > map_width) { fe->x = map_width; fe->vx *= -1; }

        if (fe->in_burst) {
            fe->shot_timer -= dt;

            while (fe->shot_timer <= 0.0 && fe->burst_shots_left > 0) {
                // Create enemy bullet (this would need bullet system integration)
                // For now, just decrement shot counter
                fe->burst_shots_left--;
                fe->shot_timer += fe->shot_interval;

                if (fe->burst_shots_left <= 0) {
                    fe->in_burst = false;
                    fe->rest_timer = 2.0;
                }
            }
        }
        else {
            fe->rest_timer -= dt;
            if (fe->rest_timer <= 0.0) {
                fe->in_burst = true;
                fe->burst_shots_left = 10;
                fe->shot_timer = 0.0; /* fire immediately */
            }
        }
    }
}

/* ===== Enemy Rendering ===== */

void enemies_draw(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;
        
        // Draw enemy (basic rectangle for now)
        al_draw_filled_rectangle(e->x, e->y, e->x + 32, e->y + 20, al_map_rgb(200, 50, 50));
        
        // HP bar would be drawn by HUD system
    }
}

void flying_enemies_draw(void) {
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;
        
        // Draw flying enemy (basic rectangle for now)
        al_draw_filled_rectangle(fe->x, fe->y, fe->x + 28, fe->y + 16, al_map_rgb(180, 0, 180));
        
        // HP bar would be drawn by HUD system
    }
}

/* ===== Enemy Utilities ===== */

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

/* ===== Enemy Damage and Effects ===== */

void damage_enemy(Enemy* enemy, int damage) {
    if (!enemy || !enemy->alive) return;
    
    enemy->hp -= damage;
    if (enemy->hp <= 0) {
        enemy->alive = false;
    }
}

void damage_flying_enemy(FlyingEnemy* fe, int damage) {
    if (!fe || !fe->alive) return;
    
    fe->hp -= damage;
    if (fe->hp <= 0) {
        fe->alive = false;
    }
}

void apply_cannon_explosion(double ex, double ey, double radius) {
    /* ground enemies */
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;
        double cx = e->x + 32 * 0.5; // ENEMY_W * 0.5
        double cy = e->y + 20 * 0.5; // ENEMY_H * 0.5
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

    /* flying enemies: apply damage only */
    for (int i = 0; i < MAX_FLY_ENEMIES; ++i) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;
        double cx = fe->x + 28 * 0.5; // FLY_W * 0.5
        double cy = fe->y + 16 * 0.5; // FLY_H * 0.5
        double dx = cx - ex, dy = cy - ey;
        double dist = sqrt(dx * dx + dy * dy);
        int dmg = (int)(DMG_CANNON * (1.0 - (dist / radius)));
        if (dmg > 0) {
            damage_flying_enemy(fe, dmg);
        }
    }
}

/* ===== Enemy Movement Helpers ===== */

double get_enemy_ground_y(double x) {
    // This would need to integrate with map generation system
    // For now, return a simple ground level
    // TODO: Integrate with actual map system
    return 500.0; // Placeholder
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

/* ===== Getter functions for external access ===== */

Enemy* get_enemies(void) {
    return enemies;
}

FlyingEnemy* get_flying_enemies(void) {
    return f_enemies;
}
