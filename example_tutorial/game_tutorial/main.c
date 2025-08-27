#if 0

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BUFFER_W 1280
#define BUFFER_H 720
#define MAP_W 200
#define MAX_BULLETS 50

/* --- ���÷��� / ���� --- */
ALLEGRO_DISPLAY* disp;
ALLEGRO_BITMAP* buffer;
void disp_init() {
    disp = al_create_display(BUFFER_W, BUFFER_H);
    if (!disp) exit(1);
    buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
    if (!buffer) exit(1);
}
void disp_pre_draw() { al_set_target_bitmap(buffer); }
void disp_post_draw() {
    al_set_target_backbuffer(disp);
    al_draw_bitmap(buffer, 0, 0, 0);
    al_flip_display();
}

/* --- Ű���� �Է� --- */
typedef struct {
    bool left, right, jump;
    bool change_weapon;
    bool esc;
} Input;
Input input;

/* --- �� / ���� --- */
float map_height[MAP_W];

void map_init() {
    float base = BUFFER_H - 40;
    for (int i = 0; i < MAP_W; i++)
        map_height[i] = base + (rand() % 20 - 10);
}

float ground_y_at(int x_tile) {
    if (x_tile < 0) x_tile = 0;
    if (x_tile >= MAP_W) x_tile = MAP_W - 1;
    return map_height[x_tile];
}

/* --- ī�޶� --- */
typedef struct { float x, y; } Camera;
Camera camera;

/* --- ��ũ --- */
typedef struct {
    float x, y;
    float vx, vy;
    float cannon_angle;
    bool on_ground;
    int weapon; // 0 = MG, 1 = Cannon

    // ĳ�� ���� ����
    bool charging;
    float cannon_power;

    // �����(weapon==0) ���ӹ߻�/������ ����
    bool mg_firing;            // ���콺 ������ ���� ���� ������
    float mg_fire_time;        // ���� ���� �߻� ���� �ð� (��) ? ���� ����: ���� ����
    float mg_shot_cooldown;   // ���� �߻� ���ɱ��� ���� �ð�
    bool mg_reloading;         // ������ ������
    float mg_reload_time;      // ������ ���� �ð� (��)
} Tank;
Tank tank;

void tank_init() {
    tank.x = 50;
    tank.y = ground_y_at((int)tank.x) - 20;
    tank.vx = tank.vy = 0;
    tank.on_ground = true;
    tank.cannon_angle = M_PI / 4;
    tank.weapon = 0;
    tank.charging = false;
    tank.cannon_power = 0;

    tank.mg_firing = false;
    tank.mg_fire_time = 0.0f;
    tank.mg_shot_cooldown = 0.0f;
    tank.mg_reloading = false;
    tank.mg_reload_time = 0.0f;
}

/* --- źȯ --- */
typedef struct {
    float x, y;
    float vx, vy;
    bool alive;
    int weapon;
} Bullet;
Bullet bullets[MAX_BULLETS];

void bullets_init() {
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].alive = false;
}

void shoot_bullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) {
            bullets[i].alive = true;
            bullets[i].x = tank.x + 16;
            bullets[i].y = tank.y + 10;
            bullets[i].weapon = tank.weapon;
            float speed;
            if (tank.weapon == 0) {
                // ����� �⺻ speed (���� 8.0f * 0.7f) �� 1.5�� ���� ����
                speed = (8.0f * 0.7f) * 1.5f;
            }
            else {
                // ĳ��: �ӵ��� ������ ��� (���� ���)
                speed = tank.cannon_power * 0.7f;
            }
            bullets[i].vx = cosf(tank.cannon_angle) * speed;
            bullets[i].vy = sinf(tank.cannon_angle) * speed; // y�� �Ʒ� ����
            break;
        }
    }
}

void bullets_update() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) continue;
        if (bullets[i].weapon == 1) bullets[i].vy += 0.3f; // ĳ�� �߷�
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;
        if (bullets[i].x < 0 || bullets[i].x >= MAP_W * 4 || bullets[i].y >= BUFFER_H)
            bullets[i].alive = false;
    }
}

/* --- ��ũ ������Ʈ --- */
void tank_update() {
    const float accel = 0.4f;
    const float maxspeed = 3.0f;
    const float friction = 0.85f;
    const float gravity = 0.5f;
    const float dt = 1.0f / 60.0f; // Ÿ�̸� �ֱ� ����

    if (input.left) tank.vx -= accel;
    if (input.right) tank.vx += accel;
    tank.vx *= friction;
    if (tank.vx > maxspeed) tank.vx = maxspeed;
    if (tank.vx < -maxspeed) tank.vx = -maxspeed;

    tank.x += tank.vx;

    if (input.jump && tank.on_ground) {
        tank.vy = -8;
        tank.on_ground = false;
    }

    tank.vy += gravity;
    tank.y += tank.vy;

    float ground = ground_y_at((int)tank.x);
    if (tank.y > ground - 20) { tank.y = ground - 20; tank.vy = 0; tank.on_ground = true; }

    if (input.change_weapon) { tank.weapon = 1 - tank.weapon; input.change_weapon = false; }

    // ĳ�� ���� ó��
    if (tank.weapon == 1) {
        if (tank.charging) {
            tank.cannon_power += 0.2f;
            if (tank.cannon_power > 15.0f) tank.cannon_power = 15.0f;
        }
    }

    // === �����(weapon == 0) ���� / ������ ���� ===
    if (tank.weapon == 0) {
        // ��ٿ� ����
        if (tank.mg_shot_cooldown > 0.0f) tank.mg_shot_cooldown -= dt;

        if (tank.mg_reloading) {
            // ������ ����
            tank.mg_reload_time -= dt;
            if (tank.mg_reload_time <= 0.0f) {
                tank.mg_reloading = false;
                tank.mg_reload_time = 0.0f;
                tank.mg_fire_time = 0.0f;
                tank.mg_shot_cooldown = 0.0f;
            }
        }
        else {
            // ������ ���� �ƴ� ���� �߻� ���
            if (tank.mg_firing) {
                // ���� �߻� ���� (�������� ������ �ִ� ���ȸ� ����)
                tank.mg_fire_time += dt;

                // �߻� �ӵ�: �ʴ� �� 10�� => ���� 0.1��
                if (tank.mg_shot_cooldown <= 0.0f) {
                    shoot_bullet();
                    tank.mg_shot_cooldown = 0.1f;
                }

                // ���� �߻� �ð��� 5�� �����ϸ� ������ ����
                if (tank.mg_fire_time >= 5.0f) {
                    tank.mg_reloading = true;
                    tank.mg_reload_time = 3.0f;
                    tank.mg_firing = false; // �ڵ����� ���� ����
                }
            }
            else {
                // mg_firing false �̸� ���� ���� �ʱ�ȭ (�䱸����: '�� ������' ����)
                tank.mg_fire_time = 0.0f;
            }
        }
    }

    bullets_update();

    // ī�޶�
    camera.x = tank.x - BUFFER_W / 3;
    camera.y = tank.y - BUFFER_H / 2;
}

/* --- �׸��� --- */
void draw_tank() {
    float screen_x = tank.x - camera.x;
    float screen_y = tank.y - camera.y;

    al_draw_filled_rectangle(screen_x, screen_y, screen_x + 32, screen_y + 20, al_map_rgb(60, 120, 180));

    float cx = screen_x + 16;
    float cy = screen_y + 10;
    float bx = cx + cosf(tank.cannon_angle) * 18;
    float by = cy + sinf(tank.cannon_angle) * 18;
    al_draw_line(cx, cy, bx, by, al_map_rgb(200, 200, 0), 4);

    // ĳ�� ���� �� ������ ����
    if (tank.charging && tank.weapon == 1) {
        float px = tank.x;
        float py = tank.y + 10;
        float vx = cosf(tank.cannon_angle) * tank.cannon_power;
        float vy = sinf(tank.cannon_angle) * tank.cannon_power;
        for (int i = 0; i < 60; i++) {
            px += vx;
            py += vy;
            vy += 0.3f; // �߷�
            float sx = px - camera.x;
            float sy = py - camera.y;
            if (sx < 0 || sx > BUFFER_W || sy > BUFFER_H) break;
            al_draw_filled_circle(sx, sy, 2, al_map_rgb(255, 255, 255));
        }

        // ������ ǥ�� (ĳ��)
        float gauge_w = tank.cannon_power * 10;
        al_draw_filled_rectangle(screen_x, screen_y - 20, screen_x + gauge_w, screen_y - 10, al_map_rgb(255, 0, 0));
        al_draw_rectangle(screen_x, screen_y - 20, screen_x + 150, screen_y - 10, al_map_rgb(255, 255, 255), 2);
    }

    // ������ ������: ����� ������ ���� �� �Ӹ� ���� ǥ��
    if (tank.mg_reloading) {
        float total = 3.0f;
        float filled = (total - tank.mg_reload_time) / total; // 0..1
        if (filled < 0.0f) filled = 0.0f;
        if (filled > 1.0f) filled = 1.0f;
        float full_w = 150.0f;
        float gw = full_w * filled;
        // ��� (�׵θ�)
        al_draw_rectangle(screen_x, screen_y - 35, screen_x + full_w, screen_y - 20, al_map_rgb(255, 255, 255), 2);
        // ä���� �κ�
        al_draw_filled_rectangle(screen_x + 1, screen_y - 34, screen_x + 1 + gw, screen_y - 21, al_map_rgb(0, 200, 255));
    }
}

void draw_map() {
    for (int i = 0; i < MAP_W; i++) {
        float sx = i * 4 - camera.x;
        float sy = map_height[i] - camera.y;
        al_draw_filled_rectangle(sx, sy, sx + 4, BUFFER_H, al_map_rgb(30, 150, 40));
    }
}

void draw_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) continue;
        float sx = bullets[i].x - camera.x;
        float sy = bullets[i].y - camera.y;
        ALLEGRO_COLOR col = (bullets[i].weapon == 0) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 128, 0);
        al_draw_filled_circle(sx, sy, 4, col);
    }
}

/* --- ���� --- */
int main() {
    srand((unsigned int)time(NULL));
    if (!al_init()) return 1;
    al_install_keyboard();
    al_install_mouse();
    al_init_primitives_addon();

    disp_init();

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    map_init();
    tank_init();
    bullets_init();

    al_start_timer(timer);
    bool done = false;
    ALLEGRO_EVENT event;

    while (!done) {
        al_wait_for_event(queue, &event);

        switch (event.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE: done = true; break;
        case ALLEGRO_EVENT_TIMER: tank_update(); break;
        case ALLEGRO_EVENT_KEY_DOWN:
            if (event.keyboard.keycode == ALLEGRO_KEY_A) input.left = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_D) input.right = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_W) input.jump = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_R) input.change_weapon = true;
            break;
        case ALLEGRO_EVENT_KEY_UP:
            if (event.keyboard.keycode == ALLEGRO_KEY_A) input.left = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_D) input.right = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_W) input.jump = false;
            break;
        case ALLEGRO_EVENT_MOUSE_AXES: {
            float cx = (tank.x - camera.x) + 16;
            float cy = (tank.y - camera.y) + 10;
            float dx = event.mouse.x - cx;
            float dy = event.mouse.y - cy;
            tank.cannon_angle = atan2f(dy, dx);
            break;
        }
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (event.mouse.button == 1) { // ��Ŭ��
                if (tank.weapon == 1) {
                    tank.charging = true;
                    tank.cannon_power = 0;
                }
                else {
                    // �����: ������ ������ ���� ���� (��, ������ ���̸� ����)
                    if (!tank.mg_reloading) {
                        tank.mg_firing = true;
                        // �߻� ��� �ѹߵ� �����ϵ��� ��ٿ��� 0���� �صӴϴ�.
                        tank.mg_shot_cooldown = 0.0f;
                    }
                }
            }
            break;
        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            if (event.mouse.button == 1) {
                if (tank.weapon == 1 && tank.charging) {
                    shoot_bullet();
                    tank.charging = false;
                    tank.cannon_power = 0;
                }
                else {
                    // �����: ���� ���� -> ���� ���� �� ���� �ʱ�ȭ (�䱸: '�� ������' ����)
                    if (tank.weapon == 0) {
                        tank.mg_firing = false;
                        tank.mg_fire_time = 0.0f;
                    }
                }
            }
            break;
        }

        disp_pre_draw();
        al_clear_to_color(al_map_rgb(20, 20, 30));
        draw_map();
        draw_tank();
        draw_bullets();
        disp_post_draw();
    }

    al_destroy_bitmap(buffer);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    return 0;
}

#endif

#if 0



#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

/* ============================================================
   ��� ����
   ============================================================ */
#define BUFFER_W 1280
#define BUFFER_H 720
#define MAP_W 200
#define MAX_BULLETS 50

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

   /* ============================================================
      ���÷��� / ����
      ============================================================ */
ALLEGRO_DISPLAY* disp;
ALLEGRO_BITMAP* buffer;

void disp_init() {
    disp = al_create_display(BUFFER_W, BUFFER_H);
    buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
}

void disp_pre_draw() { al_set_target_bitmap(buffer); }

void disp_post_draw() {
    al_set_target_backbuffer(disp);
    al_draw_bitmap(buffer, 0, 0, 0);
    al_flip_display();
}

/* ============================================================
   �Է� (Input)
   ============================================================ */
typedef struct {
    bool left, right, jump;
    bool change_weapon;
    bool esc;
    bool mouse_left;
    double mouse_x, mouse_y;
} Input;

Input input = { 0 };

/* ============================================================
   �� (Map)
   ============================================================ */
typedef struct {
    double height[MAP_W]; // �� Ÿ���� ���� ����
} Map;

Map map;

void map_init(Map* m) {
    double base = BUFFER_H - 40;
    for (int i = 0; i < MAP_W; i++) {
        m->height[i] = base + (rand() % 20 - 10);
    }
}

double map_ground_y(Map* m, int x_tile) {
    if (x_tile < 0) x_tile = 0;
    if (x_tile >= MAP_W) x_tile = MAP_W - 1;
    return m->height[x_tile];
}

void map_draw(Map* m, double cam_x, double cam_y) {
    for (int i = 0; i < MAP_W; i++) {
        double sx = i * 4 - cam_x;
        double sy = m->height[i] - cam_y;
        al_draw_filled_rectangle(sx, sy, sx + 4, BUFFER_H, al_map_rgb(30, 150, 40));
    }
}

/* ============================================================
   ���� (Weapon)
   ============================================================ */
typedef struct {
    double x, y;
    double vx, vy;
    bool alive;
    int type; // 0 = �����, 1 = ĳ��
} Bullet;

typedef struct {
    Bullet bullets[MAX_BULLETS];
} WeaponSystem;

void weapon_init(WeaponSystem* ws) {
    for (int i = 0; i < MAX_BULLETS; i++) ws->bullets[i].alive = false;
}

void weapon_shoot(WeaponSystem* ws, double x, double y, double angle, int type, double power) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet* b = &ws->bullets[i];
        if (!b->alive) {
            b->alive = true;
            b->x = x;
            b->y = y;
            b->type = type;
            double speed = (type == 0) ? 12.0 : power * 0.7 * 2.0;
            b->vx = cos(angle) * speed;
            b->vy = sin(angle) * speed;
            break;
        }
    }
}

void weapon_update(WeaponSystem* ws) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet* b = &ws->bullets[i];
        if (!b->alive) continue;
        if (b->type == 1) b->vy += 0.3; // ĳ�� �߷�
        b->x += b->vx;
        b->y += b->vy;
        if (b->x < 0 || b->x >= BUFFER_W * 2 || b->y >= BUFFER_H) b->alive = false;
    }
}

void weapon_draw(WeaponSystem* ws, double cam_x, double cam_y) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet* b = &ws->bullets[i];
        if (!b->alive) continue;
        double sx = b->x - cam_x;
        double sy = b->y - cam_y;
        ALLEGRO_COLOR col = (b->type == 0) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 128, 0);
        al_draw_filled_circle(sx, sy, 4, col);
    }
}

/* ============================================================
   ��ũ (Tank)
   ============================================================ */
typedef struct {
    double x, y, vx, vy;
    bool on_ground;

    double cannon_angle;
    int weapon; // 0 = MG, 1 = Cannon

    // ĳ�� ����
    bool charging;
    double cannon_power;

    // ����� ����/������
    bool mg_firing;
    double mg_fire_time;
    double mg_shot_cooldown;
    bool mg_reloading;
    double mg_reload_time;
} Tank;

typedef struct {
    double x, y; // ī�޶� ��ġ
} Camera;

Tank tank;
Camera camera;
WeaponSystem weapon;

void tank_init(Tank* t, Map* m) {
    t->x = 50;
    t->y = map_ground_y(m, (int)t->x) - 20;
    t->vx = t->vy = 0;
    t->on_ground = true;
    t->cannon_angle = M_PI / 4;
    t->weapon = 0;
    t->charging = false;
    t->cannon_power = 0;
    t->mg_firing = false;
    t->mg_fire_time = 0;
    t->mg_shot_cooldown = 0;
    t->mg_reloading = false;
    t->mg_reload_time = 0;
}

void tank_update(Tank* t, Map* m, WeaponSystem* ws) {
    const double accel = 0.4, maxspeed = 3.0, friction = 0.85, gravity = 0.5;
    const double dt = 1.0 / 60.0;

    if (input.left) t->vx -= accel;
    if (input.right) t->vx += accel;
    t->vx *= friction;
    if (t->vx > maxspeed) t->vx = maxspeed;
    if (t->vx < -maxspeed) t->vx = -maxspeed;
    t->x += t->vx;

    if (input.jump && t->on_ground) { t->vy = -8; t->on_ground = false; }
    t->vy += gravity;
    t->y += t->vy;

    double ground = map_ground_y(m, (int)t->x);
    if (t->y > ground - 20) { t->y = ground - 20; t->vy = 0; t->on_ground = true; }

    if (input.change_weapon) { t->weapon = 1 - t->weapon; input.change_weapon = false; }

    // ĳ�� ����
    if (t->weapon == 1 && t->charging) {
        t->cannon_power += 0.2;
        if (t->cannon_power > 15.0) t->cannon_power = 15.0;
    }

    // ����� �߻�/������
    if (t->weapon == 0) {
        if (t->mg_shot_cooldown > 0) t->mg_shot_cooldown -= dt;
        if (t->mg_reloading) {
            t->mg_reload_time -= dt;
            if (t->mg_reload_time <= 0) {
                t->mg_reloading = false;
                t->mg_fire_time = 0;
            }
        }
        else if (t->mg_firing) {
            t->mg_fire_time += dt;
            if (t->mg_shot_cooldown <= 0) {
                weapon_shoot(ws, t->x + 16, t->y + 10, t->cannon_angle, 0, 0);
                t->mg_shot_cooldown = 0.1;
            }
            if (t->mg_fire_time >= 3.0) {
                t->mg_reloading = true;
                t->mg_reload_time = 2.0;
                t->mg_firing = false;
            }
        }
    }

    weapon_update(ws);
    camera.x = t->x - BUFFER_W / 3;
    camera.y = t->y - BUFFER_H / 2;
}

void tank_draw(Tank* t) {
    double sx = t->x - camera.x;
    double sy = t->y - camera.y;
    al_draw_filled_rectangle(sx, sy, sx + 32, sy + 20, al_map_rgb(60, 120, 180));

    double cx = sx + 16, cy = sy + 10;
    double bx = cx + cos(t->cannon_angle) * 18;
    double by = cy + sin(t->cannon_angle) * 18;
    al_draw_line(cx, cy, bx, by, al_map_rgb(200, 200, 0), 4);

    // ����� ������ ������
    if (t->mg_reloading) {
        double total = 2.0;
        double filled = (total - t->mg_reload_time) / total;
        if (filled < 0) filled = 0;
        if (filled > 1) filled = 1;
        double full_w = 50;
        double gw = full_w * filled;
        al_draw_rectangle(sx, sy - 20, sx + full_w, sy - 15, al_map_rgb(255, 255, 255), 2);
        al_draw_filled_rectangle(sx, sy - 20, sx + gw, sy - 15, al_map_rgb(0, 200, 255));
    }

    // ĳ�� ���� ������
    if (t->weapon == 1) {
        double gauge_w = t->cannon_power * 10;
        al_draw_rectangle(sx, sy - 30, sx + 150, sy - 25, al_map_rgb(255, 255, 255), 2);
        al_draw_filled_rectangle(sx, sy - 30, sx + gauge_w, sy - 25, al_map_rgb(255, 0, 0));
    }
}

/* ============================================================
   ���� ����
   ============================================================ */
int main() {
    srand((unsigned int)time(NULL));
    al_init(); al_install_keyboard(); al_install_mouse(); al_init_primitives_addon();

    disp_init();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    map_init(&map);
    tank_init(&tank, &map);
    weapon_init(&weapon);

    al_start_timer(timer);
    bool done = false;
    ALLEGRO_EVENT ev;

    while (!done) {
        al_wait_for_event(queue, &ev);
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) done = true;
        else if (ev.type == ALLEGRO_EVENT_TIMER) tank_update(&tank, &map, &weapon);
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            double cx = (tank.x - camera.x) + 16;
            double cy = (tank.y - camera.y) + 10;
            double dx = ev.mouse.x - cx, dy = ev.mouse.y - cy;
            tank.cannon_angle = atan2(dy, dx);
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (ev.mouse.button == 1) {
                if (tank.weapon == 1) { tank.charging = true; tank.cannon_power = 0; }
                else if (!tank.mg_reloading) { tank.mg_firing = true; tank.mg_shot_cooldown = 0; }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            if (ev.mouse.button == 1) {
                if (tank.weapon == 1 && tank.charging) {
                    weapon_shoot(&weapon, tank.x + 16, tank.y + 10, tank.cannon_angle, 1, tank.cannon_power);
                    tank.charging = false; tank.cannon_power = 0;
                }
                else { tank.mg_firing = false; }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_A) input.left = true;
            if (ev.keyboard.keycode == ALLEGRO_KEY_D) input.right = true;
            if (ev.keyboard.keycode == ALLEGRO_KEY_W) input.jump = true;
            if (ev.keyboard.keycode == ALLEGRO_KEY_R) input.change_weapon = true;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_A) input.left = false;
            if (ev.keyboard.keycode == ALLEGRO_KEY_D) input.right = false;
            if (ev.keyboard.keycode == ALLEGRO_KEY_W) input.jump = false;
        }

        disp_pre_draw();
        al_clear_to_color(al_map_rgb(20, 20, 30));
        map_draw(&map, camera.x, camera.y);
        tank_draw(&tank);
        weapon_draw(&weapon, camera.x, camera.y);
        disp_post_draw();
    }
}

#endif // 0

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

/* --- ��� ���� --- */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BUFFER_W 1280
#define BUFFER_H 720
#define MAP_W 200
#define MAX_BULLETS 100
#define MAX_ENEMIES 20
#define MAX_ROUNDS 10

/* =========================
    ����ü ����
========================= */

/* --- Ű���� �Է� --- */
typedef struct {
    bool left, right, jump;
    bool change_weapon;
    bool esc;
} Input;

/* --- ī�޶� --- */
typedef struct {
    double x, y;
} Camera;

/* --- �� --- */
typedef struct {
    double height[MAP_W];
} Map;

/* --- źȯ --- */
typedef struct {
    double x, y;
    double vx, vy;
    bool alive;
    int weapon; // 0=MG,1=Cannon,2=Laser,3=Missile
} Bullet;

/* --- ��ũ --- */
typedef struct {
    double x, y;
    double vx, vy;
    double cannon_angle;
    bool on_ground;
    int weapon; // 0=MG,1=Cannon

    // ĳ�� ����
    bool charging;
    double cannon_power;

    // ����� ����/������
    bool mg_firing;
    double mg_fire_time;
    double mg_shot_cooldown;
    bool mg_reloading;
    double mg_reload_time;
} Tank;

/* --- �� --- */
typedef struct {
    double x, y, vx, vy;
    double cannon_angle;
    int weapon;
    bool alive;
    double fire_cooldown;
    int hp;
} Enemy;

/* =========================
    ���� ����
========================= */
ALLEGRO_DISPLAY* disp;
ALLEGRO_BITMAP* buffer;
Input input = { 0 };
Camera camera = { 0 };
Map map;
Tank tank;
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
int round_number = 1;

/* =========================
    ���÷��� �ʱ�ȭ
========================= */
void disp_init() {
    disp = al_create_display(BUFFER_W, BUFFER_H);
    if (!disp) exit(1);
    buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
    if (!buffer) exit(1);
}
void disp_pre_draw() { al_set_target_bitmap(buffer); }
void disp_post_draw() {
    al_set_target_backbuffer(disp);
    al_draw_bitmap(buffer, 0, 0, 0);
    al_flip_display();
}

/* =========================
    �� ����
========================= */
void map_init() {
    double base = BUFFER_H - 40;
    for (int i = 0; i < MAP_W; i++)
        map.height[i] = base + (rand() % 20 - 10);
}

// Ư�� x ��ġ���� ���� y ��ǥ ��������
double map_ground_y(int x_tile) {
    if (x_tile < 0) x_tile = 0;
    if (x_tile >= MAP_W) x_tile = MAP_W - 1;
    return map.height[x_tile];
}

void draw_map() {
    for (int i = 0; i < MAP_W; i++) {
        double sx = i * 4 - camera.x;
        double sy = map.height[i] - camera.y;
        al_draw_filled_rectangle(sx, sy, sx + 4, BUFFER_H, al_map_rgb(30, 150, 40));
    }
}

/* =========================
    ��ũ ����
========================= */
void tank_init() {
    tank.x = 50;
    tank.y = map_ground_y((int)tank.x) - 20;
    tank.vx = tank.vy = 0;
    tank.on_ground = true;
    tank.cannon_angle = M_PI / 4;
    tank.weapon = 0;

    tank.charging = false;
    tank.cannon_power = 0;

    tank.mg_firing = false;
    tank.mg_fire_time = 0;
    tank.mg_shot_cooldown = 0;
    tank.mg_reloading = false;
    tank.mg_reload_time = 0;
}

void draw_tank() {
    double sx = tank.x - camera.x;
    double sy = tank.y - camera.y;

    // ��ũ ��ü
    al_draw_filled_rectangle(sx, sy, sx + 32, sy + 20, al_map_rgb(60, 120, 180));

    // ����
    double cx = sx + 16;
    double cy = sy + 10;
    double bx = cx + cos(tank.cannon_angle) * 18;
    double by = cy + sin(tank.cannon_angle) * 18;
    al_draw_line(cx, cy, bx, by, al_map_rgb(200, 200, 0), 4);

    // ĳ�� ���� ������
    if (tank.charging && tank.weapon == 1) {
        double gauge_w = tank.cannon_power * 10;
        al_draw_filled_rectangle(sx, sy - 20, sx + gauge_w, sy - 10, al_map_rgb(255, 0, 0));
        al_draw_rectangle(sx, sy - 20, sx + 150, sy - 10, al_map_rgb(255, 255, 255), 2);
    }

    // ����� ������ ������
    if (tank.mg_reloading) {
        double total = 2.0;
        double filled = (total - tank.mg_reload_time) / total;
        if (filled < 0) filled = 0;
        if (filled > 1) filled = 1;
        double full_w = 150;
        double gw = full_w * filled;
        al_draw_rectangle(sx, sy - 35, sx + full_w, sy - 20, al_map_rgb(255, 255, 255), 2);
        al_draw_filled_rectangle(sx + 1, sy - 34, sx + 1 + gw, sy - 21, al_map_rgb(0, 200, 255));
    }
}

void tank_update(double dt) {
    const double accel = 0.4;
    const double maxspeed = 3.0;
    const double friction = 0.85;
    const double gravity = 0.5;

    // �̵�
    if (input.left) tank.vx -= accel;
    if (input.right) tank.vx += accel;
    tank.vx *= friction;
    if (tank.vx > maxspeed) tank.vx = maxspeed;
    if (tank.vx < -maxspeed) tank.vx = -maxspeed;
    tank.x += tank.vx;

    // ����
    if (input.jump && tank.on_ground) { tank.vy = -8; tank.on_ground = false; }
    tank.vy += gravity;
    tank.y += tank.vy;

    double ground = map_ground_y((int)tank.x);
    if (tank.y > ground - 20) { tank.y = ground - 20; tank.vy = 0; tank.on_ground = true; }

    // ���� ����
    if (input.change_weapon) { tank.weapon = 1 - tank.weapon; input.change_weapon = false; }

    // ĳ�� ����
    if (tank.weapon == 1 && tank.charging) {
        tank.cannon_power += 0.2;
        if (tank.cannon_power > 15) tank.cannon_power = 15;
    }

    // ����� ����
    if (tank.weapon == 0) {
        if (tank.mg_shot_cooldown > 0) tank.mg_shot_cooldown -= dt;

        if (tank.mg_reloading) {
            tank.mg_reload_time -= dt;
            if (tank.mg_reload_time <= 0) {
                tank.mg_reloading = false;
                tank.mg_fire_time = 0;
                tank.mg_shot_cooldown = 0;
            }
        }
        else {
            if (tank.mg_firing) {
                tank.mg_fire_time += dt;
                if (tank.mg_shot_cooldown <= 0) {
                    // �߻�
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].alive) {
                            bullets[i].alive = true;
                            bullets[i].x = tank.x + 16;
                            bullets[i].y = tank.y + 10;
                            bullets[i].weapon = 0;
                            bullets[i].vx = cos(tank.cannon_angle) * 8.0 * 1.5;
                            bullets[i].vy = sin(tank.cannon_angle) * 8.0 * 1.5;
                            tank.mg_shot_cooldown = 0.1;
                            break;
                        }
                    }
                }
                if (tank.mg_fire_time >= 3.0) {
                    tank.mg_reloading = true;
                    tank.mg_reload_time = 2.0;
                    tank.mg_firing = false;
                }
            }
            else {
                tank.mg_fire_time = 0;
            }
        }
    }

    camera.x = tank.x - BUFFER_W / 3;
    camera.y = tank.y - BUFFER_H / 2;
}

/* =========================
    źȯ ����
========================= */
void bullets_update() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) continue;

        // �߷�
        if (bullets[i].weapon == 1) bullets[i].vy += 0.3;

        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;

        // ȭ�� ���̸� ����
        if (bullets[i].x < 0 || bullets[i].x > MAP_W * 4 || bullets[i].y > BUFFER_H) bullets[i].alive = false;
    }
}

void draw_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) continue;
        double sx = bullets[i].x - camera.x;
        double sy = bullets[i].y - camera.y;
        ALLEGRO_COLOR col = (bullets[i].weapon == 0) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 128, 0);
        al_draw_filled_circle(sx, sy, 4, col);
    }
}

/* =========================
    �� ����
========================= */
void spawn_enemies() {
    int count = round_number + 2;
    for (int i = 0; i < MAX_ENEMIES && count > 0; i++) {
        if (!enemies[i].alive) {
            enemies[i].alive = true;
            if (rand() % 2) enemies[i].x = -50 + rand() % 30;
            else enemies[i].x = BUFFER_W + rand() % 30;
            enemies[i].y = map_ground_y((int)(enemies[i].x / 4)) - 20;
            enemies[i].vx = (rand() % 2 ? 1 : -1) * (1.0 + round_number * 0.2);
            enemies[i].cannon_angle = M_PI / 4;
            enemies[i].weapon = rand() % 2;
            enemies[i].fire_cooldown = rand() % 60 / 60.0;
            enemies[i].hp = 10 + round_number * 5;
            count--;
        }
    }
}

void enemies_update(double dt) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;

        // �̵�
        e->x += e->vx;
        double ground = map_ground_y((int)(e->x / 4));
        if (e->y > ground - 20) { e->y = ground - 20; e->vy = 0; }

        // �÷��̾� ����
        double dx = tank.x - e->x;
        double dy = tank.y - e->y;
        e->cannon_angle = atan2(dy, dx);

        // �߻�
        e->fire_cooldown -= dt;
        if (e->fire_cooldown <= 0) {
            for (int j = 0; j < MAX_BULLETS; j++) {
                if (!bullets[j].alive) {
                    bullets[j].alive = true;
                    bullets[j].x = e->x + 16;
                    bullets[j].y = e->y + 10;
                    bullets[j].weapon = e->weapon;
                    bullets[j].vx = cos(e->cannon_angle) * 6.0;
                    bullets[j].vy = sin(e->cannon_angle) * 6.0;
                    e->fire_cooldown = 1.0 + rand() % 60 / 30.0;
                    break;
                }
            }
        }
    }
}

void enemies_draw() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;
        double sx = e->x - camera.x;
        double sy = e->y - camera.y;

        // ��ü
        al_draw_filled_rectangle(sx, sy, sx + 32, sy + 20, al_map_rgb(200, 50, 50));
        // ����
        al_draw_line(sx + 16, sy + 10, sx + 16 + cos(e->cannon_angle) * 18, sy + 10 + sin(e->cannon_angle) * 18,
            al_map_rgb(255, 255, 0), 4);
    }
}

/* =========================
    ����
========================= */
int main() {
    srand((unsigned int)time(NULL));
    if (!al_init()) return 1;
    al_install_keyboard();
    al_install_mouse();
    al_init_primitives_addon();

    disp_init();

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    map_init();
    tank_init();
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].alive = false;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].alive = false;

    al_start_timer(timer);

    bool done = false;
    ALLEGRO_EVENT event;

    spawn_enemies(); // ���� ���� �� ����

    while (!done) {
        al_wait_for_event(queue, &event);

        switch (event.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE: done = true; break;
        case ALLEGRO_EVENT_TIMER:
            tank_update(1.0 / 60);
            bullets_update();
            enemies_update(1.0 / 60);
            break;
        case ALLEGRO_EVENT_KEY_DOWN:
            if (event.keyboard.keycode == ALLEGRO_KEY_A) input.left = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_D) input.right = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_W) input.jump = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_R) input.change_weapon = true;
            break;
        case ALLEGRO_EVENT_KEY_UP:
            if (event.keyboard.keycode == ALLEGRO_KEY_A) input.left = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_D) input.right = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_W) input.jump = false;
            break;
        case ALLEGRO_EVENT_MOUSE_AXES: {
            double cx = tank.x - camera.x + 16;
            double cy = tank.y - camera.y + 10;
            double dx = event.mouse.x - cx;
            double dy = event.mouse.y - cy;
            tank.cannon_angle = atan2(dy, dx);
            break;
        }
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (event.mouse.button == 1) {
                if (tank.weapon == 1) { tank.charging = true; tank.cannon_power = 0; }
                else if (!tank.mg_reloading) tank.mg_firing = true;
            }
            break;
        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            if (event.mouse.button == 1) {
                if (tank.weapon == 1 && tank.charging) {
                    // ĳ�� �߻�
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].alive) {
                            bullets[i].alive = true;
                            bullets[i].x = tank.x + 16;
                            bullets[i].y = tank.y + 10;
                            bullets[i].weapon = 1;
                            bullets[i].vx = cos(tank.cannon_angle) * tank.cannon_power * 0.7;
                            bullets[i].vy = sin(tank.cannon_angle) * tank.cannon_power * 0.7;
                            break;
                        }
                    }
                    tank.charging = false; tank.cannon_power = 0;
                }
                else if (tank.weapon == 0) {
                    tank.mg_firing = false; tank.mg_fire_time = 0;
                }
            }
            break;
        }

        disp_pre_draw();
        al_clear_to_color(al_map_rgb(20, 20, 30));
        draw_map();
        draw_tank();
        draw_bullets();
        enemies_draw();
        disp_post_draw();
    }

    al_destroy_bitmap(buffer);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    return 0;
}
