#ifndef AUDIO_H
#define AUDIO_H

#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

// Audio system structure
typedef struct {
    ALLEGRO_SAMPLE* intro_bgm;
    ALLEGRO_SAMPLE_INSTANCE* intro_bgm_instance;
    bool is_initialized;
} AudioSystem;

// Function declarations
void audio_init(void);
void audio_cleanup(void);
void play_intro_bgm(void);
void stop_intro_bgm(void);
bool is_intro_bgm_playing(void);
void set_intro_bgm_volume(float volume);

// Global audio system instance
extern AudioSystem audio_system;

#endif // AUDIO_H
