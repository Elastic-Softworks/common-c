/* 	
   ===================================
   C O M M C / O C T R E E . H
   OCTREE 3D SPATIAL PARTITIONING
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- OCTREE MODULE ---

	    this module provides a complete octree implementation for
	    3D spatial partitioning and geometric queries. an octree
	    recursively subdivides 3D space into eight octants,
	    creating a hierarchical structure optimized for spatial
	    indexing, collision detection, and range searching.
	    
	    the implementation supports point insertion, range queries
	    using 3D bounding boxes, spherical queries, nearest neighbor
	    search, and various utility functions. like its 2D counterpart
	    (quadtree), the octree automatically subdivides nodes when
	    they exceed a configurable point capacity.
	    
	    spatial data structures are fundamental in computer graphics,
	    game engines, geographic information systems, and scientific
	    simulations where efficient 3D point location and range
	    searching are critical for performance.

*/

#ifndef COMMC_OCTREE_H
#define COMMC_OCTREE_H

#include "error.h"
#include "list.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
	==================================
             --- TYPES ---
	==================================
*/

/*

         commc_point3d_t
	       ---
	       represents a point in 3D space with x, y, and z coordinates.
	       used as the fundamental data element stored in octree nodes.

*/

typedef struct {

  double x;         /* X-coordinate */
  double y;         /* Y-coordinate */
  double z;         /* Z-coordinate */

} commc_point3d_t;

/*

         commc_bounding_box_t
	       ---
	       defines a 3D rectangular bounding box (axis-aligned bounding box).
	       used for octree node boundaries and range query specifications.

*/

typedef struct {

  double x;         /* minimum X coordinate */
  double y;         /* minimum Y coordinate */
  double z;         /* minimum Z coordinate */
  double width;     /* extent in X direction */
  double height;    /* extent in Y direction */
  double depth;     /* extent in Z direction */

} commc_bounding_box_t;

/*

         commc_octree_node_t
	       ---
	       internal structure representing a single octree node.
	       contains boundary information, point storage, and
	       references to eight child octants when subdivided.

*/

typedef struct commc_octree_node_t {

  commc_bounding_box_t       boundary;     /* 3D boundary of this node */
  commc_list_t*              points;       /* points stored in this node */
  struct commc_octree_node_t* nne;         /* north-northeast octant */
  struct commc_octree_node_t* nnw;         /* north-northwest octant */
  struct commc_octree_node_t* nse;         /* north-southeast octant */
  struct commc_octree_node_t* nsw;         /* north-southwest octant */
  struct commc_octree_node_t* sne;         /* south-northeast octant */
  struct commc_octree_node_t* snw;         /* south-northwest octant */
  struct commc_octree_node_t* sse;         /* south-southeast octant */
  struct commc_octree_node_t* ssw;         /* south-southwest octant */

} commc_octree_node_t;

/*

         commc_octree_t
	       ---
	       main octree structure containing root node and configuration.
	       manages the overall tree state and provides access to
	       octree operations and statistics.

*/

typedef struct {

  commc_octree_node_t* root;          /* root node of the octree */
  size_t               capacity;      /* max points per node before subdivision */
  size_t               total_points;  /* total points in the octree */
  size_t               max_depth;     /* maximum allowed depth */

} commc_octree_t;

/*

         commc_octree_query_callback_t
	       ---
	       callback function type for octree traversal operations.
	       used in range queries, spherical queries, and iteration.
	       return non-zero to stop traversal early.

*/

typedef int (*commc_octree_query_callback_t)(const commc_point3d_t* point, void* user_data);

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_octree_create()
	       ---
	       creates a new octree with specified 3D boundary and capacity.
	       
	       the boundary defines the total 3D space that the octree
	       will manage. points outside this boundary cannot be inserted.
	       capacity determines how many points a node can hold before
	       it subdivides into eight child octants.
	       
	       max_depth limits recursion to prevent excessive subdivision
	       with tightly clustered points. typical values range from
	       10-20 depending on data distribution and performance needs.

*/

commc_octree_t* commc_octree_create(commc_bounding_box_t boundary, 
                                    size_t capacity, 
                                    size_t max_depth);

/*

         commc_octree_destroy()
	       ---
	       destroys the octree and frees all associated memory.
	       recursively destroys all nodes and their point lists.

*/

void commc_octree_destroy(commc_octree_t* octree);

/*

         commc_octree_insert()
	       ---
	       inserts a 3D point into the octree at the appropriate location.
	       
	       automatically handles node subdivision when capacity is
	       exceeded. points are inserted into leaf nodes only.
	       when a leaf exceeds capacity, it creates eight child
	       octants and redistributes its points among them.

*/

commc_error_t commc_octree_insert(commc_octree_t* octree, commc_point3d_t point);

/*

         commc_octree_query_range()
	       ---
	       finds all points within the specified 3D bounding box.
	       
	       uses recursive traversal with spatial pruning - child
	       nodes that don't intersect the query box are skipped
	       entirely. for each matching point, the callback function
	       is invoked with the point and user data.

*/

commc_error_t commc_octree_query_range(commc_octree_t* octree,
                                       commc_bounding_box_t* range,
                                       commc_octree_query_callback_t callback,
                                       void* user_data);

/*

         commc_octree_query_sphere()
	       ---
	       finds all points within the specified spherical region.
	       
	       first performs a range query using the sphere's bounding
	       box, then filters results by exact distance calculation.
	       this two-phase approach optimizes performance by using
	       the octree's spatial structure for initial filtering.

*/

commc_error_t commc_octree_query_sphere(commc_octree_t* octree,
                                        double center_x, double center_y, double center_z,
                                        double radius,
                                        commc_octree_query_callback_t callback,
                                        void* user_data);

/*

         commc_octree_nearest_neighbor()
	       ---
	       finds the point closest to the specified 3D coordinates.
	       
	       uses a brute-force approach for educational clarity.
	       more sophisticated implementations could use branch-and-bound
	       with distance-based pruning for better performance on
	       large datasets.

*/

commc_point3d_t* commc_octree_nearest_neighbor(commc_octree_t* octree,
                                               double x, double y, double z,
                                               double* distance);

/*

         commc_octree_point_count()
	       ---
	       returns the total number of points stored in the octree.
	       this count is maintained incrementally during insertions
	       for O(1) access time.

*/

size_t commc_octree_point_count(commc_octree_t* octree);

/*

         commc_octree_depth()
	       ---
	       calculates the maximum depth of the octree.
	       useful for performance analysis and debugging.
	       depth increases with subdivision and clustered data.

*/

size_t commc_octree_depth(commc_octree_t* octree);

/*

         commc_octree_clear()
	       ---
	       removes all points from the octree and resets to initial state.
	       preserves the root node structure and configuration.

*/

void commc_octree_clear(commc_octree_t* octree);

/*

         commc_octree_contains_point()
	       ---
	       checks if the specified 3D coordinates fall within
	       the octree's boundary. returns 1 if contained, 0 otherwise.

*/

int commc_octree_contains_point(commc_octree_t* octree, double x, double y, double z);

/*

         commc_octree_get_boundary()
	       ---
	       retrieves the 3D bounding box that defines the octree's
	       total spatial extent. useful for visualization and debugging.

*/

commc_bounding_box_t commc_octree_get_boundary(commc_octree_t* octree);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_OCTREE_H */

/*
	==================================
             --- EOF ---
	==================================
*/