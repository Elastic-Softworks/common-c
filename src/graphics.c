/*
   ===================================
   C O M M O N - C
   GRAPHICS ABSTRACTION IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- GRAPHICS MODULE ---

    implementation of the basic 2D graphics rendering
    abstraction. this module provides a software-based
    rendering fallback for drawing primitives.
    see include/commc/graphics.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include <stdlib.h>          /* for malloc, free */
#include <string.h>          /* for memset */

#include "commc/graphics.h"
#include "commc/error.h"     /* for error handling */

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal graphics context structure. */

struct commc_graphics_context_t {

  commc_bitmap_t* framebuffer; /* the main rendering target */
  int             width;       /* context width */
  int             height;      /* context height */
  /* in a real implementation, this would hold platform-specific handles */
  /* (e.g., SDL_Renderer*, OpenGL context, etc.) */

};

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         set_pixel_internal()
	       ---
	       internal helper to set a pixel in a bitmap,
	       with bounds checking.

*/

static void set_pixel_internal(commc_bitmap_t* bitmap, int x, int y, commc_color_t color) {

  if  (!bitmap || !bitmap->pixels || x < 0 || y < 0 || x >= bitmap->width || y >= bitmap->height) {

    return;

  }

  size_t index = (y * bitmap->width + x) * 4; /* 4 bytes per pixel (RGBA) */
  
  bitmap->pixels[index + 0] = color.r;
  bitmap->pixels[index + 1] = color.g;
  bitmap->pixels[index + 2] = color.b;
  bitmap->pixels[index + 3] = color.a;

}

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_graphics_init()
	       ---
	       initializes the graphics context with a software framebuffer.
	       the 'title' parameter is conceptual in this software-only mode.

*/

commc_graphics_context_t* commc_graphics_init(int width, int height, const char* title) {

  commc_graphics_context_t* context;
  
  context = (commc_graphics_context_t*)malloc(sizeof(commc_graphics_context_t));

  if  (!context) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  context->framebuffer = commc_bitmap_create(width, height);

  if  (!context->framebuffer) {

    free(context);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  context->width  = width;
  context->height = height;

  /* conceptual: in a real app, this would create a window/display */
  if  (title) {
    /* printf("Graphics initialized: %s (%dx%d)\n", title, width, height); */
  }

  return context;

}

/*

         commc_graphics_shutdown()
	       ---
	       shuts down the graphics context and frees the framebuffer.

*/

void commc_graphics_shutdown(commc_graphics_context_t* context) {

  if  (context) {

    if  (context->framebuffer) {

      commc_bitmap_destroy(context->framebuffer);

    }

    free(context);

  }

}

/*

         commc_graphics_begin_frame()
	       ---
	       prepares the software renderer for a new frame.
	       (no-op for this basic software implementation)

*/

void commc_graphics_begin_frame(commc_graphics_context_t* context) {

  if  (!context) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

  }
  /* no-op for software renderer */

}

/*

         commc_graphics_end_frame()
	       ---
	       presents the rendered frame.
	       (no-op for this basic software implementation)

*/

void commc_graphics_end_frame(commc_graphics_context_t* context) {

  if  (!context) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

  }
  /* no-op for software renderer */

}

/*

         commc_graphics_clear()
	       ---
	       clears the framebuffer with a specified color.

*/

void commc_graphics_clear(commc_graphics_context_t* context, commc_color_t color) {

  int x, y;

  if  (!context || !context->framebuffer) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  for  (y = 0; y < context->height; y++) {

    for  (x = 0; x < context->width; x++) {

      set_pixel_internal(context->framebuffer, x, y, color);

    }

  }

}

/*

         commc_graphics_draw_pixel()
	       ---
	       draws a single pixel on the framebuffer.

*/

void commc_graphics_draw_pixel(commc_graphics_context_t* context, int x, int y, commc_color_t color) {

  if  (!context || !context->framebuffer) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  set_pixel_internal(context->framebuffer, x, y, color);

}

/*

         commc_graphics_draw_line()
	       ---
	       draws a line using Bresenham's line algorithm.

*/

void commc_graphics_draw_line(commc_graphics_context_t* context, int x1, int y1, int x2, int y2, commc_color_t color) {

  int dx, dy, sx, sy, err, e2;

  if  (!context || !context->framebuffer) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  dx = abs(x2 - x1);
  dy = abs(y2 - y1);
  sx = x1 < x2 ? 1 : -1;
  sy = y1 < y2 ? 1 : -1;
  err = (dx > dy ? dx : -dy) / 2;

  while  (1) {

    set_pixel_internal(context->framebuffer, x1, y1, color);

    if  (x1 == x2 && y1 == y2) {
      break;
    }

    e2 = err;

    if  (e2 > -dx) {
      err -= dy;
      x1 += sx;
    }
    if  (e2 < dy) {
      err += dx;
      y1 += sy;
    }

  }

}

/*

         commc_graphics_draw_rect()
	       ---
	       draws the outline of a rectangle.

*/

void commc_graphics_draw_rect(commc_graphics_context_t* context, commc_rect_t rect, commc_color_t color) {

  if  (!context || !context->framebuffer) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  /* top line */
  commc_graphics_draw_line(context, rect.x, rect.y, rect.x + rect.w - 1, rect.y, color);
  /* bottom line */
  commc_graphics_draw_line(context, rect.x, rect.y + rect.h - 1, rect.x + rect.w - 1, rect.y + rect.h - 1, color);
  /* left line */
  commc_graphics_draw_line(context, rect.x, rect.y, rect.x, rect.y + rect.h - 1, color);
  /* right line */
  commc_graphics_draw_line(context, rect.x + rect.w - 1, rect.y, rect.x + rect.w - 1, rect.y + rect.h - 1, color);

}

/*

         commc_graphics_fill_rect()
	       ---
	       fills a rectangle with a solid color.

*/

void commc_graphics_fill_rect(commc_graphics_context_t* context, commc_rect_t rect, commc_color_t color) {

  int x, y;

  if  (!context || !context->framebuffer) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  for  (y = rect.y; y < rect.y + rect.h; y++) {

    for  (x = rect.x; x < rect.x + rect.w; x++) {

      set_pixel_internal(context->framebuffer, x, y, color);

    }

  }

}

/*

         commc_graphics_draw_bitmap()
	       ---
	       draws a bitmap onto the framebuffer.

*/

void commc_graphics_draw_bitmap(commc_graphics_context_t* context, const commc_bitmap_t* bitmap, int x_offset, int y_offset) {

  int x, y;

  if  (!context || !context->framebuffer || !bitmap || !bitmap->pixels) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  for  (y = 0; y < bitmap->height; y++) {

    for  (x = 0; x < bitmap->width; x++) {

      commc_color_t color = commc_bitmap_get_pixel(bitmap, x, y);
      set_pixel_internal(context->framebuffer, x_offset + x, y_offset + y, color);

    }

  }

}

/*

         commc_bitmap_create()
	       ---
	       creates a new bitmap and allocates its pixel data.

*/

commc_bitmap_t* commc_bitmap_create(int width, int height) {

  commc_bitmap_t* bitmap;
  
  if  (width <= 0 || height <= 0) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  bitmap = (commc_bitmap_t*)malloc(sizeof(commc_bitmap_t));

  if  (!bitmap) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  bitmap->width  = width;
  bitmap->height = height;
  bitmap->pixels = (unsigned char*)malloc(width * height * 4); /* RGBA */

  if  (!bitmap->pixels) {

    free(bitmap);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  memset(bitmap->pixels, 0, width * height * 4); /* initialize to black transparent */

  return bitmap;

}

/*

         commc_bitmap_destroy()
	       ---
	       frees the pixel data and the bitmap structure.

*/

void commc_bitmap_destroy(commc_bitmap_t* bitmap) {

  if  (bitmap) {

    if  (bitmap->pixels) {

      free(bitmap->pixels);

    }

    free(bitmap);

  }

}

/*

         commc_bitmap_set_pixel()
	       ---
	       sets the color of a pixel in a bitmap.

*/

void commc_bitmap_set_pixel(commc_bitmap_t* bitmap, int x, int y, commc_color_t color) {

  set_pixel_internal(bitmap, x, y, color);

}

/*

         commc_bitmap_get_pixel()
	       ---
	       gets the color of a pixel from a bitmap.

*/

commc_color_t commc_bitmap_get_pixel(const commc_bitmap_t* bitmap, int x, int y) {

  commc_color_t default_color = {0, 0, 0, 0}; /* black transparent */

  if  (!bitmap || !bitmap->pixels || x < 0 || y < 0 || x >= bitmap->width || y >= bitmap->height) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return default_color;

  }

  size_t index = (y * bitmap->width + x) * 4;
  
  default_color.r = bitmap->pixels[index + 0];
  default_color.g = bitmap->pixels[index + 1];
  default_color.b = bitmap->pixels[index + 2];
  default_color.a = bitmap->pixels[index + 3];

  return default_color;

}

/*
	==================================
             --- EOF ---
	==================================
*/
