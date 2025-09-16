/*
   ===================================
   C O M M O N - C
   AUDIO ABSTRACTION IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- AUDIO MODULE ---

    implementation of the basic audio playback and mixing
    abstraction. this module provides conceptual functions
    that a user would implement with their chosen audio API.
    see include/commc/audio.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include  <stdlib.h>       /* for malloc, free */
#include  <string.h>       /* for memset */
#include  <stdio.h>        /* for printf (conceptual logging) */

#include  "commc/audio.h"
#include  "commc/error.h"  /* for error handling */

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal audio context structure. */

struct commc_audio_context_t {

  int sample_rate;  /* desired sample rate */
  int channels;     /* desired number of channels (mono/stereo) */
  int buffer_size;  /* size of internal audio buffer */

  /* in a real implementation, this would hold platform-specific handles */
  /* (e.g., SDL_AudioDeviceID, OpenAL context, etc.) */

};

/* internal sound effect structure. */

struct commc_sound_t {

  commc_audio_buffer_t* buffer; /* reference to the audio data */

  /* platform-specific sound handle */

};

/* internal music stream structure. */

struct commc_music_t {

  commc_audio_buffer_t* buffer; /* reference to the audio data */

  /* platform-specific music handle */

};

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_audio_init()
	       ---
	       initializes the audio context.
	       this is a conceptual function; user must implement
	       platform-specific audio device initialization.

*/

commc_audio_context_t* commc_audio_init(int sample_rate, int channels, int buffer_size) {

  commc_audio_context_t* context;
  
  context = (commc_audio_context_t*)malloc(sizeof(commc_audio_context_t));

  if  (!context) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  context->sample_rate = sample_rate;
  context->channels    = channels;
  context->buffer_size = buffer_size;

  printf("AUDIO INITIALIZED: SAMPLE RATE=%d, CHANNELS=%d, BUFFER SIZE=%d...!\n", sample_rate, channels, buffer_size);

  return context;

}

/*

         commc_audio_shutdown()
	       ---
	       shuts down the audio context.
	       user must implement platform-specific audio device shutdown.

*/

void commc_audio_shutdown(commc_audio_context_t* context) {

  if  (context) {

    printf("AUDIO SHUTDOWN...!\n");
    free(context);

  }

}

/*

         commc_audio_buffer_create()
	       ---
	       creates an audio buffer from raw PCM data.

*/

commc_audio_buffer_t* commc_audio_buffer_create(const void* data, size_t size, int sample_rate, int channels, int bits_per_sample) {

  commc_audio_buffer_t* buffer;
  
  if  (!data || size == 0 || sample_rate <= 0 || channels <= 0 || (bits_per_sample != 8 && bits_per_sample != 16)) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  buffer = (commc_audio_buffer_t*)malloc(sizeof(commc_audio_buffer_t));

  if  (!buffer) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  buffer->data            = malloc(size);

  if  (!buffer->data) {

    free(buffer);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  memcpy(buffer->data, data, size);

  buffer->size            = size;
  buffer->sample_rate     = sample_rate;
  buffer->channels        = channels;
  buffer->bits_per_sample = bits_per_sample;

  return buffer;

}

/*

         commc_audio_buffer_destroy()
	       ---
	       frees the audio buffer's data.

*/

void commc_audio_buffer_destroy(commc_audio_buffer_t* buffer) {

  if  (buffer) {

    if  (buffer->data) {

      free(buffer->data);

    }

    free(buffer);

  }

}

/*

         commc_sound_load()
	       ---
	       loads a sound effect.
	       user must implement platform-specific sound loading.

*/

commc_sound_t* commc_sound_load(commc_audio_context_t* context, const commc_audio_buffer_t* buffer) {

  commc_sound_t* sound;
  
  if  (!context || !buffer) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  sound = (commc_sound_t*)malloc(sizeof(commc_sound_t));

  if  (!sound) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  sound->buffer = (commc_audio_buffer_t*)buffer; /* reference, not copy */
  printf("SOUND LOADED FROM BUFFER (SIZE: %lu BYTES)...!\n", (unsigned long)buffer->size);

  return sound;

}

/*

         commc_sound_destroy()
	       ---
	       frees a loaded sound effect.
	       user must implement platform-specific sound freeing.

*/

void commc_sound_destroy(commc_sound_t* sound) {

  if  (sound) {

    printf("SOUND DESTROYED...!\n");

    free(sound);

  }

}

/*

         commc_sound_play()
	       ---
	       plays a sound effect.
	       user must implement platform-specific sound playback.

*/

int commc_sound_play(commc_audio_context_t* context, commc_sound_t* sound, int channel, float volume, int loops) {

  if  (!context || !sound || channel < 0 || channel >= COMMC_AUDIO_MAX_CHANNELS || volume < 0.0f || volume > 1.0f) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

    return 0;

  }

  printf("PLAYING SOUND ON CHANNEL %d (VOLUME: %.2f, LOOPS: %d)...!\n", channel, volume, loops);

  return 1;

}

/*

         commc_sound_stop()
	       ---
	       stops a sound effect.
	       user must implement platform-specific sound stopping.

*/

void commc_sound_stop(commc_audio_context_t* context, int channel) {

  if  (!context || channel < 0 || channel >= COMMC_AUDIO_MAX_CHANNELS) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

    return;

  }

  printf("STOPPING SOUND ON CHANNEL %d...!\n", channel);

}

/*

         commc_music_load()
	       ---
	       loads music.
	       user must implement platform-specific music loading.

*/

commc_music_t* commc_music_load(commc_audio_context_t* context, const commc_audio_buffer_t* buffer) {

  commc_music_t* music;
  
  if  (!context || !buffer) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

    return NULL;

  }

  music = (commc_music_t*)malloc(sizeof(commc_music_t));

  if  (!music) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);

    return NULL;

  }

  music->buffer = (commc_audio_buffer_t*)buffer;        /* reference, not copy */

  printf("MUSIC LOADED FROM BUFFER (SIZE: %lu BYTES)...!\n", (unsigned long)buffer->size);

  return music;

}

/*

         commc_music_destroy()
	       ---
	       frees loaded music.
	       user must implement platform-specific music freeing.

*/

void commc_music_destroy(commc_music_t* music) {

  if  (music) {

    printf("MUSIC DESTROYED...!\n");
    
    free(music);

  }

}

/*

         commc_music_play()
	       ---
	       plays music.
	       user must implement platform-specific music playback.

*/

int commc_music_play(commc_audio_context_t* context, commc_music_t* music, float volume, int loops) {

  if  (!context || !music || volume < 0.0f || volume > 1.0f) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

  printf("PLAYING MUSIC (VOLUME: %.2f, LOOPS: %d)...!\n", volume, loops);

  return 1;

}

/*

         commc_music_stop()
	       ---
	       stops playing music.
	       user must implement platform-specific music stopping.

*/

void commc_music_stop(commc_audio_context_t* context) {

  if  (!context) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  printf("STOPPING MUSIC...!\n");

}

/*

         commc_audio_set_master_volume()
	       ---
	       sets the global master volume.
	       user must implement platform-specific volume control.

*/

void commc_audio_set_master_volume(commc_audio_context_t* context, float volume) {

  if  (!context || volume < 0.0f || volume > 1.0f) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  printf("SETTING MASTER VOLUME TO %.2f...!\n", volume);

}

/*
	==================================
             --- EOF ---
	==================================
*/
