/*
   ===================================
   C O M M O N - C
   INPUT HANDLING MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- INPUT MODULE ---

    this module provides cross-platform input handling
    abstractions for keyboard and mouse input.
    
    the actual input polling is platform-specific and
    requires integration with your windowing system or
    input API (e.g., SDL, Win32, X11).

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef   COMMC_INPUT_H
#define   COMMC_INPUT_H

#include  <stddef.h>       /* for size_t */

/*
	==================================
             --- DEFINES ---
	==================================
*/

/* key codes for common keyboard keys. */

typedef enum {

  COMMC_KEY_UNKNOWN = -1,

  /* letter keys */
  COMMC_KEY_A = 'A',  COMMC_KEY_B = 'B',  COMMC_KEY_C = 'C',
  COMMC_KEY_D = 'D',  COMMC_KEY_E = 'E',  COMMC_KEY_F = 'F',
  COMMC_KEY_G = 'G',  COMMC_KEY_H = 'H',  COMMC_KEY_I = 'I',
  COMMC_KEY_J = 'J',  COMMC_KEY_K = 'K',  COMMC_KEY_L = 'L',
  COMMC_KEY_M = 'M',  COMMC_KEY_N = 'N',  COMMC_KEY_O = 'O',
  COMMC_KEY_P = 'P',  COMMC_KEY_Q = 'Q',  COMMC_KEY_R = 'R',
  COMMC_KEY_S = 'S',  COMMC_KEY_T = 'T',  COMMC_KEY_U = 'U',
  COMMC_KEY_V = 'V',  COMMC_KEY_W = 'W',  COMMC_KEY_X = 'X',
  COMMC_KEY_Y = 'Y',  COMMC_KEY_Z = 'Z',

  /* number keys */
  COMMC_KEY_0 = '0',  COMMC_KEY_1 = '1',  COMMC_KEY_2 = '2',
  COMMC_KEY_3 = '3',  COMMC_KEY_4 = '4',  COMMC_KEY_5 = '5',
  COMMC_KEY_6 = '6',  COMMC_KEY_7 = '7',  COMMC_KEY_8 = '8',
  COMMC_KEY_9 = '9',

  /* special keys (platform-specific values) */
  COMMC_KEY_SPACE     = 256,
  COMMC_KEY_ENTER     = 257,
  COMMC_KEY_ESCAPE    = 258,
  COMMC_KEY_BACKSPACE = 259,
  COMMC_KEY_TAB       = 260,
  COMMC_KEY_LEFT      = 261,
  COMMC_KEY_RIGHT     = 262,
  COMMC_KEY_UP        = 263,
  COMMC_KEY_DOWN      = 264,
  COMMC_KEY_SHIFT     = 265,
  COMMC_KEY_CTRL      = 266,
  COMMC_KEY_ALT       = 267,

  COMMC_KEY_LAST      = 268

} commc_key_code_t;

/* mouse button identifiers. */

typedef enum {

  COMMC_MOUSE_LEFT   = 0,
  COMMC_MOUSE_RIGHT  = 1,
  COMMC_MOUSE_MIDDLE = 2,

  COMMC_MOUSE_BUTTON_LAST = 3

} commc_mouse_button_t;

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* keyboard event structure. */

typedef struct {

  commc_key_code_t key;        /* which key */
  int              pressed;    /* 1 if pressed, 0 if released */

} commc_key_event_t;

/* mouse motion event structure. */

typedef struct {

  int x;                       /* mouse x coordinate */
  int y;                       /* mouse y coordinate */

} commc_mouse_motion_event_t;

/* mouse button event structure. */

typedef struct {

  commc_mouse_button_t button;   /* which button */
  int                  pressed;  /* 1 if pressed, 0 if released */
  int                  x;        /* mouse x coordinate */
  int                  y;        /* mouse y coordinate */

} commc_mouse_button_event_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_input_poll_keyboard()
	       ---
	       polls all keyboard events since last call.
	       returns array of events and sets count.
	       caller should not free the returned array.

*/

commc_key_event_t* commc_input_poll_keyboard(size_t* count);

/*

         commc_input_poll_mouse_motion()
	       ---
	       polls current mouse position and motion.
	       returns motion event with current coordinates.

*/

commc_mouse_motion_event_t commc_input_poll_mouse_motion(void);

/*

         commc_input_poll_mouse_button()
	       ---
	       polls the state of a specific mouse button.
	       returns 1 if currently pressed, 0 otherwise.

*/

int commc_input_poll_mouse_button(commc_mouse_button_t button);

/*

         commc_input_is_key_pressed()
	       ---
	       checks if a specific key is currently pressed.
	       returns 1 if pressed, 0 otherwise.

*/

int commc_input_is_key_pressed(commc_key_code_t key);

/*

         commc_input_is_mouse_button_pressed()
	       ---
	       checks if a specific mouse button is currently pressed.
	       returns 1 if pressed, 0 otherwise.

*/

int commc_input_is_mouse_button_pressed(commc_mouse_button_t button);

/*

         commc_input_get_mouse_position()
	       ---
	       gets the current mouse position.
	       returns motion event with current coordinates.

*/

commc_mouse_motion_event_t commc_input_get_mouse_position(void);

#endif /* COMMC_INPUT_H */

/*
	==================================
             --- EOF ---
	==================================
*/