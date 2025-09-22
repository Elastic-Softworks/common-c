/*
   ===================================
   C O M M O N - C
   QUADTREE MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- QUADTREE MODULE ---

    this module provides a quadtree data structure for efficient
    2D spatial partitioning and querying. quadtrees recursively
    divide 2D space into four quadrants, creating a hierarchical
    spatial index ideal for collision detection, nearest neighbor
    searches, and spatial culling in graphics applications.

    fundamental quadtree concepts:
    - each node represents a rectangular region of 2D space
    - nodes are subdivided into four child quadrants (NE, NW, SE, SW)
    - points are stored in leaf nodes when subdivision limits are reached
    - spatial queries can be performed efficiently by traversing
      only relevant quadrants, achieving logarithmic performance

    applications include:
    - game engine spatial partitioning for collision detection
    - graphics culling for efficient rendering
    - spatial databases for geographic information systems
    - image compression and procedural generation
    - nearest neighbor searches in 2D datasets

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef  COMMC_QUADTREE_H
#define  COMMC_QUADTREE_H

#include  <stddef.h>
#include  "error.h"

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/*

         commc_point2d_t
	       ---
	       represents a 2D point with optional associated data.
	       the data pointer allows storing arbitrary user data
	       associated with each spatial location.

*/

typedef struct {

  double x;      /* x coordinate */
  double y;      /* y coordinate */
  void*  data;   /* optional user data associated with point */

} commc_point2d_t;

/*

         commc_rectangle_t
	       ---
	       defines a rectangular region in 2D space.
	       used for quadtree node boundaries and spatial queries.

*/

typedef struct {

  double x;       /* left boundary */
  double y;       /* bottom boundary */
  double width;   /* rectangle width */
  double height;  /* rectangle height */

} commc_rectangle_t;

/*

         commc_quadtree_t
	       ---
	       opaque quadtree structure. internal implementation
	       uses recursive node subdivision with configurable
	       capacity limits and maximum depth constraints.

*/

typedef struct commc_quadtree_t commc_quadtree_t;

/*

         commc_quadtree_query_callback_t
	       ---
	       callback function for spatial query operations.
	       called for each point found within query region.
	       return 0 to continue iteration, non-zero to stop.

*/

typedef int (*commc_quadtree_query_callback_t)(const commc_point2d_t* point, void* user_data);

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_quadtree_create()
	       ---
	       creates a quadtree covering the specified rectangular boundary.
	       capacity determines maximum points per node before subdivision.
	       max_depth limits recursion depth to prevent excessive subdivision.

*/

commc_quadtree_t* commc_quadtree_create(commc_rectangle_t boundary, 
                                        size_t capacity, 
                                        size_t max_depth);

/*

         commc_quadtree_destroy()
	       ---
	       frees all memory associated with the quadtree.
	       does not free user data pointed to by points.

*/

void commc_quadtree_destroy(commc_quadtree_t* quadtree);

/*

         commc_quadtree_insert()
	       ---
	       inserts a point into the quadtree. if the point falls
	       outside the quadtree boundary, insertion fails.
	       returns COMMC_SUCCESS on successful insertion.

*/

commc_error_t commc_quadtree_insert(commc_quadtree_t* quadtree, commc_point2d_t point);

/*

         commc_quadtree_query_range()
	       ---
	       finds all points within the specified rectangular region.
	       calls the callback function for each point found.
	       efficient implementation only traverses relevant quadrants.

*/

commc_error_t commc_quadtree_query_range(commc_quadtree_t* quadtree, 
                                         commc_rectangle_t range,
                                         commc_quadtree_query_callback_t callback, 
                                         void* user_data);

/*

         commc_quadtree_query_circle()
	       ---
	       finds all points within the specified circular region.
	       uses bounding box optimization followed by distance checking.
	       center_x, center_y define circle center, radius defines search area.

*/

commc_error_t commc_quadtree_query_circle(commc_quadtree_t* quadtree, 
                                          double center_x, double center_y, double radius,
                                          commc_quadtree_query_callback_t callback, 
                                          void* user_data);

/*

         commc_quadtree_nearest_neighbor()
	       ---
	       finds the single point closest to the specified coordinates.
	       returns the nearest point, or NULL if quadtree is empty.
	       distance pointer receives the distance to the nearest point.

*/

commc_point2d_t* commc_quadtree_nearest_neighbor(commc_quadtree_t* quadtree, 
                                                 double x, double y, 
                                                 double* distance);

/*

         commc_quadtree_point_count()
	       ---
	       returns the total number of points stored in the quadtree.
	       useful for performance analysis and memory management.

*/

size_t commc_quadtree_point_count(commc_quadtree_t* quadtree);

/*

         commc_quadtree_depth()
	       ---
	       returns the maximum depth of subdivision in the quadtree.
	       indicates the tree's structural complexity and performance characteristics.

*/

size_t commc_quadtree_depth(commc_quadtree_t* quadtree);

/*

         commc_quadtree_clear()
	       ---
	       removes all points from the quadtree, resetting it to empty state.
	       maintains the original boundary and capacity settings.

*/

void commc_quadtree_clear(commc_quadtree_t* quadtree);

/*

         commc_quadtree_contains_point()
	       ---
	       checks if the specified coordinates fall within the quadtree boundary.
	       returns 1 if point is within bounds, 0 otherwise.

*/

int commc_quadtree_contains_point(commc_quadtree_t* quadtree, double x, double y);

/*

         commc_quadtree_get_boundary()
	       ---
	       retrieves the rectangular boundary of the quadtree.
	       useful for spatial query optimization and visualization.

*/

commc_rectangle_t commc_quadtree_get_boundary(commc_quadtree_t* quadtree);

#endif /* COMMC_QUADTREE_H */

/*
	==================================
             --- EOF ---
	==================================
*/