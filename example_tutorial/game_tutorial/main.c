/* this is a complete copy of the source from allegro vivace's 'gameplay' section.
 *
 * for gcc users, it can be compiled & run with:
 *
 * gcc game.c -o game $(pkg-config allegro-5 allegro_font-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 allegro_image-5 --libs --cflags)
 * ./game
 */

#if 0

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>


 // --- general ---

long frames;
long score;

void must_init(bool test, const char* description)
{
    if (test) return;

    printf("couldn't initialize %s\n", description);
    exit(1);
}

int between(int lo, int hi)
{
    return lo + (rand() % (hi - lo));
}

float between_f(float lo, float hi)
{
    return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo);
}

bool collide(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2)
{
    if (ax1 > bx2) return false;
    if (ax2 < bx1) return false;
    if (ay1 > by2) return false;
    if (ay2 < by1) return false;

    return true;
}


// --- display ---

#define BUFFER_W 320
#define BUFFER_H 240

#define DISP_SCALE 3
#define DISP_W (BUFFER_W * DISP_SCALE)
#define DISP_H (BUFFER_H * DISP_SCALE)

ALLEGRO_DISPLAY* disp;
ALLEGRO_BITMAP* buffer;

void disp_init()
{
    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);

    disp = al_create_display(DISP_W, DISP_H);
    must_init(disp, "display");

    buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
    must_init(buffer, "bitmap buffer");
}

void disp_deinit()
{
    al_destroy_bitmap(buffer);
    al_destroy_display(disp);
}

void disp_pre_draw()
{
    al_set_target_bitmap(buffer);
}

void disp_post_draw()
{
    al_set_target_backbuffer(disp);
    al_draw_scaled_bitmap(buffer, 0, 0, BUFFER_W, BUFFER_H, 0, 0, DISP_W, DISP_H, 0);

    al_flip_display();
}


// --- keyboard ---

#define KEY_SEEN     1
#define KEY_DOWN     2
unsigned char key[ALLEGRO_KEY_MAX];

void keyboard_init()
{
    memset(key, 0, sizeof(key));
}

void keyboard_update(ALLEGRO_EVENT* event)
{
    switch (event->type)
    {
    case ALLEGRO_EVENT_TIMER:
        for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
            key[i] &= ~KEY_SEEN;
        break;

    case ALLEGRO_EVENT_KEY_DOWN:
        key[event->keyboard.keycode] = KEY_SEEN | KEY_DOWN;
        break;
    case ALLEGRO_EVENT_KEY_UP:
        key[event->keyboard.keycode] &= ~KEY_DOWN;
        break;
    }
}


// --- sprites ---

#define SHIP_W 12
#define SHIP_H 13

#define SHIP_SHOT_W 2
#define SHIP_SHOT_H 9

#define LIFE_W 6
#define LIFE_H 6

const int ALIEN_W[] = { 14, 13, 45 };
const int ALIEN_H[] = { 9, 10, 27 };

#define ALIEN_BUG_W      ALIEN_W[0]
#define ALIEN_BUG_H      ALIEN_H[0]
#define ALIEN_ARROW_W    ALIEN_W[1]
#define ALIEN_ARROW_H    ALIEN_H[1]
#define ALIEN_THICCBOI_W ALIEN_W[2]
#define ALIEN_THICCBOI_H ALIEN_H[2]

#define ALIEN_SHOT_W 4
#define ALIEN_SHOT_H 4

#define EXPLOSION_FRAMES 4
#define SPARKS_FRAMES    3


typedef struct SPRITES
{
    ALLEGRO_BITMAP* _sheet;

    ALLEGRO_BITMAP* ship;
    ALLEGRO_BITMAP* ship_shot[2];
    ALLEGRO_BITMAP* life;

    ALLEGRO_BITMAP* alien[3];
    ALLEGRO_BITMAP* alien_shot;

    ALLEGRO_BITMAP* explosion[EXPLOSION_FRAMES];
    ALLEGRO_BITMAP* sparks[SPARKS_FRAMES];

    ALLEGRO_BITMAP* powerup[4];
} SPRITES;
SPRITES sprites;

ALLEGRO_BITMAP* sprite_grab(int x, int y, int w, int h)
{
    ALLEGRO_BITMAP* sprite = al_create_sub_bitmap(sprites._sheet, x, y, w, h);
    must_init(sprite, "sprite grab");
    return sprite;
}

void sprites_init()
{
    sprites._sheet = al_load_bitmap("spritesheet.png");
    must_init(sprites._sheet, "spritesheet");

    sprites.ship = sprite_grab(0, 0, SHIP_W, SHIP_H);

    sprites.ship_shot[0] = sprite_grab(13, 0, SHIP_SHOT_W, SHIP_SHOT_H);
    sprites.ship_shot[1] = sprite_grab(16, 0, SHIP_SHOT_W, SHIP_SHOT_H);

    sprites.life = sprite_grab(0, 14, LIFE_W, LIFE_H);

    sprites.alien[0] = sprite_grab(19, 0, ALIEN_BUG_W, ALIEN_BUG_H);
    sprites.alien[1] = sprite_grab(19, 10, ALIEN_ARROW_W, ALIEN_ARROW_H);
    sprites.alien[2] = sprite_grab(0, 21, ALIEN_THICCBOI_W, ALIEN_THICCBOI_H);

    sprites.alien_shot = sprite_grab(13, 10, ALIEN_SHOT_W, ALIEN_SHOT_H);

    sprites.explosion[0] = sprite_grab(33, 10, 9, 9);
    sprites.explosion[1] = sprite_grab(43, 9, 11, 11);
    sprites.explosion[2] = sprite_grab(46, 21, 17, 18);
    sprites.explosion[3] = sprite_grab(46, 40, 17, 17);

    sprites.sparks[0] = sprite_grab(34, 0, 10, 8);
    sprites.sparks[1] = sprite_grab(45, 0, 7, 8);
    sprites.sparks[2] = sprite_grab(54, 0, 9, 8);

    sprites.powerup[0] = sprite_grab(0, 49, 9, 12);
    sprites.powerup[1] = sprite_grab(10, 49, 9, 12);
    sprites.powerup[2] = sprite_grab(20, 49, 9, 12);
    sprites.powerup[3] = sprite_grab(30, 49, 9, 12);
}

void sprites_deinit()
{
    al_destroy_bitmap(sprites.ship);

    al_destroy_bitmap(sprites.ship_shot[0]);
    al_destroy_bitmap(sprites.ship_shot[1]);

    al_destroy_bitmap(sprites.life);

    al_destroy_bitmap(sprites.alien[0]);
    al_destroy_bitmap(sprites.alien[1]);
    al_destroy_bitmap(sprites.alien[2]);

    al_destroy_bitmap(sprites.alien_shot);

    al_destroy_bitmap(sprites.explosion[0]);
    al_destroy_bitmap(sprites.explosion[1]);
    al_destroy_bitmap(sprites.explosion[2]);
    al_destroy_bitmap(sprites.explosion[3]);

    al_destroy_bitmap(sprites.sparks[0]);
    al_destroy_bitmap(sprites.sparks[1]);
    al_destroy_bitmap(sprites.sparks[2]);

    al_destroy_bitmap(sprites.powerup[0]);
    al_destroy_bitmap(sprites.powerup[1]);
    al_destroy_bitmap(sprites.powerup[2]);
    al_destroy_bitmap(sprites.powerup[3]);

    al_destroy_bitmap(sprites._sheet);
}


// --- audio ---

ALLEGRO_SAMPLE* sample_shot;
ALLEGRO_SAMPLE* sample_explode[2];

void audio_init()
{
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(128);

    sample_shot = al_load_sample("shot.flac");
    must_init(sample_shot, "shot sample");

    sample_explode[0] = al_load_sample("explode1.flac");
    must_init(sample_explode[0], "explode[0] sample");
    sample_explode[1] = al_load_sample("explode2.flac");
    must_init(sample_explode[1], "explode[1] sample");
}

void audio_deinit()
{
    al_destroy_sample(sample_shot);
    al_destroy_sample(sample_explode[0]);
    al_destroy_sample(sample_explode[1]);
}


// --- fx ---

typedef struct FX
{
    int x, y;
    int frame;
    bool spark;
    bool used;
} FX;

#define FX_N 128
FX fx[FX_N];

void fx_init()
{
    for (int i = 0; i < FX_N; i++)
        fx[i].used = false;
}

void fx_add(bool spark, int x, int y)
{
    if (!spark)
        al_play_sample(sample_explode[between(0, 2)], 0.75, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);

    for (int i = 0; i < FX_N; i++)
    {
        if (fx[i].used)
            continue;

        fx[i].x = x;
        fx[i].y = y;
        fx[i].frame = 0;
        fx[i].spark = spark;
        fx[i].used = true;
        return;
    }
}

void fx_update()
{
    for (int i = 0; i < FX_N; i++)
    {
        if (!fx[i].used)
            continue;

        fx[i].frame++;

        if ((!fx[i].spark && (fx[i].frame == (EXPLOSION_FRAMES * 2)))
            || (fx[i].spark && (fx[i].frame == (SPARKS_FRAMES * 2)))
            )
            fx[i].used = false;
    }
}

void fx_draw()
{
    for (int i = 0; i < FX_N; i++)
    {
        if (!fx[i].used)
            continue;

        int frame_display = fx[i].frame / 2;
        ALLEGRO_BITMAP* bmp =
            fx[i].spark
            ? sprites.sparks[frame_display]
            : sprites.explosion[frame_display]
            ;

        int x = fx[i].x - (al_get_bitmap_width(bmp) / 2);
        int y = fx[i].y - (al_get_bitmap_height(bmp) / 2);
        al_draw_bitmap(bmp, x, y, 0);
    }
}


// --- shots ---

typedef struct SHOT
{
    int x, y, dx, dy;
    int frame;
    bool ship;
    bool used;
} SHOT;

#define SHOTS_N 128
SHOT shots[SHOTS_N];

void shots_init()
{
    for (int i = 0; i < SHOTS_N; i++)
        shots[i].used = false;
}

bool shots_add(bool ship, bool straight, int x, int y)
{
    al_play_sample(
        sample_shot,
        0.3,
        0,
        ship ? 1.0 : between_f(1.5, 1.6),
        ALLEGRO_PLAYMODE_ONCE,
        NULL
    );

    for (int i = 0; i < SHOTS_N; i++)
    {
        if (shots[i].used)
            continue;

        shots[i].ship = ship;

        if (ship)
        {
            shots[i].x = x - (SHIP_SHOT_W / 2);
            shots[i].y = y;
        }
        else // alien
        {
            shots[i].x = x - (ALIEN_SHOT_W / 2);
            shots[i].y = y - (ALIEN_SHOT_H / 2);

            if (straight)
            {
                shots[i].dx = 0;
                shots[i].dy = 2;
            }
            else
            {

                shots[i].dx = between(-2, 2);
                shots[i].dy = between(-2, 2);
            }

            // if the shot has no speed, don't bother
            if (!shots[i].dx && !shots[i].dy)
                return true;

            shots[i].frame = 0;
        }

        shots[i].frame = 0;
        shots[i].used = true;

        return true;
    }
    return false;
}

void shots_update()
{
    for (int i = 0; i < SHOTS_N; i++)
    {
        if (!shots[i].used)
            continue;

        if (shots[i].ship)
        {
            shots[i].y -= 5;

            if (shots[i].y < -SHIP_SHOT_H)
            {
                shots[i].used = false;
                continue;
            }
        }
        else // alien
        {
            shots[i].x += shots[i].dx;
            shots[i].y += shots[i].dy;

            if ((shots[i].x < -ALIEN_SHOT_W)
                || (shots[i].x > BUFFER_W)
                || (shots[i].y < -ALIEN_SHOT_H)
                || (shots[i].y > BUFFER_H)
                ) {
                shots[i].used = false;
                continue;
            }
        }

        shots[i].frame++;
    }
}

bool shots_collide(bool ship, int x, int y, int w, int h)
{
    for (int i = 0; i < SHOTS_N; i++)
    {
        if (!shots[i].used)
            continue;

        // don't collide with one's own shots
        if (shots[i].ship == ship)
            continue;

        int sw, sh;
        if (ship)
        {
            sw = ALIEN_SHOT_W;
            sh = ALIEN_SHOT_H;
        }
        else
        {
            sw = SHIP_SHOT_W;
            sh = SHIP_SHOT_H;
        }

        if (collide(x, y, x + w, y + h, shots[i].x, shots[i].y, shots[i].x + sw, shots[i].y + sh))
        {
            fx_add(true, shots[i].x + (sw / 2), shots[i].y + (sh / 2));
            shots[i].used = false;
            return true;
        }
    }

    return false;
}

void shots_draw()
{
    for (int i = 0; i < SHOTS_N; i++)
    {
        if (!shots[i].used)
            continue;

        int frame_display = (shots[i].frame / 2) % 2;

        if (shots[i].ship)
            al_draw_bitmap(sprites.ship_shot[frame_display], shots[i].x, shots[i].y, 0);
        else // alien
        {
            ALLEGRO_COLOR tint =
                frame_display
                ? al_map_rgb_f(1, 1, 1)
                : al_map_rgb_f(0.5, 0.5, 0.5)
                ;
            al_draw_tinted_bitmap(sprites.alien_shot, tint, shots[i].x, shots[i].y, 0);
        }
    }
}


// --- ship ---

#define SHIP_SPEED 3
#define SHIP_MAX_X (BUFFER_W - SHIP_W)
#define SHIP_MAX_Y (BUFFER_H - SHIP_H)

typedef struct SHIP
{
    int x, y;
    int shot_timer;
    int lives;
    int respawn_timer;
    int invincible_timer;
} SHIP;
SHIP ship;

void ship_init()
{
    ship.x = (BUFFER_W / 2) - (SHIP_W / 2);
    ship.y = (BUFFER_H / 2) - (SHIP_H / 2);
    ship.shot_timer = 0;
    ship.lives = 3;
    ship.respawn_timer = 0;
    ship.invincible_timer = 120;
}

void ship_update()
{
    if (ship.lives < 0)
        return;

    if (ship.respawn_timer)
    {
        ship.respawn_timer--;
        return;
    }

    if (key[ALLEGRO_KEY_LEFT])
        ship.x -= SHIP_SPEED;
    if (key[ALLEGRO_KEY_RIGHT])
        ship.x += SHIP_SPEED;
    if (key[ALLEGRO_KEY_UP])
        ship.y -= SHIP_SPEED;
    if (key[ALLEGRO_KEY_DOWN])
        ship.y += SHIP_SPEED;

    if (ship.x < 0)
        ship.x = 0;
    if (ship.y < 0)
        ship.y = 0;

    if (ship.x > SHIP_MAX_X)
        ship.x = SHIP_MAX_X;
    if (ship.y > SHIP_MAX_Y)
        ship.y = SHIP_MAX_Y;

    if (ship.invincible_timer)
        ship.invincible_timer--;
    else
    {
        if (shots_collide(true, ship.x, ship.y, SHIP_W, SHIP_H))
        {
            int x = ship.x + (SHIP_W / 2);
            int y = ship.y + (SHIP_H / 2);
            fx_add(false, x, y);
            fx_add(false, x + 4, y + 2);
            fx_add(false, x - 2, y - 4);
            fx_add(false, x + 1, y - 5);

            ship.lives--;
            ship.respawn_timer = 90;
            ship.invincible_timer = 180;
        }
    }

    if (ship.shot_timer)
        ship.shot_timer--;
    else if (key[ALLEGRO_KEY_X])
    {
        int x = ship.x + (SHIP_W / 2);
        if (shots_add(true, false, x, ship.y))
            ship.shot_timer = 5;
    }
}

void ship_draw()
{
    if (ship.lives < 0)
        return;
    if (ship.respawn_timer)
        return;
    if (((ship.invincible_timer / 2) % 3) == 1)
        return;

    al_draw_bitmap(sprites.ship, ship.x, ship.y, 0);
}


// --- aliens ---

typedef enum ALIEN_TYPE
{
    ALIEN_TYPE_BUG = 0,
    ALIEN_TYPE_ARROW,
    ALIEN_TYPE_THICCBOI,
    ALIEN_TYPE_N
} ALIEN_TYPE;

typedef struct ALIEN
{
    int x, y;
    ALIEN_TYPE type;
    int shot_timer;
    int blink;
    int life;
    bool used;
} ALIEN;

#define ALIENS_N 16
ALIEN aliens[ALIENS_N];

void aliens_init()
{
    for (int i = 0; i < ALIENS_N; i++)
        aliens[i].used = false;
}

void aliens_update()
{
    int new_quota =
        (frames % 120)
        ? 0
        : between(2, 4)
        ;
    int new_x = between(10, BUFFER_W - 50);

    for (int i = 0; i < ALIENS_N; i++)
    {
        if (!aliens[i].used)
        {
            // if this alien is unused, should it spawn?
            if (new_quota > 0)
            {
                new_x += between(40, 80);
                if (new_x > (BUFFER_W - 60))
                    new_x -= (BUFFER_W - 60);

                aliens[i].x = new_x;

                aliens[i].y = between(-40, -30);
                aliens[i].type = between(0, ALIEN_TYPE_N);
                aliens[i].shot_timer = between(1, 99);
                aliens[i].blink = 0;
                aliens[i].used = true;

                switch (aliens[i].type)
                {
                case ALIEN_TYPE_BUG:
                    aliens[i].life = 4;
                    break;
                case ALIEN_TYPE_ARROW:
                    aliens[i].life = 2;
                    break;
                case ALIEN_TYPE_THICCBOI:
                    aliens[i].life = 12;
                    break;
                }

                new_quota--;
            }
            continue;
        }

        switch (aliens[i].type)
        {
        case ALIEN_TYPE_BUG:
            if (frames % 2)
                aliens[i].y++;
            break;

        case ALIEN_TYPE_ARROW:
            aliens[i].y++;
            break;

        case ALIEN_TYPE_THICCBOI:
            if (!(frames % 4))
                aliens[i].y++;
            break;
        }

        if (aliens[i].y >= BUFFER_H)
        {
            aliens[i].used = false;
            continue;
        }

        if (aliens[i].blink)
            aliens[i].blink--;

        if (shots_collide(false, aliens[i].x, aliens[i].y, ALIEN_W[aliens[i].type], ALIEN_H[aliens[i].type]))
        {
            aliens[i].life--;
            aliens[i].blink = 4;
        }

        int cx = aliens[i].x + (ALIEN_W[aliens[i].type] / 2);
        int cy = aliens[i].y + (ALIEN_H[aliens[i].type] / 2);

        if (aliens[i].life <= 0)
        {
            fx_add(false, cx, cy);

            switch (aliens[i].type)
            {
            case ALIEN_TYPE_BUG:
                score += 200;
                break;

            case ALIEN_TYPE_ARROW:
                score += 150;
                break;

            case ALIEN_TYPE_THICCBOI:
                score += 800;
                fx_add(false, cx - 10, cy - 4);
                fx_add(false, cx + 4, cy + 10);
                fx_add(false, cx + 8, cy + 8);
                break;
            }

            aliens[i].used = false;
            continue;
        }

        aliens[i].shot_timer--;
        if (aliens[i].shot_timer == 0)
        {
            switch (aliens[i].type)
            {
            case ALIEN_TYPE_BUG:
                shots_add(false, false, cx, cy);
                aliens[i].shot_timer = 150;
                break;
            case ALIEN_TYPE_ARROW:
                shots_add(false, true, cx, aliens[i].y);
                aliens[i].shot_timer = 80;
                break;
            case ALIEN_TYPE_THICCBOI:
                shots_add(false, true, cx - 5, cy);
                shots_add(false, true, cx + 5, cy);
                shots_add(false, true, cx - 5, cy + 8);
                shots_add(false, true, cx + 5, cy + 8);
                aliens[i].shot_timer = 200;
                break;
            }
        }
    }
}

void aliens_draw()
{
    for (int i = 0; i < ALIENS_N; i++)
    {
        if (!aliens[i].used)
            continue;
        if (aliens[i].blink > 2)
            continue;

        al_draw_bitmap(sprites.alien[aliens[i].type], aliens[i].x, aliens[i].y, 0);
    }
}


// --- stars ---

typedef struct STAR
{
    float y;
    float speed;
} STAR;

#define STARS_N ((BUFFER_W / 2) - 1)
STAR stars[STARS_N];

void stars_init()
{
    for (int i = 0; i < STARS_N; i++)
    {
        stars[i].y = between_f(0, BUFFER_H);
        stars[i].speed = between_f(0.1, 1);
    }
}

void stars_update()
{
    for (int i = 0; i < STARS_N; i++)
    {
        stars[i].y += stars[i].speed;
        if (stars[i].y >= BUFFER_H)
        {
            stars[i].y = 0;
            stars[i].speed = between_f(0.1, 1);
        }
    }
}

void stars_draw()
{
    float star_x = 1.5;
    for (int i = 0; i < STARS_N; i++)
    {
        float l = stars[i].speed * 0.8;
        al_draw_pixel(star_x, stars[i].y, al_map_rgb_f(l, l, l));
        star_x += 2;
    }
}


// --- hud ---

ALLEGRO_FONT* font;
long score_display;

void hud_init()
{
    font = al_create_builtin_font();
    must_init(font, "font");

    score_display = 0;
}

void hud_deinit()
{
    al_destroy_font(font);
}

void hud_update()
{
    if (frames % 2)
        return;

    for (long i = 5; i > 0; i--)
    {
        long diff = 1 << i;
        if (score_display <= (score - diff))
            score_display += diff;
    }
}

void hud_draw()
{
    al_draw_textf(
        font,
        al_map_rgb_f(1, 1, 1),
        1, 1,
        0,
        "%06ld",
        score_display
    );

    int spacing = LIFE_W + 1;
    for (int i = 0; i < ship.lives; i++)
        al_draw_bitmap(sprites.life, 1 + (i * spacing), 10, 0);

    if (ship.lives < 0)
        al_draw_text(
            font,
            al_map_rgb_f(1, 1, 1),
            BUFFER_W / 2, BUFFER_H / 2,
            ALLEGRO_ALIGN_CENTER,
            "G A M E  O V E R"
        );
}


// --- main ---

int main()
{
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    disp_init();

    audio_init();

    must_init(al_init_image_addon(), "image");
    sprites_init();

    hud_init();

    must_init(al_init_primitives_addon(), "primitives");

    must_init(al_install_audio(), "audio");
    must_init(al_init_acodec_addon(), "audio codecs");
    must_init(al_reserve_samples(16), "reserve samples");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    keyboard_init();
    fx_init();
    shots_init();
    ship_init();
    aliens_init();
    stars_init();

    frames = 0;
    score = 0;

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;

    al_start_timer(timer);

    // for checking loop period
    double acc_loop_time = 0.0;
    int acc_loop_count = 0;
    clock_t print_prev = clock();

    while (1)
    {
        clock_t loop_start = clock();
        al_wait_for_event(queue, &event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
            fx_update();
            shots_update();
            stars_update();
            ship_update();
            aliens_update();
            hud_update();

            if (key[ALLEGRO_KEY_ESCAPE])
                done = true;

            redraw = true;
            frames++;
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            break;
        }

        if (done)
            break;

        keyboard_update(&event);

        if (redraw && al_is_event_queue_empty(queue))
        {
            disp_pre_draw();
            al_clear_to_color(al_map_rgb(0, 0, 0));

            stars_draw();
            aliens_draw();
            shots_draw();
            fx_draw();
            ship_draw();

            hud_draw();

            disp_post_draw();
            redraw = false;
        }

        clock_t loop_end = clock();
        double loop_duration = (double)(loop_end - loop_start) / CLOCKS_PER_SEC * 1000.0;

        acc_loop_time += loop_duration;
        acc_loop_count++;

        // print per 1sec
        clock_t print_now = clock();
        double elapsed = (double)(print_now - print_prev) / CLOCKS_PER_SEC;
        if (elapsed >= 1.0) {
            double avg_loop = acc_loop_time / acc_loop_count;
            printf("loop: %.3f ms (%d fps)\n", avg_loop, acc_loop_count);
            acc_loop_time = 0.0;
            acc_loop_count = 0;
            print_prev = print_now;
        }
    }

    sprites_deinit();
    hud_deinit();
    audio_deinit();
    disp_deinit();
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}

#endif


#if 0

/* game.c
 *
 * TankBoy - single-file Allegro5 game prototype
 * Implements: tank centered, moving background (parallax), tank movement & jump (parabola),
 * machinegun (straight) and cannon (parabolic projectile), 4 enemy types, stages, HUD,
 * game over + name entry. All bitmaps are generated at runtime (no external assets).
 *
 * Build:
 * gcc game.c -o game $(pkg-config allegro-5 allegro_font-5 allegro_primitives-5 --libs --cflags) -lm
 * ./game
 */

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* --- general --- */
long frames;
long score;

void must_init(bool test, const char* description)
{
    if (test) return;
    fprintf(stderr, "couldn't initialize %s\n", description);
    exit(1);
}

int between(int lo, int hi) { return lo + (rand() % (hi - lo)); }
float between_f(float lo, float hi) { return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo); }

bool collide(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2)
{
    if (ax1 > bx2) return false;
    if (ax2 < bx1) return false;
    if (ay1 > by2) return false;
    if (ay2 < by1) return false;
    return true;
}

/* --- display --- */
#define BUFFER_W 640
#define BUFFER_H 360

#define DISP_SCALE 1
#define DISP_W (BUFFER_W * DISP_SCALE)
#define DISP_H (BUFFER_H * DISP_SCALE)

ALLEGRO_DISPLAY* disp;
ALLEGRO_BITMAP* buffer;

void disp_init()
{
    disp = al_create_display(DISP_W, DISP_H);
    must_init(disp, "display");
    buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
    must_init(buffer, "bitmap buffer");
}

void disp_deinit()
{
    if (buffer) al_destroy_bitmap(buffer);
    if (disp) al_destroy_display(disp);
}

void disp_pre_draw() { al_set_target_bitmap(buffer); }
void disp_post_draw()
{
    al_set_target_backbuffer(disp);
    al_draw_scaled_bitmap(buffer, 0, 0, BUFFER_W, BUFFER_H, 0, 0, DISP_W, DISP_H, 0);
    al_flip_display();
}

/* --- keyboard --- */
#define KEY_SEEN 1
#define KEY_DOWN 2
unsigned char key[ALLEGRO_KEY_MAX];

void keyboard_init() { memset(key, 0, sizeof(key)); }
void keyboard_update(ALLEGRO_EVENT* event)
{
    switch (event->type)
    {
    case ALLEGRO_EVENT_TIMER:
        for (int i = 0; i < ALLEGRO_KEY_MAX; i++) key[i] &= ~KEY_SEEN;
        break;
    case ALLEGRO_EVENT_KEY_DOWN:
        key[event->keyboard.keycode] = KEY_SEEN | KEY_DOWN;
        break;
    case ALLEGRO_EVENT_KEY_UP:
        key[event->keyboard.keycode] &= ~KEY_DOWN;
        break;
    }
}

/* --- simple runtime-drawn spritesheet --- */
typedef struct SPRITES {
    ALLEGRO_BITMAP* tank;
    ALLEGRO_BITMAP* bullet_mg;
    ALLEGRO_BITMAP* bullet_cn;
    ALLEGRO_BITMAP* enemy_small;
    ALLEGRO_BITMAP* enemy_cannon;
    ALLEGRO_BITMAP* enemy_air;
    ALLEGRO_BITMAP* enemy_mover;
    ALLEGRO_BITMAP* life;
    ALLEGRO_BITMAP* portal;
} SPRITES;
SPRITES sprites;

void make_sprite_bitmaps()
{
    // tank (simple rectangle + turret)
    sprites.tank = al_create_bitmap(32, 20);
    al_set_target_bitmap(sprites.tank);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_filled_rounded_rectangle(2, 8, 30, 18, 3, 3, al_map_rgb(60, 120, 180));
    al_draw_filled_rectangle(12, 2, 20, 10, al_map_rgb(30, 80, 150));
    // mg bullet
    sprites.bullet_mg = al_create_bitmap(4, 4);
    al_set_target_bitmap(sprites.bullet_mg); al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_filled_rectangle(0, 0, 4, 4, al_map_rgb(255, 220, 0));
    // cannon projectile
    sprites.bullet_cn = al_create_bitmap(6, 6);
    al_set_target_bitmap(sprites.bullet_cn); al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_filled_circle(3, 3, 3, al_map_rgb(255, 140, 0));
    // enemy small (machinegun)
    sprites.enemy_small = al_create_bitmap(20, 12);
    al_set_target_bitmap(sprites.enemy_small); al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_filled_rectangle(2, 4, 18, 10, al_map_rgb(200, 60, 60));
    // enemy cannon
    sprites.enemy_cannon = al_create_bitmap(24, 14);
    al_set_target_bitmap(sprites.enemy_cannon); al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_filled_rectangle(2, 4, 22, 12, al_map_rgb(180, 80, 40));
    // enemy air
    sprites.enemy_air = al_create_bitmap(18, 10);
    al_set_target_bitmap(sprites.enemy_air); al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_filled_triangle(9, 0, 2, 10, 16, 10, al_map_rgb(120, 200, 120));
    // enemy mover
    sprites.enemy_mover = al_create_bitmap(22, 16);
    al_set_target_bitmap(sprites.enemy_mover); al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_filled_rectangle(2, 2, 20, 14, al_map_rgb(140, 60, 180));
    // life icon
    sprites.life = al_create_bitmap(10, 8);
    al_set_target_bitmap(sprites.life); al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_filled_rectangle(1, 1, 9, 7, al_map_rgb(255, 0, 0));
    // portal
    sprites.portal = al_create_bitmap(24, 40);
    al_set_target_bitmap(sprites.portal); al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_filled_rounded_rectangle(2, 2, 22, 38, 6, 6, al_map_rgb(80, 200, 255));
    // reset back buffer target will be set by disp_pre_draw when drawing
}

/* --- fx (simple explosions as circles) --- */
typedef struct FX { int x, y, frame; bool used; } FX;
#define FX_N 32
FX fx[FX_N];
void fx_init() { for (int i = 0; i < FX_N; i++) fx[i].used = false; }
void fx_add(int x, int y) { for (int i = 0; i < FX_N; i++) { if (!fx[i].used) { fx[i].used = true; fx[i].x = x; fx[i].y = y; fx[i].frame = 0; return; } } }
void fx_update() { for (int i = 0; i < FX_N; i++) if (fx[i].used) { fx[i].frame++; if (fx[i].frame > 18) fx[i].used = false; } }
void fx_draw() {
    for (int i = 0; i < FX_N; i++) if (fx[i].used) {
        float r = fx[i].frame * 1.4f;
        float a = (1.0f - fx[i].frame / 18.0f);
        al_draw_filled_circle(fx[i].x, fx[i].y, r, al_map_rgba_f(1.0, 0.6, 0.0, a));
    }
}

/* --- shots --- */
typedef struct SHOT {
    float x, y;
    float vx, vy;
    bool from_player;
    bool used;
    int life; // frames
    bool is_cannon;
} SHOT;
#define SHOTS_N 128
SHOT shots[SHOTS_N];

void shots_init() { for (int i = 0; i < SHOTS_N; i++) shots[i].used = false; }
void shots_add_player_mg(float x, float y) {
    for (int i = 0; i < SHOTS_N; i++) if (!shots[i].used) {
        shots[i].used = true; shots[i].from_player = true;
        shots[i].is_cannon = false;
        shots[i].x = x; shots[i].y = y; shots[i].vx = 0; shots[i].vy = -8; shots[i].life = 60; return;
    }
}
void shots_add_player_cannon(float x, float y, float angle, float power) {
    for (int i = 0; i < SHOTS_N; i++) if (!shots[i].used) {
        shots[i].used = true; shots[i].from_player = true;
        shots[i].is_cannon = true;
        shots[i].x = x; shots[i].y = y;
        shots[i].vx = cosf(angle) * power;
        shots[i].vy = -sinf(angle) * power;
        shots[i].life = 300;
        return;
    }
}
void shots_add_enemy(float x, float y, float vx, float vy) {
    for (int i = 0; i < SHOTS_N; i++) if (!shots[i].used) {
        shots[i].used = true; shots[i].from_player = false;
        shots[i].is_cannon = false;
        shots[i].x = x; shots[i].y = y; shots[i].vx = vx; shots[i].vy = vy; shots[i].life = 200; return;
    }
}

void shots_update()
{
    const float gravity = 0.25f;
    for (int i = 0; i < SHOTS_N; i++) {
        if (!shots[i].used) continue;
        if (shots[i].is_cannon) shots[i].vy += gravity;
        shots[i].x += shots[i].vx;
        shots[i].y += shots[i].vy;
        shots[i].life--;
        if (shots[i].life <= 0 || shots[i].x < -20 || shots[i].x > BUFFER_W + 20 || shots[i].y < -40 || shots[i].y > BUFFER_H + 40)
            shots[i].used = false;
    }
}

void shots_draw()
{
    for (int i = 0; i < SHOTS_N; i++) {
        if (!shots[i].used) continue;
        if (shots[i].from_player) {
            if (shots[i].is_cannon)
                al_draw_bitmap(sprites.bullet_cn, (int)shots[i].x - 3, (int)shots[i].y - 3, 0);
            else
                al_draw_bitmap(sprites.bullet_mg, (int)shots[i].x - 2, (int)shots[i].y - 2, 0);
        }
        else {
            al_draw_filled_circle(shots[i].x, shots[i].y, 3, al_map_rgb_f(1, 0.4, 0.4));
        }
    }
}

/* --- player (tank) --- */
typedef struct TANK {
    int x_screen, y_screen; // fixed screen pos (center-ish)
    float world_x; // background/world offset (camera)
    float vx, vy;
    bool on_ground;
    int hp;
    int shot_timer_mg;
    int shot_timer_cn;
    float cannon_angle; // radians
    int lives;
    int respawn_timer;
    int invincible;
} TANK;
TANK tank;

#define TANK_START_LIVES 3

void tank_init()
{
    tank.x_screen = BUFFER_W / 3; // tank is fixed on left-middle of screen
    tank.y_screen = BUFFER_H - 60;
    tank.world_x = 0;
    tank.vx = 0; tank.vy = 0;
    tank.on_ground = true;
    tank.hp = 100;
    tank.shot_timer_mg = 0;
    tank.shot_timer_cn = 0;
    tank.cannon_angle = M_PI / 4.0f; // 45deg
    tank.lives = TANK_START_LIVES;
    tank.respawn_timer = 0;
    tank.invincible = 0;
}

/* simple ground and hills: generate height by tiles, also used for collision */
#define MAP_W_TILES 300
float map_height[MAP_W_TILES]; // absolute y (top) of ground for each x tile
void map_init()
{
    // base ground line near BUFFER_H - 40, with some hills
    float base = BUFFER_H - 40;
    float x = 0;
    for (int i = 0; i < MAP_W_TILES; i++) {
        float hill = 0;
        if ((i / 20) % 3 == 1) hill = -30; // small hill
        if ((i / 50) % 5 == 2) hill = -60; // bigger hill
        map_height[i] = base + hill + between(-6, 6);
    }
}

/* world -> screen conversion */
float world_to_screen_x(float world_x, float camera_world_x) {
    return (world_x - camera_world_x) + tank.x_screen;
}

/* get ground y at world_x (approx by tile index) */
float ground_y_at(float world_x)
{
    int idx = (int)(world_x / 8.0f);
    if (idx < 0) idx = 0;
    if (idx >= MAP_W_TILES) idx = MAP_W_TILES - 1;
    return map_height[idx];
}

/* update tank physics (move & jump & firing) */
void tank_update()
{
    if (tank.lives < 0) return;
    if (tank.respawn_timer) { tank.respawn_timer--; return; }

    const float accel = 0.5f;
    const float maxspeed = 3.5f;
    const float friction = 0.85f;
    const float gravity = 0.6f;

    // left/right: a/d or arrows
    if (key[ALLEGRO_KEY_A] || key[ALLEGRO_KEY_LEFT]) tank.vx -= accel;
    if (key[ALLEGRO_KEY_D] || key[ALLEGRO_KEY_RIGHT]) tank.vx += accel;
    tank.vx *= friction;
    if (tank.vx > maxspeed) tank.vx = maxspeed;
    if (tank.vx < -maxspeed) tank.vx = -maxspeed;

    // jump: space
    if ((key[ALLEGRO_KEY_SPACE]) && tank.on_ground) {
        tank.vy = -9.5f;
        tank.on_ground = false;
    }

    // cannon angle adjust: W/S or mouse would be nicer; use W/S
    if (key[ALLEGRO_KEY_W]) { tank.cannon_angle += 0.03f; if (tank.cannon_angle > M_PI * 0.9f) tank.cannon_angle = M_PI * 0.9f; }
    if (key[ALLEGRO_KEY_S]) { tank.cannon_angle -= 0.03f; if (tank.cannon_angle < 0.1f) tank.cannon_angle = 0.1f; }

    // update position in world coordinates (tank.world_x increases when moving right)
    tank.world_x += tank.vx;

    // keep world_x within map
    if (tank.world_x < 0) tank.world_x = 0;
    if (tank.world_x > (MAP_W_TILES * 8 - 1)) tank.world_x = MAP_W_TILES * 8 - 1;

    // vertical physics
    tank.vy += gravity;
    float new_world_y = ground_y_at(tank.world_x); // ground level at current x
    float tank_world_y = new_world_y - 14; // tank base sits above ground
    // naive vertical: we model tank's vertical offset relative to ground (for jump)
    static float vertical_offset = 0.0f; // positive = above ground
    vertical_offset += tank.vy;
    if (vertical_offset > 0) {
        // when below ground (landing)
        if (vertical_offset > 1000) vertical_offset = 0;
    }
    // if below 0 (i.e., we've gone under ground), clamp
    float ground_screen_y = tank.y_screen + (ground_y_at(tank.world_x) - (BUFFER_H - 40));
    // Simplify: detect landing if a downward motion hits y = 0 offset
    if (tank.vy > 0) {
        // estimate absolute screen y of tank bottom and ground
        float tank_bottom_screen_y = tank.y_screen + vertical_offset + 12;
        float ground_line_screen_y = ground_screen_y + 40; // base shift
        // simpler approach: if we're below a small threshold, set on ground
    }

    // simpler (and robust) approach: keep tank on a vertical offset variable
    static float jump_offset = 0.0f;
    static float jump_v = 0.0f;
    if (!tank.on_ground) {
        jump_v += gravity;
        jump_offset += jump_v;
        if (jump_offset > 0) { // hitting ground
            jump_offset = 0;
            jump_v = 0;
            tank.on_ground = true;
            tank.vy = 0;
        }
    }
    else {
        // if just jumped we set jump_v
        if ((key[ALLEGRO_KEY_SPACE]) && tank.on_ground) {
            tank.on_ground = false;
            jump_v = -9.5f;
            jump_offset += jump_v;
        }
    }
    // store these back (approx): we'll render tank at y_screen + jump_offset
    (void)jump_offset;

    // firing
    if (tank.shot_timer_mg) tank.shot_timer_mg--;
    if (tank.shot_timer_cn) tank.shot_timer_cn--;

    // Machinegun: X key
    if ((key[ALLEGRO_KEY_X]) && tank.shot_timer_mg == 0) {
        // fire a few bullets slightly spread
        float sx = world_to_screen_x(tank.world_x, tank.world_x);
        float sy = tank.y_screen + -10;
        shots_add_player_mg(sx + 16, sy);
        tank.shot_timer_mg = 6; // rate of fire
    }

    // Cannon: C key (stronger, parabolic)
    if ((key[ALLEGRO_KEY_C]) && tank.shot_timer_cn == 0) {
        float sx = world_to_screen_x(tank.world_x, tank.world_x);
        float sy = tank.y_screen - 6;
        float angle = tank.cannon_angle; // radians
        float power = 7.2f;
        // convert to world coords: fire from tank.world_x + offset
        float fx_world_x = tank.world_x + 16;
        // spawn projectile in screen coords but with world-relative velocities
        shots_add_player_cannon(sx + 16, sy, angle, power);
        tank.shot_timer_cn = 30;
    }

    // simple invincibility tick
    if (tank.invincible) tank.invincible--;

    // camera following: here we set camera = tank.world_x - tank.x_screen so background moves
    // already handled later when drawing.
}

/* --- enemies --- */
typedef enum ENEMY_TYPE { E_SMALL = 0, E_CANNON, E_AIR, E_MOVER, E_N } ENEMY_TYPE;
typedef struct ENEMY {
    float world_x, y; // world_x is horizontal position in world coords
    ENEMY_TYPE type;
    int life;
    bool used;
    int dir; // for movers
    int shoot_cooldown;
    int spawn_stage;
} ENEMY;
#define ENEMIES_N 48
ENEMY enemies[ENEMIES_N];

void enemies_init() { for (int i = 0; i < ENEMIES_N; i++) enemies[i].used = false; }

void spawn_enemy(float world_x, ENEMY_TYPE type, int stage)
{
    for (int i = 0; i < ENEMIES_N; i++) {
        if (enemies[i].used) continue;
        enemies[i].used = true;
        enemies[i].world_x = world_x;
        enemies[i].type = type;
        enemies[i].spawn_stage = stage;
        enemies[i].y = ground_y_at(world_x) - 12;
        enemies[i].dir = (rand() % 2) ? 1 : -1;
        enemies[i].shoot_cooldown = between(30, 150);
        switch (type) {
        case E_SMALL: enemies[i].life = 4; break;
        case E_CANNON: enemies[i].life = 6; break;
        case E_AIR: enemies[i].life = 3; enemies[i].y = between(40, 120); break;
        case E_MOVER: enemies[i].life = 8; break;
        default: enemies[i].life = 5; break;
        }
        return;
    }
}

/* simple spawn logic depending on frames and stage */
int current_stage = 1;
void enemies_update()
{
    // spawn based on frames and stage
    if ((frames % (120 - current_stage * 8)) == 0) {
        float spawn_x = tank.world_x + BUFFER_W + between(40, 120);
        // pick types depending on stage
        if (current_stage == 1) spawn_enemy(spawn_x, E_SMALL, current_stage);
        else if (current_stage == 2) spawn_enemy(spawn_x, (rand() % 2) ? E_SMALL : E_CANNON, current_stage);
        else if (current_stage == 3) {
            spawn_enemy(spawn_x, (rand() % 3 == 0) ? E_AIR : ((rand() % 2) ? E_SMALL : E_CANNON), current_stage);
        }
        else {
            int r = rand() % 4;
            if (r == 0) spawn_enemy(spawn_x, E_MOVER, current_stage);
            else if (r == 1) spawn_enemy(spawn_x, E_AIR, current_stage);
            else spawn_enemy(spawn_x, (rand() % 2) ? E_SMALL : E_CANNON, current_stage);
        }
    }

    // update existing enemies
    for (int i = 0; i < ENEMIES_N; i++) {
        if (!enemies[i].used) continue;
        // behaviour by type
        switch (enemies[i].type) {
        case E_SMALL:
            enemies[i].y += (frames % 2) ? 0.5f : 0.0f;
            break;
        case E_CANNON:
            // stays, occasionally shoots
            enemies[i].shoot_cooldown--;
            if (enemies[i].shoot_cooldown <= 0) {
                // fire bullet towards player
                float sx = world_to_screen_x(enemies[i].world_x, tank.world_x);
                float sy = enemies[i].y - 6;
                // bullet velocity roughly towards player
                float dirx = (tank.x_screen - (sx)) > 0 ? 1 : -1;
                shots_add_enemy(sx, sy, dirx * -2.0f, 2.5f);
                enemies[i].shoot_cooldown = between(80, 160);
            }
            break;
        case E_AIR:
            enemies[i].world_x -= 1.2f;
            break;
        case E_MOVER:
            enemies[i].y += enemies[i].dir * 0.9f;
            if (enemies[i].y < 60) enemies[i].dir = 1;
            if (enemies[i].y > BUFFER_H - 80) enemies[i].dir = -1;
            break;
        default: break;
        }

        // move slowly left relative to world (so they come toward player)
        enemies[i].world_x -= 0.6f + current_stage * 0.1f;

        // if off-screen left, free
        if (enemies[i].world_x < tank.world_x - 200) { enemies[i].used = false; continue; }

        // collision with player shots
        int ew = 20, eh = 14;
        float ex_screen = world_to_screen_x(enemies[i].world_x, tank.world_x);
        float ey_screen = enemies[i].y;
        // check shots
        for (int s = 0; s < SHOTS_N; s++) {
            if (!shots[s].used) continue;
            if (!shots[s].from_player) continue;
            int sw = shots[s].is_cannon ? 6 : 4;
            if (collide((int)ex_screen, (int)ey_screen, (int)ex_screen + ew, (int)ey_screen + eh, (int)shots[s].x, (int)shots[s].y, (int)shots[s].x + sw, (int)shots[s].y + sw)) {
                enemies[i].life -= (shots[s].is_cannon ? 5 : 3); // cannon bigger damage
                shots[s].used = false;
                fx_add((int)ex_screen + ew / 2, (int)ey_screen + eh / 2);
                if (enemies[i].life <= 0) {
                    // scoring by type
                    switch (enemies[i].type) {
                    case E_SMALL: score += 1; break;
                    case E_CANNON: score += 2; break;
                    case E_AIR: score += 3; break;
                    case E_MOVER: score += 4; break;
                    default: score += 1; break;
                    }
                    enemies[i].used = false;
                }
                break;
            }
        }

        // collision with enemy touching the player's hull (damage to tank)
        float tank_screen_x = tank.x_screen, tank_screen_y = tank.y_screen;
        if (!tank.invincible && enemies[i].used) {
            if (collide((int)ex_screen, (int)ey_screen, (int)ex_screen + ew, (int)ey_screen + eh,
                (int)tank_screen_x, (int)tank_screen_y, (int)tank_screen_x + 32, (int)tank_screen_y + 20)) {
                tank.hp -= 10;
                tank.invincible = 90;
                fx_add((int)tank_screen_x + 16, (int)tank_screen_y + 8);
                enemies[i].used = false;
                if (tank.hp <= 0) {
                    tank.lives--;
                    if (tank.lives >= 0) {
                        tank.hp = 100;
                        tank.respawn_timer = 120;
                        tank.invincible = 180;
                    }
                    else {
                        // game over handled higher level
                    }
                }
            }
        }
    }
}

void enemies_draw()
{
    for (int i = 0; i < ENEMIES_N; i++) {
        if (!enemies[i].used) continue;
        float sx = world_to_screen_x(enemies[i].world_x, tank.world_x);
        float sy = enemies[i].y;
        switch (enemies[i].type) {
        case E_SMALL:
            al_draw_bitmap(sprites.enemy_small, (int)sx, (int)sy, 0); break;
        case E_CANNON:
            al_draw_bitmap(sprites.enemy_cannon, (int)sx, (int)sy, 0); break;
        case E_AIR:
            al_draw_bitmap(sprites.enemy_air, (int)sx, (int)sy, 0); break;
        case E_MOVER:
            al_draw_bitmap(sprites.enemy_mover, (int)sx, (int)sy, 0); break;
        default:
            al_draw_filled_rectangle((int)sx, (int)sy, (int)sx + 18, (int)sy + 12, al_map_rgb(200, 100, 100));
        }
    }
}

/* --- stars / parallax background --- */
typedef struct PARAX {
    float x, y;
    float speed;
    float layer; // 0..1
} PARAX;
#define PARAX_N 80
PARAX parallax[PARAX_N];
void parallax_init() {
    for (int i = 0; i < PARAX_N; i++) {
        parallax[i].x = rand() % (BUFFER_W * 3);
        parallax[i].y = rand() % BUFFER_H;
        parallax[i].speed = between_f(0.1f, 1.2f);
        parallax[i].layer = between_f(0.05f, 0.9f);
    }
}
void parallax_update() {
    for (int i = 0; i < PARAX_N; i++) {
        parallax[i].x -= parallax[i].speed;
        if (parallax[i].x < -BUFFER_W) parallax[i].x = BUFFER_W + (rand() % 200);
    }
}
void parallax_draw() {
    for (int i = 0; i < PARAX_N; i++) {
        float l = 0.1f + parallax[i].layer * 0.9f;
        al_draw_filled_circle(parallax[i].x, parallax[i].y, 1.0 + parallax[i].layer * 2.0, al_map_rgb_f(l, l, l));
    }
}

/* --- HUD / font --- */
ALLEGRO_FONT* font_builtin;
void hud_init() { font_builtin = al_create_builtin_font(); must_init(font_builtin, "font"); }
void hud_deinit() { if (font_builtin) al_destroy_font(font_builtin); }

void hud_draw()
{
    al_draw_textf(font_builtin, al_map_rgb_f(1, 1, 1), 8, 8, 0, "SCORE: %06ld", score);
    al_draw_textf(font_builtin, al_map_rgb_f(1, 1, 1), 8, 24, 0, "STAGE: %d", current_stage);
    al_draw_textf(font_builtin, al_map_rgb_f(1, 1, 1), 8, 40, 0, "HP: %d", tank.hp);
    // lives as small icons
    for (int i = 0; i < (tank.lives > 0 ? tank.lives : 0); i++) {
        al_draw_bitmap(sprites.life, BUFFER_W - 12 - i * 12, 8, 0);
    }
    // instruction
    al_draw_text(font_builtin, al_map_rgb_f(1, 1, 1), BUFFER_W / 2, 8, ALLEGRO_ALIGN_CENTER, "A/D Move  SPACE Jump  X MG  C Cannon  W/S Aim  ENTER Name(after game)");
}

/* --- portal / stage progression --- */
float portal_world_x = 2000; // example portal position per stage
void portal_init_for_stage(int stage) {
    // set portal further for higher stage
    portal_world_x = stage * 1200 + between(400, 800);
}
void portal_draw() {
    float sx = world_to_screen_x(portal_world_x, tank.world_x);
    if (sx > -40 && sx < BUFFER_W + 40)
        al_draw_bitmap(sprites.portal, (int)sx - 12, BUFFER_H - 40 - 20, 0);
}
void portal_update() {
    float sx = world_to_screen_x(portal_world_x, tank.world_x);
    if (sx > tank.x_screen - 20 && sx < tank.x_screen + 40 && tank.lives >= 0) {
        // simple portal trigger when near portal
        current_stage++;
        score += 500 * current_stage;
        // reset stage: move portal further and maybe reset enemies
        for (int i = 0; i < ENEMIES_N; i++) enemies[i].used = false;
        portal_init_for_stage(current_stage);
    }
}

/* --- name input & ranking (simple) --- */
char player_name[32] = "";
bool entering_name = false;
void start_name_entry() { entering_name = true; player_name[0] = 0; }
void draw_name_entry() {
    al_draw_filled_rounded_rectangle(BUFFER_W / 2 - 160, BUFFER_H / 2 - 36, BUFFER_W / 2 + 160, BUFFER_H / 2 + 36, 6, 6, al_map_rgba_f(0, 0, 0, 0.8));
    al_draw_text(font_builtin, al_map_rgb_f(1, 1, 1), BUFFER_W / 2, BUFFER_H / 2 - 16, ALLEGRO_ALIGN_CENTER, "GAME OVER! Enter name:");
    al_draw_text(font_builtin, al_map_rgb_f(1, 1, 0), BUFFER_W / 2, BUFFER_H / 2 + 6, ALLEGRO_ALIGN_CENTER, player_name);
}

/* --- initialization & main --- */
int main(int argc, char** argv)
{
    srand((unsigned int)time(NULL));
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");
    must_init(al_init_primitives_addon(), "primitives");

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    disp_init();
    make_sprite_bitmaps();

    hud_init();
    parallax_init();
    fx_init();
    shots_init();
    tank_init();
    map_init();
    enemies_init();
    portal_init_for_stage(1);

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    keyboard_init();

    frames = 0;
    score = 0;

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;

    al_start_timer(timer);

    while (1)
    {
        al_wait_for_event(queue, &event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
            if (!entering_name) {
                frames++;
                fx_update();
                shots_update();
                parallax_update();
                tank_update();
                enemies_update();
                portal_update();
                // check collision of enemy shots with tank
                for (int s = 0; s < SHOTS_N; s++) {
                    if (!shots[s].used) continue;
                    if (shots[s].from_player) continue;
                    // enemy bullet collision
                    if (!tank.invincible && collide((int)tank.x_screen, (int)tank.y_screen, (int)tank.x_screen + 32, (int)tank.y_screen + 20,
                        (int)shots[s].x, (int)shots[s].y, (int)shots[s].x + 4, (int)shots[s].y + 4)) {
                        shots[s].used = false;
                        tank.hp -= 8;
                        tank.invincible = 90;
                        fx_add((int)tank.x_screen + 16, (int)tank.y_screen + 8);
                        if (tank.hp <= 0) {
                            tank.lives--;
                            if (tank.lives >= 0) {
                                tank.hp = 100;
                                tank.respawn_timer = 120;
                                tank.invincible = 180;
                            }
                            else {
                                // game over: begin name entry
                                entering_name = true;
                            }
                        }
                    }
                }
            }
            redraw = true;
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            break;
        }

        if (done) break;

        keyboard_update(&event);

        // keyboard input for name entry (when game over)
        if (entering_name && event.type == ALLEGRO_EVENT_KEY_DOWN) {
            int k = event.keyboard.keycode;
            if (k == ALLEGRO_KEY_BACKSPACE) {
                size_t len = strlen(player_name);
                if (len > 0) player_name[len - 1] = 0;
            }
            else if (k == ALLEGRO_KEY_ENTER) {
                // finalize name and display (here simply print to stdout)
                printf("Player: %s  Score: %ld\n", player_name, score);
                entering_name = false;
                done = true; // close game after entry for demo
            }
            else {
                // accept letters and digits
                char ch = 0;
                if (k >= ALLEGRO_KEY_A && k <= ALLEGRO_KEY_Z) {
                    ch = 'a' + (k - ALLEGRO_KEY_A);
                }
                else if (k >= ALLEGRO_KEY_0 && k <= ALLEGRO_KEY_9) {
                    ch = '0' + (k - ALLEGRO_KEY_0);
                }
                else if (k == ALLEGRO_KEY_SPACE) ch = ' ';
                if (ch && strlen(player_name) < 20) {
                    size_t len = strlen(player_name);
                    player_name[len] = ch;
                    player_name[len + 1] = 0;
                }
            }
        }

        if (redraw && al_is_event_queue_empty(queue))
        {
            disp_pre_draw();
            al_clear_to_color(al_map_rgb(10, 10, 20));

            // draw distant background
            al_draw_filled_rectangle(0, BUFFER_H - 40, BUFFER_W, BUFFER_H, al_map_rgb(40, 30, 20));
            // parallax stars
            parallax_draw();

            // draw ground segments based on map around camera
            int camera_world_x = (int)tank.world_x;
            for (int tx = -40; tx < MAP_W_TILES * 8; tx += 8) {
                float world_x = tx;
                float sx = world_to_screen_x(world_x, tank.world_x);
                if (sx < -40 || sx > BUFFER_W + 40) continue;
                int idx = tx / 8;
                if (idx < 0 || idx >= MAP_W_TILES) continue;
                float gy = map_height[idx];
                float ground_top_screen_y = BUFFER_H - (BUFFER_H - gy);
                // draw a simple ground strip
                al_draw_filled_rectangle(sx, gy, sx + 8, BUFFER_H, al_map_rgb(30, 120, 40));
            }

            // draw portal
            portal_draw();

            // draw enemies
            enemies_draw();

            // draw shots
            shots_draw();

            // draw fx
            fx_draw();

            // draw tank centered at tank.x_screen with slight bob from jumping
            int draw_x = tank.x_screen;
            int draw_y = tank.y_screen;
            // invincible blinking
            if (tank.invincible && ((tank.invincible / 6) % 2 == 0)) {
                // skip drawing (blink)
            }
            else {
                al_draw_bitmap(sprites.tank, draw_x, draw_y, 0);
                // cannon barrel representation (rotate)
                float cx = draw_x + 16;
                float cy = draw_y + 6;
                float angle = tank.cannon_angle;
                float bx = cx + cosf(angle) * 18;
                float by = cy - sinf(angle) * 18;
                al_draw_line(cx, cy, bx, by, al_map_rgb_f(0.8, 0.8, 0.2), 4.0f);
            }

            // HUD
            hud_draw();

            // game over & name entry overlay
            if (entering_name) draw_name_entry();

            disp_post_draw();

            redraw = false;
        }
    }

    // Cleanup
    al_destroy_bitmap(sprites.tank);
    al_destroy_bitmap(sprites.bullet_mg);
    al_destroy_bitmap(sprites.bullet_cn);
    al_destroy_bitmap(sprites.enemy_small);
    al_destroy_bitmap(sprites.enemy_cannon);
    al_destroy_bitmap(sprites.enemy_air);
    al_destroy_bitmap(sprites.enemy_mover);
    al_destroy_bitmap(sprites.life);
    al_destroy_bitmap(sprites.portal);

    hud_deinit();
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    disp_deinit();

    return 0;
}
 
#endif




#if 0

#define _CRT_SECURE_NO_WARNINGS
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

// ---------------------------------
// 탱크의 물리적 상태를 저장하는 구조체 정의
// ---------------------------------
typedef struct {
    float world_x;     // 맵에서의 X 좌표(게임 맵 전체)
    float y_screen;    // 화면상의 Y 좌표(화면 상에서만 그려질 Y축)
    float vx;          // X 속도
    float vy;          // Y 속도 (중력/점프)
    bool on_ground;    // 땅에 닿아있는지 (점프 가능 여부 판단)
} Tank;

typedef struct {
    float camera_x; // 가로 스크롤 방식
    float camera_y; // 세로는 화면 고정 방식
} Camera;

// ---------------------------------
// 전역 변수
// ---------------------------------
Tank tank;
Camera camera;
float gravity = 0.5f; // 중력 가속도 설정
float friction = 0.9f; // 표면 마찰 계수(원래는 한 0.8)

// ---------------------------------
// 함수 원형
// ---------------------------------
void must_init(bool test, const char* description);
float ground_y_at(float x); 
void init_game();
void update_physics(ALLEGRO_KEYBOARD_STATE* ks);
void render();

// ---------------------------------
// 보조 함수
// ---------------------------------
void must_init(bool test, const char* description) {
    if (test) return;
    printf("couldn't initialize %s\n", description);
    exit(1);
}

// 단순 지형 높이 함수
float ground_y_at(float x) { //특정 x좌표에서의 지형 높이 계산
  //  return 400 + 40 * sinf(x * 0.01f); // 울퉁불퉁한 땅 -> 다양한 렌더링 추가 가능 (sin x 함수로 지형 표현
  // )
    if (x < 300) {
        return 400; // 평지 구간
    }
    else if (x < 500) {
        // 발판: 왼쪽 50픽셀은 낮고, 오른쪽 50픽셀은 높음
        if (((int)x % 100) < 50)
            return 350;
        else
            return 400;
    }
    else if (x < 700) {
        // 계단식 지형: 100픽셀마다 20씩 상승
        int step = ((int)x - 500) / 100;
        return 400 - step * 20;
    }
    else {
        // 언덕: 부드러운 사인 곡선
        return 360 + 40 * sinf((x - 700) * 0.05f);
    }
}


// ---------------------------------
// 초기화
// ---------------------------------
void init_game() {
    tank.world_x = 100;
    tank.y_screen = ground_y_at(100) - 20; // 탱크 바닥 땅 위에
    tank.vx = 0;
    tank.vy = 0;
    tank.on_ground = true;

    camera.camera_x = 0;
    camera.camera_y = 0;
}

// ---------------------------------
// 물리 업데이트
// ---------------------------------
void update_physics(ALLEGRO_KEYBOARD_STATE* ks) {
    static bool prev_space = false;

    // 좌우 이동
    if (al_key_down(ks, ALLEGRO_KEY_LEFT)) { // left 자리에 내가 원하는 기호 넣으면 그게 키가 됌
        tank.vx = -2; // -2는 왼쪽으로 라는것을 의미 (숫자 크기 = 속도)
    }
    else if (al_key_down(ks, ALLEGRO_KEY_RIGHT)) {
        tank.vx = 2; // +2는 오른쪽을 의미
    }
    else {
        tank.vx *= friction; // 관성 마찰 작용 0.8로서 일반적 1*0.8해서 관성 작용
    }

    // 점프 (스페이스 키 눌렀을 때)
    if (al_key_down(ks, ALLEGRO_KEY_SPACE) && !prev_space && tank.on_ground) {
        tank.vy = -9.5f;
        tank.on_ground = false;
    }
    prev_space = al_key_down(ks, ALLEGRO_KEY_SPACE);

    // 중력 적용
    tank.vy += gravity;
    tank.world_x += tank.vx;
    tank.y_screen += tank.vy;   // <- 여기 추가 (화면 좌표도 갱신)

    // 땅 충돌 체크
    float ground_y = ground_y_at(tank.world_x);
    if (tank.y_screen >= ground_y - 20) {
        tank.y_screen = ground_y - 20;
        tank.vy = 0;
        tank.on_ground = true;
    }

    // 카메라 탱크 따라가기
    camera.camera_x = tank.world_x - 400; // 화면 중앙 기준
    camera.camera_y = 0;
}

// ---------------------------------
// 렌더링 (맵 생성 sin 파형으로)
// ---------------------------------
void render() {
    al_clear_to_color(al_map_rgb(100, 149, 237)); // 배경색 파란색

    // 땅 그리기
    for (int x = -50; x < 1280; x++) {
        float world_x = x + camera.camera_x;
        float ground_y = ground_y_at(world_x);
        al_draw_filled_rectangle(x, ground_y, x + 1, 600, al_map_rgb(34, 139, 34));
    } // 땅 색상을 풀 색으로 설정 (R,G,B 배열)

    // 탱크 (빨간색 사각형)
    float draw_x = tank.world_x - camera.camera_x;
    float draw_y = tank.y_screen;
    al_draw_filled_rectangle(draw_x - 20, draw_y - 20, draw_x + 20, draw_y, al_map_rgb(200, 0, 0));
}

// ---------------------------------
// 메인
// ---------------------------------
int main() {
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");
    must_init(al_init_primitives_addon(), "primitives");

    ALLEGRO_DISPLAY* disp = al_create_display(1280, 720); //800-600 픽셀
    must_init(disp, "display");

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    init_game();
    al_start_timer(timer);

    bool done = false;
    ALLEGRO_KEYBOARD_STATE ks;

    while (!done) {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done = true;
        }
        if (event.type == ALLEGRO_EVENT_TIMER) {
            al_get_keyboard_state(&ks);
            update_physics(&ks);
            render();
            al_flip_display();
        }
    }

    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    return 0;
}
#endif


#if 0
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BUFFER_W 1280
#define BUFFER_H 720

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

/* --- 키보드 입력 --- */
typedef struct {
    bool left, right, jump;
    bool fire, change_weapon;
    bool rotate_cw, rotate_ccw;
    bool esc;
} Input;
Input input;

void keyboard_update(ALLEGRO_EVENT* event) {
    bool down = event->type == ALLEGRO_EVENT_KEY_DOWN;
    switch (event->keyboard.keycode) {
    case ALLEGRO_KEY_LEFT: input.left = down; break;
    case ALLEGRO_KEY_RIGHT: input.right = down; break;
    case ALLEGRO_KEY_SPACE: input.jump = down; break;
    case ALLEGRO_KEY_X: input.fire = down; break;
    case ALLEGRO_KEY_R: input.change_weapon = down; break;
    case ALLEGRO_KEY_Z: input.rotate_cw = down; break;
    case ALLEGRO_KEY_C: input.rotate_ccw = down; break;
    case ALLEGRO_KEY_ESCAPE: input.esc = down; break;
    }
}

/* --- 맵 / 지형 --- */
#define MAP_W 200
float map_height[MAP_W];

void map_init() {
    float base = BUFFER_H - 40;
    for (int i = 0; i < MAP_W; i++)
        map_height[i] = base + (rand() % 20 - 10); // 간단한 지형
}

float ground_y_at(int x_tile) {
    if (x_tile < 0) x_tile = 0;
    if (x_tile >= MAP_W) x_tile = MAP_W - 1;
    return map_height[x_tile];
}

/* --- 카메라 --- */
typedef struct {
    float x, y;
} Camera;
Camera camera;

/* --- 탱크 구조체 --- */
typedef struct {
    float x, y;
    float vx, vy;
    float cannon_angle; // 포신 각도
    bool on_ground;
    int weapon; // 0 = MG, 1 = Cannon
} Tank;
Tank tank;

void tank_init() {
    tank.x = 50;
    tank.y = ground_y_at((int)tank.x) - 20;
    tank.vx = tank.vy = 0;
    tank.on_ground = true;
    tank.cannon_angle = M_PI / 4;
    tank.weapon = 0;
}

/* --- 탄환 --- */
typedef struct {
    float x, y;
    float vx, vy;
    bool alive;
    int weapon;
} Bullet;

#define MAX_BULLETS 50
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
            float speed = (tank.weapon == 0) ? 8.0f : 6.0f;
            bullets[i].vx = cosf(tank.cannon_angle) * speed;
            bullets[i].vy = -sinf(tank.cannon_angle) * speed;
            break;
        }
    }
}

void bullets_update() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) continue;
        if (bullets[i].weapon == 1) bullets[i].vy += 0.3f; // 대포는 중력 적용
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;
        // 화면 밖이나 지형 충돌 시 삭제
        if (bullets[i].x < 0 || bullets[i].x >= MAP_W * 4 || bullets[i].y >= BUFFER_H) bullets[i].alive = false;
    }
}

/* --- 탱크 업데이트 --- */
void tank_update() {
    const float accel = 0.4f;
    const float maxspeed = 3.0f;
    const float friction = 0.85f;
    const float gravity = 0.5f;

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

    if (input.rotate_cw) { tank.cannon_angle += 0.05f; if (tank.cannon_angle > M_PI * 0.9f) tank.cannon_angle = M_PI * 0.9f; }
    if (input.rotate_ccw) { tank.cannon_angle -= 0.05f; if (tank.cannon_angle < 0.1f) tank.cannon_angle = 0.1f; }

    tank.vy += gravity;
    tank.y += tank.vy;

    float ground = ground_y_at((int)tank.x);
    if (tank.y > ground - 20) { tank.y = ground - 20; tank.vy = 0; tank.on_ground = true; }

    if (input.change_weapon) { tank.weapon = 1 - tank.weapon; input.change_weapon = false; }

    if (input.fire) { shoot_bullet(); input.fire = false; }

    camera.x = tank.x - BUFFER_W / 3;
    camera.y = tank.y - BUFFER_H / 2;

    bullets_update();
}

/* --- 그리기 --- */
void draw_tank() {
    float screen_x = tank.x - camera.x;
    float screen_y = tank.y - camera.y;

    al_draw_filled_rectangle(screen_x, screen_y, screen_x + 32, screen_y + 20, al_map_rgb(60, 120, 180));

    float cx = screen_x + 16;
    float cy = screen_y + 10;
    float bx = cx + cosf(tank.cannon_angle) * 18;
    float by = cy - sinf(tank.cannon_angle) * 18;
    al_draw_line(cx, cy, bx, by, al_map_rgb(200, 200, 0), 4);
}

void draw_map() {
    for (int i = 0; i < MAP_W; i++) {
        float sx = i * 4 - camera.x;
        float sy = map_height[i];
        al_draw_filled_rectangle(sx, sy, sx + 4, BUFFER_H, al_map_rgb(30, 150, 40));
    }
}

void draw_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) continue;
        float sx = bullets[i].x - camera.x;
        float sy = bullets[i].y - camera.y;
        al_draw_filled_circle(sx, sy, 4, al_map_rgb(255, 50, 50));
    }
}

int main() {
    srand((unsigned int)time(NULL));
    if (!al_init()) return 1;
    al_install_keyboard();
    al_init_primitives_addon();

    disp_init();

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
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
        case ALLEGRO_EVENT_KEY_UP: keyboard_update(&event); break;
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

/* --- 디스플레이 / 버퍼 --- */
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

/* --- 키보드 입력 --- */
typedef struct {
    bool left, right, jump;
    bool fire, change_weapon;
    bool rotate_cw, rotate_ccw;
    bool esc;
} Input;
Input input;

void keyboard_update(ALLEGRO_EVENT* event) {
    bool down = event->type == ALLEGRO_EVENT_KEY_DOWN;
    switch (event->keyboard.keycode) {
    case ALLEGRO_KEY_LEFT: input.left = down; break;
    case ALLEGRO_KEY_RIGHT: input.right = down; break;
    case ALLEGRO_KEY_SPACE: input.jump = down; break;
    case ALLEGRO_KEY_X: input.fire = down; break;
    case ALLEGRO_KEY_R: input.change_weapon = down; break;
    case ALLEGRO_KEY_Z: input.rotate_cw = down; break;
    case ALLEGRO_KEY_C: input.rotate_ccw = down; break;
    case ALLEGRO_KEY_ESCAPE: input.esc = down; break;
    }
}

/* --- 맵 / 지형 --- */
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

/* --- 카메라 --- */
typedef struct {
    float x, y;
} Camera;
Camera camera;

/* --- 탱크 --- */
typedef struct {
    float x, y;
    float vx, vy;
    float cannon_angle;
    bool on_ground;
    int weapon; // 0 = MG, 1 = Cannon
} Tank;
Tank tank;

void tank_init() {
    tank.x = 50;
    tank.y = ground_y_at((int)tank.x) - 20;
    tank.vx = tank.vy = 0;
    tank.on_ground = true;
    tank.cannon_angle = M_PI / 4;
    tank.weapon = 0;
}

/* --- 탄환 --- */
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
            float speed = (tank.weapon == 0) ? 8.0f * 3 : 6.0f * 3; // 3배
            bullets[i].vx = cosf(tank.cannon_angle) * speed;
            bullets[i].vy = -sinf(tank.cannon_angle) * speed;
            break;
        }
    }
}

void bullets_update() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) continue;
        if (bullets[i].weapon == 1) bullets[i].vy += 0.3f;
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;
        if (bullets[i].x < 0 || bullets[i].x >= MAP_W * 4 || bullets[i].y >= BUFFER_H)
            bullets[i].alive = false;
    }
}

/* --- 탱크 업데이트 --- */
void tank_update() {
    const float accel = 0.4f;
    const float maxspeed = 3.0f;
    const float friction = 0.85f;
    const float gravity = 0.5f;

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

    if (input.rotate_cw) { tank.cannon_angle += 0.05f; if (tank.cannon_angle > M_PI * 0.9f) tank.cannon_angle = M_PI * 0.9f; }
    if (input.rotate_ccw) { tank.cannon_angle -= 0.05f; if (tank.cannon_angle < 0.1f) tank.cannon_angle = 0.1f; }

    tank.vy += gravity;
    tank.y += tank.vy;

    float ground = ground_y_at((int)tank.x);
    if (tank.y > ground - 20) { tank.y = ground - 20; tank.vy = 0; tank.on_ground = true; }

    if (input.change_weapon) { tank.weapon = 1 - tank.weapon; input.change_weapon = false; }
    if (input.fire) { shoot_bullet(); input.fire = false; }

    bullets_update();

    // 카메라 X, Y 모두 탱크 중심 기준
    camera.x = tank.x - BUFFER_W / 3;
    camera.y = tank.y - BUFFER_H / 2;
}

/* --- 그리기 --- */
void draw_tank() {
    float screen_x = tank.x - camera.x;
    float screen_y = tank.y - camera.y;

    al_draw_filled_rectangle(screen_x, screen_y, screen_x + 32, screen_y + 20, al_map_rgb(60, 120, 180));

    float cx = screen_x + 16;
    float cy = screen_y + 10;
    float bx = cx + cosf(tank.cannon_angle) * 18;
    float by = cy - sinf(tank.cannon_angle) * 18;
    al_draw_line(cx, cy, bx, by, al_map_rgb(200, 200, 0), 4);
}

void draw_map() {
    for (int i = 0; i < MAP_W; i++) {
        float sx = i * 4 - camera.x;
        float sy = map_height[i];
        al_draw_filled_rectangle(sx, sy, sx + 4, BUFFER_H, al_map_rgb(30, 150, 40));
    }
}

void draw_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) continue;
        float sx = bullets[i].x - camera.x;
        float sy = bullets[i].y - camera.y;
        al_draw_filled_circle(sx, sy, 4, al_map_rgb(255, 50, 50));
    }
}

/* --- 메인 --- */
int main() {
    srand((unsigned int)time(NULL));
    if (!al_init()) return 1;
    al_install_keyboard();
    al_init_primitives_addon();

    disp_init();

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
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
        case ALLEGRO_EVENT_KEY_UP: keyboard_update(&event); break;
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

/* --- 디스플레이 / 버퍼 --- */
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

/* --- 키보드 입력 --- */
typedef struct {
    bool left, right, jump;
    bool fire, change_weapon;
    bool rotate_cw, rotate_ccw;
    bool esc;
} Input;
Input input;

void keyboard_update(ALLEGRO_EVENT* event) {
    bool down = event->type == ALLEGRO_EVENT_KEY_DOWN;
    switch (event->keyboard.keycode) {
    case ALLEGRO_KEY_LEFT: input.left = down; break;
    case ALLEGRO_KEY_RIGHT: input.right = down; break;
    case ALLEGRO_KEY_SPACE: input.jump = down; break;
    case ALLEGRO_KEY_X: input.fire = down; break;
    case ALLEGRO_KEY_R: input.change_weapon = down; break;
    case ALLEGRO_KEY_Z: input.rotate_cw = down; break;
    case ALLEGRO_KEY_C: input.rotate_ccw = down; break;
    case ALLEGRO_KEY_ESCAPE: input.esc = down; break;
    }
}

/* --- 맵 / 지형 --- */
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

/* --- 카메라 --- */
typedef struct { float x, y; } Camera;
Camera camera;

/* --- 탱크 --- */
typedef struct {
    float x, y;
    float vx, vy;
    float cannon_angle;
    bool on_ground;
    int weapon; // 0 = MG, 1 = Cannon
    bool charging; // 캐논 충전 상태
    float cannon_power;
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
}

/* --- 탄환 --- */
typedef struct {
    float x, y;
    float vx, vy;
    bool alive;
    int weapon;
} Bullet;
Bullet bullets[MAX_BULLETS];

void bullets_init(){
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].alive = false;
}

void shoot_bullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) {
            bullets[i].alive = true;
            bullets[i].x = tank.x + 16;
            bullets[i].y = tank.y + 10;
            bullets[i].weapon = tank.weapon;
            float speed = (tank.weapon == 0) ? 8.0f * 0.7f : tank.cannon_power * 0.7f;
            bullets[i].vx = cosf(tank.cannon_angle) * speed;
            bullets[i].vy = -sinf(tank.cannon_angle) * speed;
            break;
        }
    }
}

void bullets_update() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) continue;
        if (bullets[i].weapon == 1) bullets[i].vy += 0.3f; // 캐논 중력
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;
        if (bullets[i].x < 0 || bullets[i].x >= MAP_W * 4 || bullets[i].y >= BUFFER_H)
            bullets[i].alive = false;
    }
}

/* --- 탱크 업데이트 --- */
void tank_update() {
    const float accel = 0.4f;
    const float maxspeed = 3.0f;
    const float friction = 0.85f;
    const float gravity = 0.5f;

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

    if (input.rotate_cw) { tank.cannon_angle += 0.05f; if (tank.cannon_angle > M_PI * 0.9f) tank.cannon_angle = M_PI * 0.9f; }
    if (input.rotate_ccw) { tank.cannon_angle -= 0.05f; if (tank.cannon_angle < 0.1f) tank.cannon_angle = 0.1f; }

    tank.vy += gravity;
    tank.y += tank.vy;

    float ground = ground_y_at((int)tank.x);
    if (tank.y > ground - 20) { tank.y = ground - 20; tank.vy = 0; tank.on_ground = true; }

    if (input.change_weapon) { tank.weapon = 1 - tank.weapon; input.change_weapon = false; }

    // 캐논 충전 처리
    if (tank.weapon == 1) {
        if (input.fire) {
            tank.charging = true;
            tank.cannon_power += 0.2f;
            if (tank.cannon_power > 15.0f) tank.cannon_power = 15.0f;
        }
        else if (tank.charging) {
            shoot_bullet();
            tank.charging = false;
            tank.cannon_power = 0;
        }
    }
    else if (input.fire) { // 기관총
        shoot_bullet();
    }

    bullets_update();

    // 카메라 X, Y 모두 탱크 중심 기준
    camera.x = tank.x - BUFFER_W / 3;
    camera.y = tank.y - BUFFER_H / 2;
}

/* --- 그리기 --- */
void draw_tank() {
    float screen_x = tank.x - camera.x;
    float screen_y = tank.y - camera.y;

    al_draw_filled_rectangle(screen_x, screen_y, screen_x + 32, screen_y + 20, al_map_rgb(60, 120, 180));

    float cx = screen_x + 16;
    float cy = screen_y + 10;
    float bx = cx + cosf(tank.cannon_angle) * 18;
    float by = cy - sinf(tank.cannon_angle) * 18;
    al_draw_line(cx, cy, bx, by, al_map_rgb(200, 200, 0), 4);

    // 캐논 충전 시 포물선 궤적
    if (tank.charging) {
        float px = tank.x;
        float py = tank.y + 10;
        float vx = cosf(tank.cannon_angle) * tank.cannon_power;
        float vy = -sinf(tank.cannon_angle) * tank.cannon_power;
        for (int i = 0; i < 60; i++) {
            px += vx;
            py += vy;
            vy += 0.3f; // 중력
            float sx = px - camera.x;
            float sy = py - camera.y;
            if (sx < 0 || sx > BUFFER_W || sy > BUFFER_H) break;
            al_draw_filled_circle(sx, sy, 2, al_map_rgb(255, 255, 255));
        }

        // 게이지 표시
        float gauge_w = tank.cannon_power * 10;
        al_draw_filled_rectangle(screen_x, screen_y - 20, screen_x + gauge_w, screen_y - 10, al_map_rgb(255, 0, 0));
        al_draw_rectangle(screen_x, screen_y - 20, screen_x + 150, screen_y - 10, al_map_rgb(255, 255, 255), 2);
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

/* --- 메인 --- */
int main() {
    srand((unsigned int)time(NULL));
    if (!al_init()) return 1;
    al_install_keyboard();
    al_init_primitives_addon();

    disp_init();

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
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
        case ALLEGRO_EVENT_KEY_UP: keyboard_update(&event); break;
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
