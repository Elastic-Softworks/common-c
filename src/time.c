/*
   ===================================
   C O M M O N - C
   TIME IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- TIME MODULE ---

    implementation of the time utilities.
    see include/commc/time.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/time.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_time_get_seconds()
	       ---
	       returns a double-precision value of the
	       current time in seconds.

*/

double commc_time_get_seconds(void) {

  return (double)clock() / CLOCKS_PER_SEC;

}

/*

         commc_timer_start()
	       ---
	       initializes and starts a timer. if the timer
	       is already running, it does nothing.

*/

void commc_timer_start(commc_timer_t* timer) {

  if  (timer && !timer->is_running) {

    timer->is_running = 1;
    timer->start_time = clock();

  }

}

/*

         commc_timer_stop()
	       ---
	       stops a timer and accumulates the elapsed time.
	       if the timer is not running, it does nothing.

*/

void commc_timer_stop(commc_timer_t* timer) {

  if  (timer && timer->is_running) {

    timer->elapsed_time += clock() - timer->start_time;
    timer->is_running    = 0;

  }

}

/*

         commc_timer_reset()
	       ---
	       resets a timer's state to zero.

*/

void commc_timer_reset(commc_timer_t* timer) {

  if  (timer) {

    timer->elapsed_time = 0;
    timer->start_time   = 0;
    timer->is_running   = 0;

  }

}

/*

         commc_timer_get_elapsed_seconds()
	       ---
	       returns the total accumulated time in seconds.

*/

double commc_timer_get_elapsed_seconds(commc_timer_t* timer) {

  if  (!timer) {

    return 0.0;

  }

  if  (timer->is_running) {

    return ((double)timer->elapsed_time + (clock() - timer->start_time)) / CLOCKS_PER_SEC;

  } else {

    return (double)timer->elapsed_time / CLOCKS_PER_SEC;

  }

}

/*

         commc_timer_get_elapsed_ms()
	       ---
	       returns the total accumulated time in milliseconds.

*/

double commc_timer_get_elapsed_ms(commc_timer_t* timer) {

  return commc_timer_get_elapsed_seconds(timer) * 1000.0;

}

/*

         commc_timer_is_running()
	       ---
	       checks if the timer is currently active.

*/

int commc_timer_is_running(commc_timer_t* timer) {

  if  (!timer) {

    return 0;

  }

  return timer->is_running;

}

/*

         commc_sleep_ms()
	       ---
	       provides a cross-platform way to pause execution.

*/

void commc_sleep_ms(unsigned int milliseconds) {

#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif

}

/*

         commc_time_get_datetime_str()
	       ---
	       formats the current time and date into a string.

*/

void commc_time_get_datetime_str(char* buffer, size_t buffer_size, const char* format) {

  time_t    raw_time;
  struct tm* time_info;

  time(&raw_time);
  time_info = localtime(&raw_time);

  strftime(buffer, buffer_size, format, time_info);

}

/*
	==================================
             --- EOF ---
	==================================
*/
