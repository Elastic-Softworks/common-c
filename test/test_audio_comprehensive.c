/*
    ===================================================

        COMMON-C TEST MODULE: AUDIO COMPREHENSIVE TESTS
           ---
           comprehensive test coverage for audio.c
           following established methodology template

        Note: This implementation uses stub functions with
           printf outputs for testing the API structure.
           Platform-specific implementations would replace
           the printf statements with actual audio calls.

    ===================================================
    
    Public Functions Tested:
    
    - commc_audio_context_init()
    - commc_audio_context_destroy() 
    - commc_audio_buffer_create()
    - commc_audio_buffer_destroy()
    - commc_sound_load()
    - commc_sound_destroy()
    - commc_sound_play()
    - commc_sound_stop()
    - commc_music_load()
    - commc_music_destroy()
    - commc_music_play()
    - commc_music_stop()
    - commc_audio_set_master_volume()
    
*/

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>

#include    "test_harness.h"
#include    "../include/commc/audio.h"
#include    "../include/commc/error.h"

/*
    ==================================
             TEST FUNCTIONS
    ==================================
*/

/*

    audio_context_lifecycle()
    ---
    tests audio context creation and destruction

*/

void test_audio_context_lifecycle(void) {

    commc_audio_context_t* context;
    
    printf("TESTING: AUDIO CONTEXT LIFECYCLE...\n");
    
    /* test normal context creation with typical audio parameters */

    context = commc_audio_init(44100, 2, 512);   /* 44.1kHz, stereo, 512-byte buffer */
    ASSERT_NOT_NULL(context);
    
    /* test context destruction */

    commc_audio_shutdown(context);
    
    /* test null context destruction (should handle gracefully) */

    commc_audio_shutdown(NULL);
    
    printf("PASSED: AUDIO CONTEXT LIFECYCLE\n\n");

}

/*

    audio_buffer_lifecycle()
    ---
    tests audio buffer creation and destruction

*/

void test_audio_buffer_lifecycle(void) {

    commc_audio_buffer_t* buffer;
    const char* test_data = "test_audio_data";
    size_t data_size = strlen(test_data) + 1;
    
    printf("TESTING: AUDIO BUFFER LIFECYCLE...\n");
    
    /* test normal buffer creation with typical audio parameters */

    buffer = commc_audio_buffer_create(test_data, data_size, 44100, 2, 16);
    ASSERT_NOT_NULL(buffer);
    ASSERT_NOT_NULL(buffer->data);
    ASSERT_TRUE(buffer->size == data_size);
    ASSERT_TRUE(buffer->sample_rate == 44100);
    ASSERT_TRUE(buffer->channels == 2);
    ASSERT_TRUE(buffer->bits_per_sample == 16);
    ASSERT_TRUE(memcmp(buffer->data, test_data, data_size) == 0);
    
    /* test buffer destruction */

    commc_audio_buffer_destroy(buffer);
    
    /* test null buffer destruction (should handle gracefully) */
    
    commc_audio_buffer_destroy(NULL);
    
    printf("PASSED: AUDIO BUFFER LIFECYCLE\n\n");

}

/*

    audio_buffer_null_safety()
    ---
    tests audio buffer functions with null parameters

*/

void test_audio_buffer_null_safety(void) {

    commc_audio_buffer_t* buffer;
    
    printf("TESTING: AUDIO BUFFER NULL SAFETY...\n");
    
    /* test buffer creation with null data */

    buffer = commc_audio_buffer_create(NULL, 100, 44100, 2, 16);
    ASSERT_NULL(buffer);
    
    /* test buffer creation with zero size */

    buffer = commc_audio_buffer_create("test", 0, 44100, 2, 16);
    ASSERT_NULL(buffer);
    
    /* test buffer creation with invalid sample rate */

    buffer = commc_audio_buffer_create("test", 4, 0, 2, 16);
    ASSERT_NULL(buffer);
    
    /* test buffer creation with invalid channels */

    buffer = commc_audio_buffer_create("test", 4, 44100, 0, 16);
    ASSERT_NULL(buffer);
    
    /* test buffer creation with invalid bits per sample */

    buffer = commc_audio_buffer_create("test", 4, 44100, 2, 0);
    ASSERT_NULL(buffer);
    
    printf("PASSED: AUDIO BUFFER NULL SAFETY\n\n");

}

/*

    sound_lifecycle()
    ---
    tests sound loading and destruction

*/

void test_sound_lifecycle(void) {

    commc_audio_context_t* context;
    commc_audio_buffer_t* buffer;
    commc_sound_t* sound;
    const char* test_data = "sound_test_data";
    size_t data_size = strlen(test_data) + 1;
    
    printf("TESTING: SOUND LIFECYCLE...\n");
    
    context = commc_audio_init(44100, 2, 512);
    ASSERT_NOT_NULL(context);
    
    buffer = commc_audio_buffer_create(test_data, data_size, 44100, 2, 16);
    ASSERT_NOT_NULL(buffer);
    
    /* test normal sound loading */

    sound = commc_sound_load(context, buffer);
    ASSERT_NOT_NULL(sound);
    
    /* test sound destruction */

    commc_sound_destroy(sound);
    
    /* test null sound destruction (should handle gracefully) */
    
    commc_sound_destroy(NULL);
    
    commc_audio_buffer_destroy(buffer);
    commc_audio_shutdown(context);
    
    printf("PASSED: SOUND LIFECYCLE\n\n");

}

/*

    sound_null_safety()
    ---
    tests sound functions with null parameters

*/

void test_sound_null_safety(void) {

    commc_audio_context_t* context;
    commc_audio_buffer_t* buffer;
    commc_sound_t* sound;
    const char* test_data = "sound_data";
    
    printf("TESTING: SOUND NULL SAFETY...\n");
    
    context = commc_audio_init(44100, 2, 512);
    buffer = commc_audio_buffer_create(test_data, strlen(test_data), 44100, 2, 16);
    
    /* test sound loading with null context */

    sound = commc_sound_load(NULL, buffer);
    ASSERT_NULL(sound);
    
    /* test sound loading with null buffer */
    
    sound = commc_sound_load(context, NULL);
    ASSERT_NULL(sound);
    
    commc_audio_buffer_destroy(buffer);
    commc_audio_shutdown(context);
    
    printf("PASSED: SOUND NULL SAFETY\n\n");

}

/*

    sound_playback()
    ---
    tests sound playback functionality

*/

void test_sound_playback(void) {

    commc_audio_context_t* context;
    commc_audio_buffer_t* buffer;
    commc_sound_t* sound;
    const char* test_data = "playback_data";
    int result;
    
    printf("TESTING: SOUND PLAYBACK...\n");
    
    context = commc_audio_init(44100, 2, 512);
    buffer = commc_audio_buffer_create(test_data, strlen(test_data), 44100, 2, 16);
    sound = commc_sound_load(context, buffer);
    
    /* test normal sound playback */

    result = commc_sound_play(context, sound, 0, 0.5f, 0);
    ASSERT_TRUE(result == 1);
    
    /* test playback with different parameters */

    result = commc_sound_play(context, sound, 3, 1.0f, 5);
    ASSERT_TRUE(result == 1);
    
    /* test playback with minimum volume */

    result = commc_sound_play(context, sound, 1, 0.0f, 0);
    ASSERT_TRUE(result == 1);
    
    /* test sound stop */

    commc_sound_stop(context, 0);
    commc_sound_stop(context, 3);
    
    commc_sound_destroy(sound);
    commc_audio_buffer_destroy(buffer);
    commc_audio_shutdown(context);
    
    printf("PASSED: SOUND PLAYBACK\n\n");

}

/*

    sound_playback_invalid_params()
    ---
    tests sound playback with invalid parameters

*/

void test_sound_playback_invalid_params(void) {

    commc_audio_context_t* context;
    commc_audio_buffer_t* buffer;
    commc_sound_t* sound;
    const char* test_data = "invalid_test_data";
    int result;
    
    printf("TESTING: SOUND PLAYBACK INVALID PARAMETERS...\n");
    
    context = commc_audio_init(44100, 2, 512);
    buffer = commc_audio_buffer_create(test_data, strlen(test_data), 44100, 2, 16);
    sound = commc_sound_load(context, buffer);
    
    /* test playback with null context */

    result = commc_sound_play(NULL, sound, 0, 0.5f, 0);
    ASSERT_TRUE(result == 0);
    
    /* test playback with null sound */

    result = commc_sound_play(context, NULL, 0, 0.5f, 0);
    ASSERT_TRUE(result == 0);
    
    /* test playback with invalid channel (negative) */

    result = commc_sound_play(context, sound, -1, 0.5f, 0);
    ASSERT_TRUE(result == 0);
    
    /* test playback with invalid channel (too high) */

    result = commc_sound_play(context, sound, COMMC_AUDIO_MAX_CHANNELS, 0.5f, 0);
    ASSERT_TRUE(result == 0);
    
    /* test playback with invalid volume (negative) */

    result = commc_sound_play(context, sound, 0, -0.1f, 0);
    ASSERT_TRUE(result == 0);
    
    /* test playback with invalid volume (too high) */

    result = commc_sound_play(context, sound, 0, 1.1f, 0);
    ASSERT_TRUE(result == 0);
    
    /* test stop with invalid parameters */

    commc_sound_stop(NULL, 0);
    commc_sound_stop(context, -1);
    commc_sound_stop(context, COMMC_AUDIO_MAX_CHANNELS);
    
    commc_sound_destroy(sound);
    commc_audio_buffer_destroy(buffer);
    commc_audio_shutdown(context);
    
    printf("PASSED: SOUND PLAYBACK INVALID PARAMETERS\n\n");

}

/*

    music_lifecycle()
    ---
    tests music loading and destruction

*/

void test_music_lifecycle(void) {

    commc_audio_context_t* context;
    commc_audio_buffer_t* buffer;
    commc_music_t* music;
    const char* test_data = "music_test_data";
    size_t data_size = strlen(test_data) + 1;
    
    printf("TESTING: MUSIC LIFECYCLE...\n");
    
    context = commc_audio_init(44100, 2, 512);
    ASSERT_NOT_NULL(context);
    
    buffer = commc_audio_buffer_create(test_data, data_size, 44100, 2, 16);
    ASSERT_NOT_NULL(buffer);
    
    /* test normal music loading */

    music = commc_music_load(context, buffer);
    ASSERT_NOT_NULL(music);
    
    /* test music destruction */

    commc_music_destroy(music);
    
    /* test null music destruction (should handle gracefully) */
    
    commc_music_destroy(NULL);
    
    commc_audio_buffer_destroy(buffer);
    commc_audio_shutdown(context);
    
    printf("PASSED: MUSIC LIFECYCLE\n\n");

}

/*

    music_null_safety()
    ---
    tests music functions with null parameters

*/

void test_music_null_safety(void) {

    commc_audio_context_t* context;
    commc_audio_buffer_t* buffer;
    commc_music_t* music;
    const char* test_data = "music_data";
    
    printf("TESTING: MUSIC NULL SAFETY...\n");
    
    context = commc_audio_init(44100, 2, 512);
    buffer = commc_audio_buffer_create(test_data, strlen(test_data), 44100, 2, 16);
    
    /* test music loading with null context */
    music = commc_music_load(NULL, buffer);
    ASSERT_NULL(music);
    
    /* test music loading with null buffer */
    music = commc_music_load(context, NULL);
    ASSERT_NULL(music);
    
    commc_audio_buffer_destroy(buffer);
    commc_audio_shutdown(context);
    
    printf("PASSED: MUSIC NULL SAFETY\n\n");

}

/*

    music_playback()
    ---
    tests music playback functionality

*/

void test_music_playback(void) {

    commc_audio_context_t* context;
    commc_audio_buffer_t* buffer;
    commc_music_t* music;
    const char* test_data = "music_playback_data";
    int result;
    
    printf("TESTING: MUSIC PLAYBACK...\n");
    
    context = commc_audio_init(44100, 2, 512);
    buffer = commc_audio_buffer_create(test_data, strlen(test_data), 44100, 2, 16);
    music = commc_music_load(context, buffer);
    
    /* test normal music playback */

    result = commc_music_play(context, music, 0.5f, 0);
    ASSERT_TRUE(result == 1);
    
    /* test playback with different parameters */

    result = commc_music_play(context, music, 1.0f, -1);  /* infinite loop */
    ASSERT_TRUE(result == 1);
    
    /* test playback with minimum volume */

    result = commc_music_play(context, music, 0.0f, 3);
    ASSERT_TRUE(result == 1);
    
    /* test music stop */
    commc_music_stop(context);
    
    commc_music_destroy(music);
    commc_audio_buffer_destroy(buffer);
    commc_audio_shutdown(context);
    
    printf("PASSED: MUSIC PLAYBACK\n\n");

}

/*

    music_playback_invalid_params()
    ---
    tests music playback with invalid parameters

*/

void test_music_playback_invalid_params(void) {

    commc_audio_context_t* context;
    commc_audio_buffer_t* buffer;
    commc_music_t* music;
    const char* test_data = "invalid_music_data";
    int result;
    
    printf("TESTING: MUSIC PLAYBACK INVALID PARAMETERS...\n");
    
    context = commc_audio_init(44100, 2, 512);
    buffer = commc_audio_buffer_create(test_data, strlen(test_data), 44100, 2, 16);
    music = commc_music_load(context, buffer);
    
    /* test playback with null context */

    result = commc_music_play(NULL, music, 0.5f, 0);
    ASSERT_TRUE(result == 0);
    
    /* test playback with null music */

    result = commc_music_play(context, NULL, 0.5f, 0);
    ASSERT_TRUE(result == 0);
    
    /* test playback with invalid volume (negative) */

    result = commc_music_play(context, music, -0.1f, 0);
    ASSERT_TRUE(result == 0);
    
    /* test playback with invalid volume (too high) */

    result = commc_music_play(context, music, 1.1f, 0);
    ASSERT_TRUE(result == 0);
    
    /* test stop with invalid parameters */

    commc_music_stop(NULL);  /* should handle null context gracefully */
    
    commc_music_destroy(music);
    commc_audio_buffer_destroy(buffer);
    commc_audio_shutdown(context);
    
    printf("PASSED: MUSIC PLAYBACK INVALID PARAMETERS\n\n");

}

/*

    master_volume()
    ---
    tests master volume control functionality

*/

void test_master_volume(void) {

    commc_audio_context_t* context;
    
    printf("TESTING: MASTER VOLUME CONTROL...\n");
    
    context = commc_audio_init(44100, 2, 512);
    ASSERT_NOT_NULL(context);
    
    /* test normal volume settings */

    commc_audio_set_master_volume(context, 0.0f);  /* minimum volume */
    commc_audio_set_master_volume(context, 0.5f);  /* medium volume */
    commc_audio_set_master_volume(context, 1.0f);  /* maximum volume */
    
    /* test volume with invalid parameters */
    
    commc_audio_set_master_volume(NULL, 0.5f);     /* null context */
    commc_audio_set_master_volume(context, -0.1f); /* negative volume */
    commc_audio_set_master_volume(context, 1.1f);  /* volume too high */
    
    commc_audio_shutdown(context);
    
    printf("PASSED: MASTER VOLUME CONTROL\n\n");

}

/*

    complex_audio_scenario()
    ---
    tests a complex scenario combining multiple audio operations

*/

void test_complex_audio_scenario(void) {

    commc_audio_context_t* context;
    commc_audio_buffer_t* sound_buffer;
    commc_audio_buffer_t* music_buffer;
    commc_sound_t* sound1;
    commc_sound_t* sound2;
    commc_music_t* music;
    const char* sound_data = "complex_sound_data";
    const char* music_data = "complex_music_data_longer_than_sound";
    int result;
    
    printf("TESTING: COMPLEX AUDIO SCENARIO...\n");
    
    /* initialize audio system */

    context = commc_audio_init(44100, 2, 512);
    ASSERT_NOT_NULL(context);
    
    /* set master volume */

    commc_audio_set_master_volume(context, 0.8f);
    
    /* create audio buffers */

    sound_buffer = commc_audio_buffer_create(sound_data, strlen(sound_data), 44100, 2, 16);
    ASSERT_NOT_NULL(sound_buffer);
    
    music_buffer = commc_audio_buffer_create(music_data, strlen(music_data), 44100, 2, 16);
    ASSERT_NOT_NULL(music_buffer);
    
    /* load sounds and music */

    sound1 = commc_sound_load(context, sound_buffer);
    ASSERT_NOT_NULL(sound1);
    
    sound2 = commc_sound_load(context, sound_buffer);  /* reuse same buffer */
    ASSERT_NOT_NULL(sound2);
    
    music = commc_music_load(context, music_buffer);
    ASSERT_NOT_NULL(music);
    
    /* start music playback */

    result = commc_music_play(context, music, 0.6f, -1);  /* infinite loop */
    ASSERT_TRUE(result == 1);
    
    /* play sounds on different channels */

    result = commc_sound_play(context, sound1, 0, 0.7f, 0);
    ASSERT_TRUE(result == 1);
    
    result = commc_sound_play(context, sound2, 1, 0.9f, 2);
    ASSERT_TRUE(result == 1);
    
    /* stop individual sounds */

    commc_sound_stop(context, 0);
    commc_sound_stop(context, 1);
    
    /* stop music */

    commc_music_stop(context);
    
    /* change master volume */

    commc_audio_set_master_volume(context, 0.3f);
    
    /* cleanup */

    commc_music_destroy(music);
    commc_sound_destroy(sound2);
    commc_sound_destroy(sound1);
    commc_audio_buffer_destroy(music_buffer);
    commc_audio_buffer_destroy(sound_buffer);
    commc_audio_shutdown(context);
    
    printf("PASSED: COMPLEX AUDIO SCENARIO\n\n");

}

/*
    ==================================
             MAIN TEST RUNNER
    ==================================
*/

int main(void) {

    printf("\n");
    printf("========================================\n");
    printf("  COMMON-C AUDIO COMPREHENSIVE TESTS\n");
    printf("========================================\n\n");

    MEMORY_LEAK_CHECK_START();

    /* basic lifecycle tests */

    ADD_TEST(audio_context_lifecycle);
    ADD_TEST(audio_buffer_lifecycle);
    ADD_TEST(sound_lifecycle);
    ADD_TEST(music_lifecycle);

    /* null safety tests */

    ADD_TEST(audio_buffer_null_safety);
    ADD_TEST(sound_null_safety);
    ADD_TEST(music_null_safety);

    /* functionality tests */

    ADD_TEST(sound_playback);
    ADD_TEST(sound_playback_invalid_params);
    ADD_TEST(music_playback);
    ADD_TEST(music_playback_invalid_params);
    ADD_TEST(master_volume);

    /* integration tests */

    ADD_TEST(complex_audio_scenario);

    RUN_ALL_TESTS();

    MEMORY_LEAK_CHECK_END();

    printf("========================================\n");
    printf("  ALL AUDIO TESTS COMPLETED\n");
    printf("========================================\n\n");

    return 0;

}

/*
    ==================================
             --- EOF ---
    ==================================
*/
