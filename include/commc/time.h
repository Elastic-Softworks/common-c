/*
   ===================================
   C O M M O N - C
   TIME UTILITIES MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- TIME MODULE ---

    this module provides basic time and timer
    functionality for game loops and performance
    measurement. it is designed to be as portable
    as possible under C89.

    note: high-precision timing is platform-specific.
    this implementation uses standard library functions
    which may have limited resolution.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef   COMMC_TIME_H
#define   COMMC_TIME_H

#include  <stddef.h>
#include  <time.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* simple timer structure. */

typedef struct {

  clock_t   start_time;   /* time when the timer was started */
  clock_t   elapsed_time; /* total elapsed time */
  int       is_running;   /* flag to check if timer is active */

} commc_timer_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_time_get_seconds()
	       ---
	       returns the current time in seconds as a double.
	       precision depends on the system's clock() implementation.

*/

double commc_time_get_seconds(void);

/*

         commc_timer_start()
	       ---
	       starts or resumes a timer.

*/

void commc_timer_start(commc_timer_t* timer);

/*

         commc_timer_stop()
	       ---
	       stops or pauses a timer, updating its elapsed time.

*/

void commc_timer_stop(commc_timer_t* timer);

/*

         commc_timer_reset()
	       ---
	       resets a timer's elapsed time to zero.

*/

void commc_timer_reset(commc_timer_t* timer);

/*

         commc_timer_get_elapsed_seconds()
	       ---
	       returns the total elapsed time of a timer in seconds.

*/

double commc_timer_get_elapsed_seconds(commc_timer_t* timer);

/*

         commc_timer_get_elapsed_ms()
	       ---
	       returns the total elapsed time of a timer in milliseconds.

*/

double commc_timer_get_elapsed_ms(commc_timer_t* timer);

/*

         commc_timer_is_running()
	       ---
	       returns 1 if the timer is currently running, otherwise 0.

*/

int commc_timer_is_running(commc_timer_t* timer);

/*

         commc_sleep_ms()
	       ---
	       pauses the execution of the program for a given
	       number of milliseconds. portability may vary.

*/

void commc_sleep_ms(unsigned int milliseconds);

/*

         commc_time_get_datetime_str()
	       ---
	       gets the current date and time as a formatted string.
	       uses strftime format codes.

*/

void commc_time_get_datetime_str(char* buffer, size_t buffer_size, const char* format);

#endif /* COMMC_TIME_H */

/*
	==================================
             --- EOF ---
	==================================
*/
