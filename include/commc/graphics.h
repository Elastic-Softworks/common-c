/*
   ===================================
   C O M M O N - C
   GRAPHICS ABSTRACTION MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- GRAPHICS MODULE ---

    this module provides a minimalist, cross-platform
    abstraction layer for basic 2D graphics rendering.
    it defines common structures and functions for colors,
    rectangles, bitmaps, and drawing primitives.

    the actual rendering implementation is left to the
    user, allowing integration with various graphics APIs
    (e.g., SDL, OpenGL, or custom software renderers)
    while maintaining C89 compliance.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_GRAPHICS_H
#define COMMC_GRAPHICS_H

#include <stddef.h>       /* for size_t */

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* 8-bit RGBA color structure. */

typedef struct {

  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;

} commc_color_t;

/* rectangle structure. */

typedef struct {

  int x;
  int y;
  int w;
  int h;

} commc_rect_t;

/* bitmap structure. */

typedef struct {

  unsigned char*  pixels; /* raw pixel data (RGBA) */

  int  width;  /* width in pixels */
  int  height; /* height in pixels */

} commc_bitmap_t;

/* opaque rendering context. */

typedef struct commc_graphics_context_t commc_graphics_context_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_graphics_init()
	       ---
	       initializes the graphics subsystem.
	       returns a context pointer on success, NULL on failure.
	       the user must provide platform-specific initialization.

*/

commc_graphics_context_t* commc_graphics_init(int width, int height, const char* title);

/*

         commc_graphics_shutdown()
	       ---
	       shuts down the graphics subsystem and frees resources.

*/

void commc_graphics_shutdown(commc_graphics_context_t* context);

/*

         commc_graphics_begin_frame()
	       ---
	       prepares the renderer for a new frame.

*/

void commc_graphics_begin_frame(commc_graphics_context_t* context);

/*

         commc_graphics_end_frame()
	       ---
	       presents the rendered frame to the display.

*/

void commc_graphics_end_frame(commc_graphics_context_t* context);

/*

         commc_graphics_clear()
	       ---
	       clears the screen with a specified color.

*/

void commc_graphics_clear(commc_graphics_context_t* context, commc_color_t color);

/*

         commc_graphics_draw_pixel()
	       ---
	       draws a single pixel at (x, y) with a given color.

*/

void commc_graphics_draw_pixel(commc_graphics_context_t* context, int x, int y, commc_color_t color);

/*

         commc_graphics_draw_line()
	       ---
	       draws a line between two points.

*/

void commc_graphics_draw_line(commc_graphics_context_t* context, int x1, int y1, int x2, int y2, commc_color_t color);

/*

         commc_graphics_draw_rect()
	       ---
	       draws the outline of a rectangle.

*/

void commc_graphics_draw_rect(commc_graphics_context_t* context, commc_rect_t rect, commc_color_t color);

/*

         commc_graphics_fill_rect()
	       ---
	       fills a rectangle with a solid color.

*/

void commc_graphics_fill_rect(commc_graphics_context_t* context, commc_rect_t rect, commc_color_t color);

/*

         commc_graphics_draw_bitmap()
	       ---
	       draws a bitmap at a specified position.

*/

void commc_graphics_draw_bitmap(commc_graphics_context_t* context, const commc_bitmap_t* bitmap, int x, int y);

/*

         commc_bitmap_create()
	       ---
	       creates a new bitmap with specified dimensions.
	       caller must free pixels with commc_bitmap_destroy.

*/

commc_bitmap_t* commc_bitmap_create(int width, int height);

/*

         commc_bitmap_destroy()
	       ---
	       frees the pixel data of a bitmap.

*/

void commc_bitmap_destroy(commc_bitmap_t* bitmap);

/*

         commc_bitmap_set_pixel()
	       ---
	       sets the color of a pixel in a bitmap.

*/

void commc_bitmap_set_pixel(commc_bitmap_t* bitmap, int x, int y, commc_color_t color);

/*

         commc_bitmap_get_pixel()
	       ---
	       gets the color of a pixel from a bitmap.

*/

commc_color_t commc_bitmap_get_pixel(const commc_bitmap_t* bitmap, int x, int y);

#endif /* COMMC_GRAPHICS_H */

/*
	==================================
             --- EOF ---
	==================================
*/
