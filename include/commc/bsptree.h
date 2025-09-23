/* 	
   ===================================
   C O M M C / B S P _ T R E E . H
   BINARY SPACE PARTITIONING TREE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- BSP TREE MODULE ---

	    this module provides a complete Binary Space Partitioning (BSP)
	    tree implementation for advanced 3D spatial subdivision and
	    rendering applications. unlike octrees which use axis-aligned
	    subdivision, BSP trees use arbitrary planes to recursively
	    divide 3D space, creating a binary tree structure.
	    
	    BSP trees are fundamental in computer graphics for visibility
	    determination, back-to-front rendering, collision detection,
	    and spatial queries. they excel when the geometry has natural
	    planar boundaries or when arbitrary subdivision orientations
	    provide better spatial balance than axis-aligned approaches.
	    
	    the implementation supports polygon storage, point classification,
	    front-to-back and back-to-front traversal for rendering,
	    and various geometric queries. educational documentation
	    explains plane equations, point-plane relationships, and
	    the mathematical foundations of spatial partitioning.

*/

#ifndef COMMC_BSP_TREE_H
#define COMMC_BSP_TREE_H

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

         commc_plane_t
	       ---
	       represents a 3D plane using the equation ax + by + cz + d = 0.
	       the normal vector (a, b, c) defines the plane orientation,
	       and d is the distance from the origin along the normal.

*/

typedef struct {

  double a, b, c;  /* normal vector components */
  double d;        /* distance from origin */

} commc_plane_t;

/*

         commc_vertex_t
	       ---
	       represents a 3D vertex with position coordinates.
	       used as building blocks for polygons in the BSP tree.

*/

typedef struct {

  double x, y, z;  /* 3D position coordinates */

} commc_vertex_t;

/*

         commc_polygon_t
	       ---
	       represents a polygon with an array of vertices.
	       polygons are the geometric primitives stored in BSP tree nodes.

*/

typedef struct {

  commc_vertex_t* vertices;      /* array of polygon vertices */
  size_t          vertex_count;  /* number of vertices */
  void*           user_data;     /* optional user-associated data */

} commc_polygon_t;

/*

         commc_bsp_node_t
	       ---
	       internal structure representing a single BSP tree node.
	       contains the partitioning plane and references to
	       front and back child subtrees.

*/

typedef struct commc_bsp_node_t {

  commc_plane_t              plane;        /* partitioning plane */
  commc_list_t*              polygons;     /* polygons on this plane */
  struct commc_bsp_node_t*   front;        /* front child (positive side) */
  struct commc_bsp_node_t*   back;         /* back child (negative side) */

} commc_bsp_node_t;

/*

         commc_bsp_tree_t
	       ---
	       main BSP tree structure containing root node and statistics.
	       manages the overall tree state and provides access to
	       BSP tree operations and traversal methods.

*/

typedef struct {

  commc_bsp_node_t* root;           /* root node of the BSP tree */
  size_t            polygon_count;  /* total polygons in the tree */
  size_t            max_depth;      /* maximum allowed depth */

} commc_bsp_tree_t;

/*

         commc_point_classification_t
	       ---
	       enumeration for point-plane relationship classification.
	       determines which side of a plane a point lies on.

*/

typedef enum {

  COMMC_POINT_ON_PLANE = 0,    /* point lies exactly on the plane */
  COMMC_POINT_IN_FRONT = 1,    /* point is on the positive side */
  COMMC_POINT_BEHIND = -1      /* point is on the negative side */

} commc_point_classification_t;

/*

         commc_bsp_traversal_callback_t
	       ---
	       callback function type for BSP tree traversal operations.
	       used for rendering, collision detection, and custom processing.
	       return non-zero to stop traversal early.

*/

typedef int (*commc_bsp_traversal_callback_t)(const commc_polygon_t* polygon, void* user_data);

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_bsp_tree_create()
	       ---
	       creates a new empty BSP tree with specified maximum depth.
	       
	       max_depth limits recursion to prevent excessive subdivision.
	       typical values range from 10-20 depending on scene complexity
	       and performance requirements. deeper trees provide better
	       spatial resolution but increase traversal costs.

*/

commc_bsp_tree_t* commc_bsp_tree_create(size_t max_depth);

/*

         commc_bsp_tree_destroy()
	       ---
	       destroys the BSP tree and frees all associated memory.
	       recursively destroys all nodes and their polygon lists.
	       does not free user_data associated with polygons.

*/

void commc_bsp_tree_destroy(commc_bsp_tree_t* tree);

/*

         commc_bsp_tree_insert_polygon()
	       ---
	       inserts a polygon into the BSP tree at the appropriate location.
	       
	       the polygon is classified against existing partitioning planes
	       and may be split if it straddles multiple spatial regions.
	       the tree automatically selects partitioning planes based on
	       polygon orientation to minimize splitting.

*/

commc_error_t commc_bsp_tree_insert_polygon(commc_bsp_tree_t* tree, 
                                            const commc_polygon_t* polygon);

/*

         commc_bsp_tree_traverse_front_to_back()
	       ---
	       traverses the BSP tree from front to back relative to a viewpoint.
	       
	       this traversal order is essential for proper alpha blending
	       and transparency rendering in 3D graphics. polygons are
	       processed in order from nearest to farthest from the viewpoint.

*/

commc_error_t commc_bsp_tree_traverse_front_to_back(commc_bsp_tree_t* tree,
                                                     commc_vertex_t viewpoint,
                                                     commc_bsp_traversal_callback_t callback,
                                                     void* user_data);

/*

         commc_bsp_tree_traverse_back_to_front()
	       ---
	       traverses the BSP tree from back to front relative to a viewpoint.
	       
	       this traversal order is used for painter's algorithm rendering
	       where polygons are drawn in order from farthest to nearest,
	       allowing proper depth sorting without a Z-buffer.

*/

commc_error_t commc_bsp_tree_traverse_back_to_front(commc_bsp_tree_t* tree,
                                                     commc_vertex_t viewpoint,
                                                     commc_bsp_traversal_callback_t callback,
                                                     void* user_data);

/*

         commc_plane_classify_point()
	       ---
	       classifies a point's position relative to a plane.
	       
	       returns COMMC_POINT_IN_FRONT for positive side,
	       COMMC_POINT_BEHIND for negative side, or COMMC_POINT_ON_PLANE
	       for points lying exactly on the plane (within tolerance).

*/

commc_point_classification_t commc_plane_classify_point(const commc_plane_t* plane,
                                                        const commc_vertex_t* point);

/*

         commc_plane_from_vertices()
	       ---
	       constructs a plane equation from three non-collinear vertices.
	       
	       uses cross product to compute the normal vector and
	       calculates the distance parameter. the resulting plane
	       passes through all three vertices with consistent orientation.

*/

commc_error_t commc_plane_from_vertices(commc_plane_t* plane,
                                        const commc_vertex_t* v1,
                                        const commc_vertex_t* v2,
                                        const commc_vertex_t* v3);

/*

         commc_polygon_create()
	       ---
	       creates a new polygon with the specified vertices.
	       allocates memory for vertex array and copies vertex data.

*/

commc_polygon_t* commc_polygon_create(const commc_vertex_t* vertices,
                                      size_t vertex_count,
                                      void* user_data);

/*

         commc_polygon_destroy()
	       ---
	       destroys a polygon and frees its vertex memory.
	       does not free user_data - caller must handle separately.

*/

void commc_polygon_destroy(commc_polygon_t* polygon);

/*

         commc_polygon_split()
	       ---
	       splits a polygon by a plane into front and back portions.
	       
	       polygons that straddle the splitting plane are divided
	       along the intersection line. new vertices are created
	       at intersection points to maintain geometric accuracy.

*/

commc_error_t commc_polygon_split(const commc_polygon_t* polygon,
                                  const commc_plane_t* plane,
                                  commc_polygon_t** front_polygon,
                                  commc_polygon_t** back_polygon);

/*

         commc_bsp_tree_polygon_count()
	       ---
	       returns the total number of polygons stored in the BSP tree.
	       this count includes all original and split polygons.

*/

size_t commc_bsp_tree_polygon_count(commc_bsp_tree_t* tree);

/*

         commc_bsp_tree_depth()
	       ---
	       calculates the maximum depth of the BSP tree.
	       useful for performance analysis and tree balance evaluation.

*/

size_t commc_bsp_tree_depth(commc_bsp_tree_t* tree);

/*

         commc_bsp_tree_clear()
	       ---
	       removes all polygons from the BSP tree and resets to empty state.
	       preserves the tree structure for reuse with new geometry.

*/

void commc_bsp_tree_clear(commc_bsp_tree_t* tree);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_BSP_TREE_H */

/*
	==================================
             --- EOF ---
	==================================
*/