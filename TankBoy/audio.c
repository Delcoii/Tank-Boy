#include "audio.h"
#include <stdio.h>

// Global audio system instance
AudioSystem audio_system = {0};

void audio_init(void) {
    printf("Initializing audio system...\n");
    
    // Initialize Allegro audio
    if (!al_install_audio()) {
        printf("ERROR: Could not install audio system\n");
        return;
    }
    printf("Audio system installed successfully\n");
    
    // Initialize audio codec addon
    if (!al_init_acodec_addon()) {
        printf("ERROR: Could not initialize audio codec addon\n");
        return;
    }
    printf("Audio codec addon initialized successfully\n");
    
    // Reserve audio samples
    if (!al_reserve_samples(16)) {
        printf("ERROR: Could not reserve audio samples\n");
        return;
    }
    printf("Reserved 16 audio samples\n");
    
    // Load intro BGM
    audio_system.intro_bgm = al_load_sample("TankBoy/resources/flacs/intro_bgm.flac");
    if (!audio_system.intro_bgm) {
        printf("ERROR: Could not load intro_bgm.flac\n");
        return;
    }
    printf("intro_bgm.flac loaded successfully\n");
    
    // Create sample instance for better control
    audio_system.intro_bgm_instance = al_create_sample_instance(audio_system.intro_bgm);
    if (!audio_system.intro_bgm_instance) {
        printf("ERROR: Could not create sample instance\n");
        return;
    }
    printf("Sample instance created successfully\n");
    
    // Configure sample instance
    al_set_sample_instance_playmode(audio_system.intro_bgm_instance, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_gain(audio_system.intro_bgm_instance, 0.3); // 30% volume
    al_set_sample_instance_speed(audio_system.intro_bgm_instance, 1.0); // Normal speed
    
    // Attach to default mixer
    al_attach_sample_instance_to_mixer(audio_system.intro_bgm_instance, al_get_default_mixer());
    printf("Sample instance configured and attached to mixer\n");
    
    audio_system.is_initialized = true;
    printf("Audio system initialized successfully\n");
}

void audio_cleanup(void) {
    if (audio_system.intro_bgm_instance) {
        al_destroy_sample_instance(audio_system.intro_bgm_instance);
        audio_system.intro_bgm_instance = NULL;
    }
    
    if (audio_system.intro_bgm) {
        al_destroy_sample(audio_system.intro_bgm);
        audio_system.intro_bgm = NULL;
    }
    
    audio_system.is_initialized = false;
    printf("Audio system cleaned up\n");
}

void play_intro_bgm(void) {
    if (!audio_system.is_initialized || !audio_system.intro_bgm_instance) {
        printf("WARNING: Audio system not initialized, cannot play BGM\n");
        return;
    }
    
    // Check if already playing
    if (al_get_sample_instance_playing(audio_system.intro_bgm_instance)) {
        printf("BGM is already playing\n");
        return;
    }
    
    // Play the sample instance
    al_play_sample_instance(audio_system.intro_bgm_instance);
    printf("BGM playback started (normal speed, 30%% volume)\n");
    
    // Verify it's playing
    if (al_get_sample_instance_playing(audio_system.intro_bgm_instance)) {
        printf("BGM is now playing successfully\n");
    } else {
        printf("ERROR: BGM failed to start playing\n");
    }
}

void stop_intro_bgm(void) {
    if (!audio_system.is_initialized || !audio_system.intro_bgm_instance) {
        return;
    }
    
    if (al_get_sample_instance_playing(audio_system.intro_bgm_instance)) {
        al_stop_sample_instance(audio_system.intro_bgm_instance);
        printf("BGM stopped\n");
    }
}

bool is_intro_bgm_playing(void) {
    if (!audio_system.is_initialized || !audio_system.intro_bgm_instance) {
        return false;
    }
    
    return al_get_sample_instance_playing(audio_system.intro_bgm_instance);
}

void set_intro_bgm_volume(float volume) {
    if (!audio_system.is_initialized || !audio_system.intro_bgm_instance) {
        return;
    }
    
    // Clamp volume between 0.0 and 1.0
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    al_set_sample_instance_gain(audio_system.intro_bgm_instance, volume);
    printf("BGM volume set to %.1f%%\n", volume * 100.0f);
}
