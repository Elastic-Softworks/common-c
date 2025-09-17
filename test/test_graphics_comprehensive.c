/*

    ===================================================

        COMMON-C TEST MODULE: GRAPHICS COMPREHENSIVE TESTS
           ---
           comprehensive test coverage for graphics.c
           following established methodology template

        Note: This module tests software-based 2D graphics
        rendering including context management, drawing
        primitives, and bitmap operations.

    ===================================================
    
    Public Functions Tested:
    
    - commc_graphics_init()
    - commc_graphics_shutdown()
    - commc_graphics_begin_frame()
    - commc_graphics_end_frame()
    - commc_graphics_clear()
    - commc_graphics_draw_pixel()
    - commc_graphics_draw_line()
    - commc_graphics_draw_rect()
    - commc_graphics_fill_rect()
    - commc_graphics_draw_bitmap()
    - commc_bitmap_create()
    - commc_bitmap_destroy()
    - commc_bitmap_set_pixel()
    - commc_bitmap_get_pixel()
    
*/

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>

#include    "test_harness.h"
#include    "../include/commc/graphics.h"
#include    "../include/commc/error.h"

/*
    ==================================
             TEST FUNCTIONS
    ==================================
*/

/*

    test_graphics_context_lifecycle()
    ---
    tests graphics context creation and shutdown

*/

void test_graphics_context_lifecycle(void) {

    commc_graphics_context_t* context;
    
    printf("\nOUTPUT: TESTING GRAPHICS CONTEXT LIFECYCLE...\n\n");
    
    /* test creating valid context */

    context = commc_graphics_init(800, 600, "Test Window");
    ASSERT_NOT_NULL(context);
    
    /* test shutting down context */

    commc_graphics_shutdown(context);
    
    /* test creating context with different dimensions */

    context = commc_graphics_init(1024, 768, "Another Test Window");
    ASSERT_NOT_NULL(context);
    commc_graphics_shutdown(context);
    
    /* test creating minimal size context */

    context = commc_graphics_init(1, 1, "Minimal");
    ASSERT_NOT_NULL(context);
    commc_graphics_shutdown(context);
    
    printf("\nOUTPUT: GRAPHICS CONTEXT LIFECYCLE SUCCESSFUL...!\n\n");

}

/*

    test_graphics_context_null_safety()
    ---
    tests graphics operations with null contexts

*/

void test_graphics_context_null_safety(void) {

    commc_color_t test_color = {255, 0, 0, 255}; /* red */
    commc_rect_t test_rect = {10, 10, 50, 30};
    
    printf("\nTESTING: GRAPHICS CONTEXT NULL SAFETY...\n\n");
    
    /* test invalid parameters during creation */

    ASSERT_NULL(commc_graphics_init(0, 600, "Test"));       /* zero width */
    ASSERT_NULL(commc_graphics_init(800, 0, "Test"));       /* zero height */
    ASSERT_NULL(commc_graphics_init(-100, 600, "Test"));    /* negative width */
    ASSERT_NULL(commc_graphics_init(800, -100, "Test"));    /* negative height */
    
    /* test operations with null context */

    commc_graphics_shutdown(NULL);                          /* should handle gracefully */
    commc_graphics_begin_frame(NULL);
    commc_graphics_end_frame(NULL);
    commc_graphics_clear(NULL, test_color);
    commc_graphics_draw_pixel(NULL, 10, 10, test_color);
    commc_graphics_draw_line(NULL, 0, 0, 10, 10, test_color);
    commc_graphics_draw_rect(NULL, test_rect, test_color);
    commc_graphics_fill_rect(NULL, test_rect, test_color);
    commc_graphics_draw_bitmap(NULL, NULL, 0, 0);
    
    printf("\nPASSED: GRAPHICS CONTEXT NULL SAFETY\n\n");

}

/*

    test_graphics_frame_operations()
    ---
    tests frame begin/end operations

*/

void test_graphics_frame_operations(void) {

    commc_graphics_context_t* context;
    
    printf("\nTESTING: GRAPHICS FRAME OPERATIONS...\n\n");
    
    context = commc_graphics_init(640, 480, "Frame Test");
    ASSERT_NOT_NULL(context);
    
    /* test frame operations */

    commc_graphics_begin_frame(context);
    commc_graphics_end_frame(context);
    
    /* test multiple frame cycles */

    commc_graphics_begin_frame(context);
    commc_graphics_end_frame(context);
    commc_graphics_begin_frame(context);
    commc_graphics_end_frame(context);
    
    commc_graphics_shutdown(context);
    
    printf("\nPASSED: GRAPHICS FRAME OPERATIONS\n\n");

}

/*

    test_graphics_clear_operations()
    ---
    tests screen clearing with different colors

*/

void test_graphics_clear_operations(void) {

    commc_graphics_context_t* context;
    commc_color_t black = {0, 0, 0, 255};
    commc_color_t white = {255, 255, 255, 255};
    commc_color_t red = {255, 0, 0, 255};
    commc_color_t transparent = {0, 0, 0, 0};
    
    printf("\nTESTING: GRAPHICS CLEAR OPERATIONS...\n\n");
    
    context = commc_graphics_init(320, 240, "Clear Test");
    ASSERT_NOT_NULL(context);
    
    commc_graphics_begin_frame(context);
    
    /* test clearing with different colors */

    commc_graphics_clear(context, black);
    commc_graphics_clear(context, white);
    commc_graphics_clear(context, red);
    commc_graphics_clear(context, transparent);
    
    commc_graphics_end_frame(context);
    commc_graphics_shutdown(context);
    
    printf("\nPASSED: GRAPHICS CLEAR OPERATIONS\n\n");

}

/*

    test_graphics_pixel_operations()
    ---
    tests individual pixel drawing

*/

void test_graphics_pixel_operations(void) {

    commc_graphics_context_t* context;
    commc_color_t colors[] = {
        {255, 0, 0, 255},    /* red */
        {0, 255, 0, 255},    /* green */
        {0, 0, 255, 255},    /* blue */
        {255, 255, 0, 255},  /* yellow */
        {128, 128, 128, 128} /* semi-transparent gray */
    };
    int i;
    
    printf("\nTESTING: GRAPHICS PIXEL OPERATIONS...\n\n");
    
    context = commc_graphics_init(100, 100, "Pixel Test");
    ASSERT_NOT_NULL(context);
    
    commc_graphics_begin_frame(context);
    
    /* test drawing pixels at various positions */

    for (i = 0; i < 5; i++) {
        commc_graphics_draw_pixel(context, i * 10, i * 10, colors[i]);
        commc_graphics_draw_pixel(context, 50 + i, 50 + i, colors[i]);
    }
    
    /* test edge cases */

    commc_graphics_draw_pixel(context, 0, 0, colors[0]);     /* top-left */
    commc_graphics_draw_pixel(context, 99, 99, colors[1]);   /* bottom-right */
    commc_graphics_draw_pixel(context, -1, -1, colors[2]);   /* out of bounds (should be ignored) */
    commc_graphics_draw_pixel(context, 100, 100, colors[3]); /* out of bounds (should be ignored) */
    
    commc_graphics_end_frame(context);
    commc_graphics_shutdown(context);
    
    printf("\nPASSED: GRAPHICS PIXEL OPERATIONS\n\n");

}

/*

    test_graphics_line_operations()
    ---
    tests line drawing primitives

*/

void test_graphics_line_operations(void) {

    commc_graphics_context_t* context;
    commc_color_t line_color = {100, 200, 50, 255};
    
    printf("\nTESTING: GRAPHICS LINE OPERATIONS...\n\n");
    
    context = commc_graphics_init(200, 200, "Line Test");
    ASSERT_NOT_NULL(context);
    
    commc_graphics_begin_frame(context);
    
    /* test drawing lines in different orientations */

    commc_graphics_draw_line(context, 10, 10, 50, 10, line_color);      /* horizontal */
    commc_graphics_draw_line(context, 10, 20, 10, 60, line_color);      /* vertical */
    commc_graphics_draw_line(context, 70, 10, 110, 50, line_color);     /* diagonal */
    commc_graphics_draw_line(context, 110, 10, 70, 50, line_color);     /* diagonal reverse */
    
    /* test single point line */

    commc_graphics_draw_line(context, 100, 100, 100, 100, line_color);
    
    /* test lines with negative coordinates or out of bounds */

    commc_graphics_draw_line(context, -10, -10, 20, 20, line_color);
    commc_graphics_draw_line(context, 180, 180, 250, 250, line_color);
    
    commc_graphics_end_frame(context);
    commc_graphics_shutdown(context);
    
    printf("\nPASSED: GRAPHICS LINE OPERATIONS\n\n");

}

/*

    test_graphics_rect_operations()
    ---
    tests rectangle outline and fill operations

*/

void test_graphics_rect_operations(void) {

    commc_graphics_context_t* context;
    commc_color_t outline_color = {255, 255, 0, 255};   /* yellow */
    commc_color_t fill_color = {0, 0, 255, 128};        /* semi-transparent blue */

    commc_rect_t rects[] = {

        {10, 10, 50, 30},           /* normal rectangle */
        {70, 10, 1, 1},             /* minimal rectangle */
        {100, 50, 80, 60},          /* large rectangle */
        {-10, -10, 30, 30},         /* partially out of bounds */
        {150, 150, 100, 100}        /* mostly out of bounds */

    };
    int i;
    
    printf("\nTESTING: GRAPHICS RECT OPERATIONS...\n\n");
    
    context = commc_graphics_init(300, 200, "Rect Test");
    ASSERT_NOT_NULL(context);
    
    commc_graphics_begin_frame(context);
    
    /* test drawing rectangle outlines */

    for (i = 0; i < 5; i++) {
        commc_graphics_draw_rect(context, rects[i], outline_color);
    }
    
    /* test filling rectangles */

    for (i = 0; i < 3; i++) {

        commc_rect_t fill_rect = rects[i];
        fill_rect.y += 80; /* offset to avoid overlap */
        commc_graphics_fill_rect(context, fill_rect, fill_color);

    }
    
    /* test zero-size rectangles */

    {

        commc_rect_t zero_rect = {50, 50, 0, 0};
        commc_graphics_draw_rect(context, zero_rect, outline_color);
        commc_graphics_fill_rect(context, zero_rect, fill_color);

    }
    
    commc_graphics_end_frame(context);
    commc_graphics_shutdown(context);
    
    printf("\nPASSED: GRAPHICS RECT OPERATIONS\n\n");

}

/*

    test_bitmap_lifecycle()
    ---
    tests bitmap creation and destruction

*/

void test_bitmap_lifecycle(void) {

    commc_bitmap_t* bitmap;
    
    printf("\nTESTING: BITMAP LIFECYCLE...\n\n");
    
    /* test creating various sized bitmaps */

    bitmap = commc_bitmap_create(100, 100);
    ASSERT_NOT_NULL(bitmap);
    ASSERT_TRUE(bitmap->width == 100);
    ASSERT_TRUE(bitmap->height == 100);
    ASSERT_NOT_NULL(bitmap->pixels);
    commc_bitmap_destroy(bitmap);
    
    bitmap = commc_bitmap_create(1, 1);
    ASSERT_NOT_NULL(bitmap);
    ASSERT_TRUE(bitmap->width == 1);
    ASSERT_TRUE(bitmap->height == 1);
    commc_bitmap_destroy(bitmap);
    
    bitmap = commc_bitmap_create(256, 256);
    ASSERT_NOT_NULL(bitmap);
    ASSERT_TRUE(bitmap->width == 256);
    ASSERT_TRUE(bitmap->height == 256);
    commc_bitmap_destroy(bitmap);
    
    printf("\nPASSED: BITMAP LIFECYCLE\n\n");

}

/*

    test_bitmap_null_safety()
    ---
    tests bitmap operations with invalid parameters

*/

void test_bitmap_null_safety(void) {

    commc_color_t test_color = {255, 0, 0, 255};
    
    printf("\nTESTING: BITMAP NULL SAFETY...\n\n");
    
    /* test creating invalid bitmaps */

    ASSERT_NULL(commc_bitmap_create(0, 100));    /* zero width */
    ASSERT_NULL(commc_bitmap_create(100, 0));    /* zero height */
    ASSERT_NULL(commc_bitmap_create(-10, 100));  /* negative width */
    ASSERT_NULL(commc_bitmap_create(100, -10));  /* negative height */
    
    /* test operations on null bitmap */

    commc_bitmap_destroy(NULL);                         /* should handle gracefully */
    commc_bitmap_set_pixel(NULL, 10, 10, test_color);
    commc_bitmap_get_pixel(NULL, 10, 10);               /* should return some default values */
    
    printf("\nPASSED: BITMAP NULL SAFETY\n\n");

}

/*

    test_bitmap_pixel_operations()
    ---
    tests setting and getting pixels in bitmaps

*/

void test_bitmap_pixel_operations(void) {

    commc_bitmap_t* bitmap;

    commc_color_t colors[] = {

        {255, 0, 0, 255},       /* red */
        {0, 255, 0, 255},       /* green */
        {0, 0, 255, 255},       /* blue */
        {255, 255, 255, 0}      /* transparent white */

    };
    commc_color_t retrieved_color;
    int i;
    
    printf("\nTESTING: BITMAP PIXEL OPERATIONS...\n\n");
    
    bitmap = commc_bitmap_create(50, 50);
    ASSERT_NOT_NULL(bitmap);
    
    /* test setting and getting individual pixels */

    for (i = 0; i < 4; i++) {

        commc_bitmap_set_pixel(bitmap, i * 10, i * 10, colors[i]);
        retrieved_color = commc_bitmap_get_pixel(bitmap, i * 10, i * 10);
        
        ASSERT_TRUE(retrieved_color.r == colors[i].r);
        ASSERT_TRUE(retrieved_color.g == colors[i].g);
        ASSERT_TRUE(retrieved_color.b == colors[i].b);
        ASSERT_TRUE(retrieved_color.a == colors[i].a);

    }
    
    /* test edge pixels */

    commc_bitmap_set_pixel(bitmap, 0, 0, colors[0]);           /* top-left */
    commc_bitmap_set_pixel(bitmap, 49, 49, colors[1]);         /* bottom-right */
    
    retrieved_color = commc_bitmap_get_pixel(bitmap, 0, 0);
    ASSERT_TRUE(retrieved_color.r == colors[0].r);
    
    retrieved_color = commc_bitmap_get_pixel(bitmap, 49, 49);
    ASSERT_TRUE(retrieved_color.g == colors[1].g);
    
    /* test out of bounds operations (should be ignored/return default) */

    commc_bitmap_set_pixel(bitmap, -1, -1, colors[2]);
    commc_bitmap_set_pixel(bitmap, 50, 50, colors[3]);
    retrieved_color = commc_bitmap_get_pixel(bitmap, -1, -1);
    retrieved_color = commc_bitmap_get_pixel(bitmap, 50, 50);
    
    commc_bitmap_destroy(bitmap);
    
    printf("\nPASSED: BITMAP PIXEL OPERATIONS\n\n");

}

/*

    test_bitmap_drawing()
    ---
    tests drawing bitmaps to graphics context

*/

void test_bitmap_drawing(void) {

    commc_graphics_context_t* context;
    commc_bitmap_t* bitmap;

    commc_color_t pattern_colors[] = {

        {255, 0, 0, 255},    /* red */
        {0, 255, 0, 255},    /* green */
        {0, 0, 255, 255},    /* blue */
        {255, 255, 0, 255}   /* yellow */

    };

    int x, y;
    
    printf("\nTESTING: BITMAP DRAWING...\n\n");
    
    context = commc_graphics_init(200, 200, "Bitmap Draw Test");
    ASSERT_NOT_NULL(context);
    
    /* create a small test bitmap with a pattern */

    bitmap = commc_bitmap_create(10, 10);
    ASSERT_NOT_NULL(bitmap);
    
    /* fill bitmap with a simple pattern */

    for (y = 0; y < 10; y++) {

        for (x = 0; x < 10; x++) {

            int color_index = (x + y) % 4;
            commc_bitmap_set_pixel(bitmap, x, y, pattern_colors[color_index]);

        }
    }
    
    commc_graphics_begin_frame(context);
    
    /* test drawing bitmap at various positions */

    commc_graphics_draw_bitmap(context, bitmap, 10, 10);     /* normal position */
    commc_graphics_draw_bitmap(context, bitmap, 0, 0);       /* top-left corner */
    commc_graphics_draw_bitmap(context, bitmap, 190, 190);   /* bottom-right (partially clipped) */
    commc_graphics_draw_bitmap(context, bitmap, -5, -5);     /* negative position (partially clipped) */
    commc_graphics_draw_bitmap(context, bitmap, 250, 250);   /* completely out of bounds */
    
    /* test drawing null bitmap (should be handled gracefully) */

    commc_graphics_draw_bitmap(context, NULL, 50, 50);
    
    commc_graphics_end_frame(context);
    
    commc_bitmap_destroy(bitmap);
    commc_graphics_shutdown(context);
    
    printf("\nPASSED: BITMAP DRAWING\n\n");

}

/*

    test_complex_graphics_scenario()
    ---
    tests a complex graphics scenario combining multiple operations

*/

void test_complex_graphics_scenario(void) {

    commc_graphics_context_t* context;
    commc_bitmap_t* sprite;
    commc_color_t bg_color = {50, 50, 100, 255};      /* dark blue background */
    commc_color_t line_color = {255, 255, 255, 255};  /* white lines */
    commc_color_t rect_color = {255, 100, 100, 255};  /* light red rectangles */
    commc_color_t fill_color = {100, 255, 100, 128};  /* semi-transparent green */
    commc_color_t sprite_color = {255, 255, 0, 255};  /* yellow sprite */

    commc_rect_t ui_elements[] = {

        {10, 10, 100, 30},    /* header bar */
        {10, 50, 50, 50},     /* button 1 */
        {70, 50, 50, 50},     /* button 2 */
        {10, 110, 110, 80}    /* content area */

    };

    int i, x, y;
    
    printf("\nTESTING: COMPLEX GRAPHICS SCENARIO...\n\n");
    
    /* create a game-like graphics context */

    context = commc_graphics_init(400, 300, "Complex Graphics Test");
    ASSERT_NOT_NULL(context);
    
    /* create a small sprite */

    sprite = commc_bitmap_create(16, 16);
    ASSERT_NOT_NULL(sprite);
    
    /* fill sprite with a simple pattern (yellow cross) */

    for (y = 0; y < 16; y++) {

        for (x = 0; x < 16; x++) {

            if  (x == 8 || y == 8) {

                commc_bitmap_set_pixel(sprite, x, y, sprite_color);

            }
        }
    }
    
    commc_graphics_begin_frame(context);
    
    /* clear with background color */

    commc_graphics_clear(context, bg_color);
    
    /* draw UI framework */

    for (i = 0; i < 4; i++) {

        commc_graphics_draw_rect(context, ui_elements[i], rect_color);

        if  (i >= 1 && i <= 2) {        /* fill buttons */

            commc_graphics_fill_rect(context, ui_elements[i], fill_color);

        }
    }
    
    /* draw connecting lines between UI elements */

    commc_graphics_draw_line(context, 35, 40, 35, 50, line_color);      /* header to button 1 */
    commc_graphics_draw_line(context, 95, 40, 95, 50, line_color);      /* header to button 2 */
    commc_graphics_draw_line(context, 60, 100, 60, 110, line_color);    /* buttons to content */
    
    /* draw grid pattern in content area */

    for (i = 0; i < 5; i++) {

        int grid_x = 20 + i * 20;
        int grid_y = 120 + i * 15;

        commc_graphics_draw_line(context, grid_x, 120, grid_x, 180, line_color);
        commc_graphics_draw_line(context, 20, grid_y, 100, grid_y, line_color);

    }
    
    /* draw sprites at various positions */

    commc_graphics_draw_bitmap(context, sprite, 200, 50);       /* main sprite */
    commc_graphics_draw_bitmap(context, sprite, 250, 100);      /* second sprite */
    commc_graphics_draw_bitmap(context, sprite, 300, 150);      /* third sprite */
    
    /* draw decorative pixels around sprites */

    for (i = 0; i < 20; i++) {

        int px = 200 + (i * 7) % 100;
        int py = 200 + (i * 11) % 50;
        commc_color_t pixel_color;
        pixel_color.r = (unsigned char)(i * 10);
        pixel_color.g = (unsigned char)(255 - i * 10);
        pixel_color.b = (unsigned char)(128 + i * 5);
        pixel_color.a = 255;
        commc_graphics_draw_pixel(context, px, py, pixel_color);

    }
    
    commc_graphics_end_frame(context);
    
    /* cleanup */

    commc_bitmap_destroy(sprite);
    commc_graphics_shutdown(context);
    
    printf("\nPASSED: COMPLEX GRAPHICS SCENARIO\n\n");

}

/*

    test_large_bitmap_operations()
    ---
    tests operations with larger bitmaps

*/

void test_large_bitmap_operations(void) {

    commc_bitmap_t* large_bitmap;

    commc_color_t test_colors[] = {

        {255, 0, 0, 255},    /* red */
        {0, 255, 0, 255},    /* green */
        {0, 0, 255, 255}     /* blue */

    };

    commc_color_t retrieved_color;
    int test_positions[][2] = {{0, 0}, {127, 127}, {255, 255}, {100, 200}, {200, 100}};
    int i;
    
    printf("\nTESTING: LARGE BITMAP OPERATIONS...\n\n");
    
    /* create a reasonably large bitmap */

    large_bitmap = commc_bitmap_create(256, 256);
    ASSERT_NOT_NULL(large_bitmap);
    ASSERT_TRUE(large_bitmap->width == 256);
    ASSERT_TRUE(large_bitmap->height == 256);
    ASSERT_NOT_NULL(large_bitmap->pixels);
    
    /* test setting pixels at various positions */

    for (i = 0; i < 5; i++) {

        int x = test_positions[i][0];
        int y = test_positions[i][1];
        commc_color_t color = test_colors[i % 3];
        
        commc_bitmap_set_pixel(large_bitmap, x, y, color);
        retrieved_color = commc_bitmap_get_pixel(large_bitmap, x, y);
        
        ASSERT_TRUE(retrieved_color.r == color.r);
        ASSERT_TRUE(retrieved_color.g == color.g);
        ASSERT_TRUE(retrieved_color.b == color.b);
        ASSERT_TRUE(retrieved_color.a == color.a);

    }
    
    commc_bitmap_destroy(large_bitmap);
    
    printf("\nPASSED: LARGE BITMAP OPERATIONS\n\n");

}

/*
    ==================================
             MAIN TEST RUNNER
    ==================================
*/

int main(void) {

    printf("\n");
    printf("========================================\n");
    printf("  COMMON-C GRAPHICS COMPREHENSIVE TESTS\n");
    printf("========================================\n\n");

    MEMORY_LEAK_CHECK_START();

    /* graphics context operations */

    ADD_TEST(graphics_context_lifecycle);
    ADD_TEST(graphics_context_null_safety);

    /* frame operations */

    ADD_TEST(graphics_frame_operations);

    /* drawing operations */

    ADD_TEST(graphics_clear_operations);
    ADD_TEST(graphics_pixel_operations);
    ADD_TEST(graphics_line_operations);
    ADD_TEST(graphics_rect_operations);

    /* bitmap operations */

    ADD_TEST(bitmap_lifecycle);
    ADD_TEST(bitmap_null_safety);
    ADD_TEST(bitmap_pixel_operations);
    ADD_TEST(bitmap_drawing);

    /* performance and integration tests */
    
    ADD_TEST(large_bitmap_operations);
    ADD_TEST(complex_graphics_scenario);

    RUN_ALL_TESTS();

    MEMORY_LEAK_CHECK_END();

    printf("========================================\n");
    printf("  ALL GRAPHICS TESTS COMPLETED\n");
    printf("========================================\n\n");

    return 0;

}

/*
    ==================================
             --- EOF ---
    ==================================
*/