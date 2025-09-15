/*
   ===================================
   C O M M O N - C
   AUDIO ABSTRACTION MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- AUDIO MODULE ---

    this module provides a minimalist, cross-platform
    abstraction layer for basic audio playback and mixing.
    it defines common structures and functions for audio
    buffers, sound effects, and music streams.

    the actual audio implementation is left to the
    user, allowing integration with various audio APIs
    (e.g., SDL_mixer, OpenAL, or custom audio engines)
    while maintaining C89 compliance.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_AUDIO_H
#define COMMC_AUDIO_H

#include <stddef.h>                   /* for size_t */

/*
	==================================
             --- DEFINES ---
	==================================
*/

#define COMMC_AUDIO_MAX_CHANNELS 8   /* maximum number of simultaneous 
                                         sound channels */

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* opaque audio context. */

typedef struct commc_audio_context_t commc_audio_context_t;

/* audio buffer structure (raw PCM data). */

typedef struct {

  void*    data;             /* raw audio data */
  size_t   size;             /* size of data in bytes */
  int      sample_rate;      /* samples per second */
  int      channels;         /* 1 for mono, 2 for stereo */
  int      bits_per_sample;  /* 8 or 16 */

} commc_audio_buffer_t;

/* opaque sound effect handle. */

typedef struct commc_sound_t commc_sound_t;

/* opaque music stream handle. */

typedef struct commc_music_t commc_music_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_audio_init()
	       ---
	       initializes the audio subsystem.
	       returns a context pointer on success, NULL on failure.
	       the user must provide platform-specific initialization.

*/

commc_audio_context_t* commc_audio_init(int sample_rate, int channels, int buffer_size);

/*

         commc_audio_shutdown()
	       ---
	       shuts down the audio subsystem and frees resources.

*/

void commc_audio_shutdown(commc_audio_context_t* context);

/*

         commc_audio_buffer_create()
	       ---
	       creates an audio buffer from raw PCM data.
	       caller must free data with commc_audio_buffer_destroy.

*/

commc_audio_buffer_t* commc_audio_buffer_create(const void* data, size_t size, int sample_rate, int channels, int bits_per_sample);

/*

         commc_audio_buffer_destroy()
	       ---
	       frees the audio buffer's data.

*/

void commc_audio_buffer_destroy(commc_audio_buffer_t* buffer);

/*

         commc_sound_load()
	       ---
	       loads a sound effect from an audio buffer.
	       returns a sound handle on success, NULL on failure.

*/

commc_sound_t* commc_sound_load(commc_audio_context_t* context, const commc_audio_buffer_t* buffer);

/*

         commc_sound_destroy()
	       ---
	       frees a loaded sound effect.

*/

void commc_sound_destroy(commc_sound_t* sound);

/*

         commc_sound_play()
	       ---
	       plays a sound effect on a specific channel.
	       returns 1 on success, 0 on failure.

*/

int commc_sound_play(commc_audio_context_t* context, commc_sound_t* sound, int channel, float volume, int loops);

/*

         commc_sound_stop()
	       ---
	       stops a sound effect on a specific channel.

*/

void commc_sound_stop(commc_audio_context_t* context, int channel);

/*

         commc_music_load()
	       ---
	       loads music from an audio buffer.
	       returns a music handle on success, NULL on failure.

*/

commc_music_t* commc_music_load(commc_audio_context_t* context, const commc_audio_buffer_t* buffer);

/*

         commc_music_destroy()
	       ---
	       frees loaded music.

*/

void commc_music_destroy(commc_music_t* music);

/*

         commc_music_play()
	       ---
	       plays music. returns 1 on success, 0 on failure.

*/

int commc_music_play(commc_audio_context_t* context, commc_music_t* music, float volume, int loops);

/*

         commc_music_stop()
	       ---
	       stops playing music.

*/

void commc_music_stop(commc_audio_context_t* context);

/*

         commc_audio_set_master_volume()
	       ---
	       sets the global master volume (0.0 to 1.0).

*/

void commc_audio_set_master_volume(commc_audio_context_t* context, float volume);

#endif /* COMMC_AUDIO_H */

/*
	==================================
             --- EOF ---
	==================================
*/
