/* 	
   ===================================
   C O M M C / B S P _ T R E E . C
   BINARY SPACE PARTITIONING TREE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- BSP TREE IMPLEMENTATION ---

	    this implementation provides a complete Binary Space Partitioning
	    tree system for advanced 3D spatial subdivision. BSP trees use
	    arbitrary planes to recursively divide 3D space, creating optimal
	    spatial organization for rendering, visibility determination,
	    and collision detection applications.
	    
	    the implementation handles automatic polygon splitting, plane
	    selection heuristics, and depth-ordered traversal for proper
	    rendering without Z-buffer requirements.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/bsp_tree.h"  /* BSP TREE API DECLARATIONS */
#include "commc/error.h"     /* ERROR HANDLING */
#include "commc/list.h"      /* DYNAMIC LISTS */
#include <math.h>            /* MATHEMATICAL FUNCTIONS */
#include <stdlib.h>          /* STANDARD LIBRARY FUNCTIONS */

/* 
	==================================
             --- CONSTANTS ---
	==================================
*/

#define COMMC_BSP_EPSILON 1e-9  /* tolerance for plane distance calculations */

/* 
	==================================
             --- HELPERS ---
	==================================
*/

/*

         create_node()
	       ---
	       creates a new BSP tree node with specified partitioning plane.
	       initializes polygon list and sets child pointers to null.

*/

static commc_bsp_node_t* create_node(commc_plane_t plane) {

  commc_bsp_node_t* node;
  
  node = (commc_bsp_node_t*)malloc(sizeof(commc_bsp_node_t));
  
  if  (!node) {
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  node->plane = plane;
  node->polygons = commc_list_create();
  node->front = NULL;
  node->back = NULL;
  
  if  (!node->polygons) {
    free(node);
    return NULL;
  }

  return node;

}

/*

         destroy_node()
	       ---
	       recursively destroys a BSP tree node and all its children.
	       frees all polygon storage and node memory.

*/

static void destroy_node(commc_bsp_node_t* node) {

  commc_polygon_t* polygon;

  if  (!node) {
    return;
  }

  /* free all polygons in this node */
  while  (commc_list_size(node->polygons) > 0) {

    polygon = (commc_polygon_t*)commc_list_front(node->polygons);

    if  (polygon) {
      commc_polygon_destroy(polygon);
    }

    commc_list_pop_front(node->polygons);

  }

  commc_list_destroy(node->polygons);

  /* recursively destroy children */
  destroy_node(node->front);
  destroy_node(node->back);

  free(node);

}

/*

         calculate_plane_distance()
	       ---
	       calculates the signed distance from a point to a plane.
	       positive values indicate the point is on the front side,
	       negative values indicate the back side.

*/

static double calculate_plane_distance(const commc_plane_t* plane, const commc_vertex_t* point) {

  return plane->a * point->x + plane->b * point->y + plane->c * point->z + plane->d;

}

/*

         normalize_plane()
	       ---
	       normalizes a plane equation so that the normal vector
	       has unit length. ensures consistent distance calculations.

*/

static void normalize_plane(commc_plane_t* plane) {

  double length = sqrt(plane->a * plane->a + plane->b * plane->b + plane->c * plane->c);
  
  if  (length > COMMC_BSP_EPSILON) {
    plane->a /= length;
    plane->b /= length;
    plane->c /= length;
    plane->d /= length;
  }

}

/*

         select_splitting_plane() - UNUSED IN EDUCATIONAL VERSION
	       ---
	       selects the best plane for splitting a set of polygons.
	       uses heuristics to minimize polygon splitting and balance
	       the resulting tree structure.

*/

#if 0  /* disabled for educational implementation */

static commc_error_t select_splitting_plane(commc_list_t* polygons, commc_plane_t* best_plane) {

  commc_list_iterator_t iterator;
  commc_polygon_t*      polygon;
  commc_plane_t         candidate_plane;
  int                   best_score = -1;
  int                   current_score;
  int                   found_plane = 0;

  if  (!polygons || commc_list_size(polygons) == 0) {
    return COMMC_ARGUMENT_ERROR;
  }

  /* try each polygon as a potential splitting plane */
  iterator = commc_list_begin(polygons);

  while  (commc_list_iterator_data(&iterator) != NULL) {

    polygon = (commc_polygon_t*)commc_list_iterator_data(&iterator);

    if  (polygon && polygon->vertex_count >= 3) {

      /* create plane from first three vertices */
      if  (commc_plane_from_vertices(&candidate_plane,
                                     &polygon->vertices[0],
                                     &polygon->vertices[1], 
                                     &polygon->vertices[2]) == COMMC_SUCCESS) {

        /* simple scoring heuristic - prefer planes that balance tree */
        current_score = (int)commc_list_size(polygons);
        
        if  (current_score > best_score) {
          *best_plane = candidate_plane;
          best_score = current_score;
          found_plane = 1;
        }

      }

    }

    commc_list_next(&iterator);

  }

  return found_plane ? COMMC_SUCCESS : COMMC_ARGUMENT_ERROR;

}

#endif

/*

         insert_polygon_into_node()
	       ---
	       recursively inserts a polygon into the appropriate BSP tree node.
	       handles polygon splitting when the polygon straddles the plane.

*/

static commc_error_t insert_polygon_into_node(commc_bsp_node_t* node,
                                              const commc_polygon_t* polygon,
                                              size_t current_depth,
                                              size_t max_depth) {

  commc_point_classification_t classification;
  commc_polygon_t*             front_poly = NULL;
  commc_polygon_t*             back_poly = NULL;
  commc_polygon_t*             polygon_copy;
  commc_error_t                result;
  size_t                       i;
  int                          front_count = 0;
  int                          back_count = 0;
  int                          on_count = 0;

  if  (!node || !polygon) {
    return COMMC_ARGUMENT_ERROR;
  }

  /* classify all vertices of the polygon against this node's plane */
  for  (i = 0; i < polygon->vertex_count; i++) {

    classification = commc_plane_classify_point(&node->plane, &polygon->vertices[i]);

    if  (classification == COMMC_POINT_IN_FRONT) {
      front_count++;
    } else if  (classification == COMMC_POINT_BEHIND) {
      back_count++;
    } else {
      on_count++;
    }

  }

  /* if all vertices are on the plane, store polygon here */
  if  (front_count == 0 && back_count == 0) {

    polygon_copy = commc_polygon_create(polygon->vertices, polygon->vertex_count, polygon->user_data);
    
    if  (!polygon_copy) {
      return COMMC_MEMORY_ERROR;
    }

    commc_list_push_back(node->polygons, polygon_copy);
    return COMMC_SUCCESS;

  }

  /* if polygon is entirely on front side */
  if  (back_count == 0) {

    if  (!node->front) {

      if  (current_depth >= max_depth) {
        /* store in current node if max depth reached */
        polygon_copy = commc_polygon_create(polygon->vertices, polygon->vertex_count, polygon->user_data);
        if  (!polygon_copy) return COMMC_MEMORY_ERROR;
        commc_list_push_back(node->polygons, polygon_copy);
        return COMMC_SUCCESS;
      }

      /* create front child with a default plane */
      {
        commc_plane_t front_plane = {1.0, 0.0, 0.0, 0.0};
        node->front = create_node(front_plane);
      }
      
      if  (!node->front) {
        return COMMC_MEMORY_ERROR;
      }

    }

    return insert_polygon_into_node(node->front, polygon, current_depth + 1, max_depth);

  }

  /* if polygon is entirely on back side */
  if  (front_count == 0) {

    if  (!node->back) {

      if  (current_depth >= max_depth) {
        /* store in current node if max depth reached */
        polygon_copy = commc_polygon_create(polygon->vertices, polygon->vertex_count, polygon->user_data);
        if  (!polygon_copy) return COMMC_MEMORY_ERROR;
        commc_list_push_back(node->polygons, polygon_copy);
        return COMMC_SUCCESS;
      }

      /* create back child with a default plane */
      {
        commc_plane_t back_plane = {-1.0, 0.0, 0.0, 0.0};
        node->back = create_node(back_plane);
      }
      
      if  (!node->back) {
        return COMMC_MEMORY_ERROR;
      }

    }

    return insert_polygon_into_node(node->back, polygon, current_depth + 1, max_depth);

  }

  /* polygon straddles the plane - split it */
  result = commc_polygon_split(polygon, &node->plane, &front_poly, &back_poly);

  if  (result != COMMC_SUCCESS) {
    return result;
  }

  /* insert split polygons into appropriate subtrees */
  if  (front_poly) {

    if  (!node->front) {
    commc_plane_t front_plane = {1.0, 0.0, 0.0, 0.0};
    node->front = create_node(front_plane);
  }

    if  (node->front) {
      insert_polygon_into_node(node->front, front_poly, current_depth + 1, max_depth);
    }

    commc_polygon_destroy(front_poly);

  }

  if  (back_poly) {

    if  (!node->back) {
    commc_plane_t back_plane = {-1.0, 0.0, 0.0, 0.0};
    node->back = create_node(back_plane);
  }

    if  (node->back) {
      insert_polygon_into_node(node->back, back_poly, current_depth + 1, max_depth);
    }

    commc_polygon_destroy(back_poly);

  }

  return COMMC_SUCCESS;

}

/*

         traverse_node_front_to_back()
	       ---
	       recursively traverses BSP tree from front to back relative to viewpoint.

*/

static commc_error_t traverse_node_front_to_back(commc_bsp_node_t* node,
                                                  commc_vertex_t viewpoint,
                                                  commc_bsp_traversal_callback_t callback,
                                                  void* user_data) {

  commc_list_iterator_t iterator;
  commc_polygon_t*      polygon;
  int                   result;

  if  (!node) {
    return COMMC_SUCCESS;
  }

  /* determine which side of the plane the viewpoint is on */
  if  (calculate_plane_distance(&node->plane, &viewpoint) >= 0) {

    /* viewpoint is on front side - traverse back first, then front */
    traverse_node_front_to_back(node->back, viewpoint, callback, user_data);

    /* process polygons on this plane */
    iterator = commc_list_begin(node->polygons);
    
    while  (commc_list_iterator_data(&iterator) != NULL) {

      polygon = (commc_polygon_t*)commc_list_iterator_data(&iterator);
      
      if  (polygon) {
        result = callback(polygon, user_data);
        if  (result != 0) return COMMC_SUCCESS;
      }

      commc_list_next(&iterator);

    }

    traverse_node_front_to_back(node->front, viewpoint, callback, user_data);

  } else {

    /* viewpoint is on back side - traverse front first, then back */
    traverse_node_front_to_back(node->front, viewpoint, callback, user_data);

    /* process polygons on this plane */
    iterator = commc_list_begin(node->polygons);
    
    while  (commc_list_iterator_data(&iterator) != NULL) {

      polygon = (commc_polygon_t*)commc_list_iterator_data(&iterator);
      
      if  (polygon) {
        result = callback(polygon, user_data);
        if  (result != 0) return COMMC_SUCCESS;
      }

      commc_list_next(&iterator);

    }

    traverse_node_front_to_back(node->back, viewpoint, callback, user_data);

  }

  return COMMC_SUCCESS;

}

/*

         traverse_node_back_to_front()
	       ---
	       recursively traverses BSP tree from back to front relative to viewpoint.

*/

static commc_error_t traverse_node_back_to_front(commc_bsp_node_t* node,
                                                  commc_vertex_t viewpoint,
                                                  commc_bsp_traversal_callback_t callback,
                                                  void* user_data) {

  commc_list_iterator_t iterator;
  commc_polygon_t*      polygon;
  int                   result;

  if  (!node) {
    return COMMC_SUCCESS;
  }

  /* determine which side of the plane the viewpoint is on */
  if  (calculate_plane_distance(&node->plane, &viewpoint) >= 0) {

    /* viewpoint is on front side - traverse front first, then back */
    traverse_node_back_to_front(node->front, viewpoint, callback, user_data);

    /* process polygons on this plane */
    iterator = commc_list_begin(node->polygons);
    
    while  (commc_list_iterator_data(&iterator) != NULL) {

      polygon = (commc_polygon_t*)commc_list_iterator_data(&iterator);
      
      if  (polygon) {
        result = callback(polygon, user_data);
        if  (result != 0) return COMMC_SUCCESS;
      }

      commc_list_next(&iterator);

    }

    traverse_node_back_to_front(node->back, viewpoint, callback, user_data);

  } else {

    /* viewpoint is on back side - traverse back first, then front */
    traverse_node_back_to_front(node->back, viewpoint, callback, user_data);

    /* process polygons on this plane */
    iterator = commc_list_begin(node->polygons);
    
    while  (commc_list_iterator_data(&iterator) != NULL) {

      polygon = (commc_polygon_t*)commc_list_iterator_data(&iterator);
      
      if  (polygon) {
        result = callback(polygon, user_data);
        if  (result != 0) return COMMC_SUCCESS;
      }

      commc_list_next(&iterator);

    }

    traverse_node_back_to_front(node->front, viewpoint, callback, user_data);

  }

  return COMMC_SUCCESS;

}

/*

         get_max_depth()
	       ---
	       recursively finds the maximum depth in the BSP tree.

*/

static size_t get_max_depth(commc_bsp_node_t* node) {

  size_t front_depth, back_depth;

  if  (!node) {
    return 0;
  }

  front_depth = get_max_depth(node->front);
  back_depth = get_max_depth(node->back);

  return 1 + (front_depth > back_depth ? front_depth : back_depth);

}

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_bsp_tree_create()
	       ---
	       creates a new empty BSP tree with specified maximum depth.

*/

commc_bsp_tree_t* commc_bsp_tree_create(size_t max_depth) {

  commc_bsp_tree_t* tree;

  if  (max_depth == 0) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  tree = (commc_bsp_tree_t*)malloc(sizeof(commc_bsp_tree_t));
  
  if  (!tree) {
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  tree->root = NULL;
  tree->polygon_count = 0;
  tree->max_depth = max_depth;

  return tree;

}

/*

         commc_bsp_tree_destroy()
	       ---
	       destroys the BSP tree and frees all associated memory.

*/

void commc_bsp_tree_destroy(commc_bsp_tree_t* tree) {

  if  (!tree) {
    return;
  }

  destroy_node(tree->root);
  free(tree);

}

/*

         commc_bsp_tree_insert_polygon()
	       ---
	       inserts a polygon into the BSP tree at the appropriate location.

*/

commc_error_t commc_bsp_tree_insert_polygon(commc_bsp_tree_t* tree, 
                                            const commc_polygon_t* polygon) {

  commc_plane_t splitting_plane;
  commc_error_t result;

  if  (!tree || !polygon || polygon->vertex_count < 3) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  /* if tree is empty, create root node */
  if  (!tree->root) {

    /* create plane from polygon's first three vertices */
    result = commc_plane_from_vertices(&splitting_plane,
                                       &polygon->vertices[0],
                                       &polygon->vertices[1],
                                       &polygon->vertices[2]);

    if  (result != COMMC_SUCCESS) {
      return result;
    }

    tree->root = create_node(splitting_plane);
    
    if  (!tree->root) {
      return COMMC_MEMORY_ERROR;
    }

  }

  result = insert_polygon_into_node(tree->root, polygon, 0, tree->max_depth);

  if  (result == COMMC_SUCCESS) {
    tree->polygon_count++;
  }

  return result;

}

/*

         commc_bsp_tree_traverse_front_to_back()
	       ---
	       traverses the BSP tree from front to back relative to a viewpoint.

*/

commc_error_t commc_bsp_tree_traverse_front_to_back(commc_bsp_tree_t* tree,
                                                     commc_vertex_t viewpoint,
                                                     commc_bsp_traversal_callback_t callback,
                                                     void* user_data) {

  if  (!tree || !callback) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  return traverse_node_front_to_back(tree->root, viewpoint, callback, user_data);

}

/*

         commc_bsp_tree_traverse_back_to_front()
	       ---
	       traverses the BSP tree from back to front relative to a viewpoint.

*/

commc_error_t commc_bsp_tree_traverse_back_to_front(commc_bsp_tree_t* tree,
                                                     commc_vertex_t viewpoint,
                                                     commc_bsp_traversal_callback_t callback,
                                                     void* user_data) {

  if  (!tree || !callback) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  return traverse_node_back_to_front(tree->root, viewpoint, callback, user_data);

}

/*

         commc_plane_classify_point()
	       ---
	       classifies a point's position relative to a plane.

*/

commc_point_classification_t commc_plane_classify_point(const commc_plane_t* plane,
                                                        const commc_vertex_t* point) {

  double distance;

  if  (!plane || !point) {
    return COMMC_POINT_ON_PLANE;
  }

  distance = calculate_plane_distance(plane, point);

  if  (distance > COMMC_BSP_EPSILON) {
    return COMMC_POINT_IN_FRONT;
  } else if  (distance < -COMMC_BSP_EPSILON) {
    return COMMC_POINT_BEHIND;
  } else {
    return COMMC_POINT_ON_PLANE;
  }

}

/*

         commc_plane_from_vertices()
	       ---
	       constructs a plane equation from three non-collinear vertices.

*/

commc_error_t commc_plane_from_vertices(commc_plane_t* plane,
                                        const commc_vertex_t* v1,
                                        const commc_vertex_t* v2,
                                        const commc_vertex_t* v3) {

  double u_x, u_y, u_z;  /* vector from v1 to v2 */
  double v_x, v_y, v_z;  /* vector from v1 to v3 */

  if  (!plane || !v1 || !v2 || !v3) {
    return COMMC_ARGUMENT_ERROR;
  }

  /* calculate vectors */
  u_x = v2->x - v1->x;
  u_y = v2->y - v1->y;
  u_z = v2->z - v1->z;

  v_x = v3->x - v1->x;
  v_y = v3->y - v1->y;
  v_z = v3->z - v1->z;

  /* calculate normal using cross product */
  plane->a = u_y * v_z - u_z * v_y;
  plane->b = u_z * v_x - u_x * v_z;
  plane->c = u_x * v_y - u_y * v_x;

  /* calculate distance parameter */
  plane->d = -(plane->a * v1->x + plane->b * v1->y + plane->c * v1->z);

  /* normalize the plane equation */
  normalize_plane(plane);

  return COMMC_SUCCESS;

}

/*

         commc_polygon_create()
	       ---
	       creates a new polygon with the specified vertices.

*/

commc_polygon_t* commc_polygon_create(const commc_vertex_t* vertices,
                                      size_t vertex_count,
                                      void* user_data) {

  commc_polygon_t* polygon;
  size_t           i;

  if  (!vertices || vertex_count < 3) {
    return NULL;
  }

  polygon = (commc_polygon_t*)malloc(sizeof(commc_polygon_t));
  
  if  (!polygon) {
    return NULL;
  }

  polygon->vertices = (commc_vertex_t*)malloc(sizeof(commc_vertex_t) * vertex_count);
  
  if  (!polygon->vertices) {
    free(polygon);
    return NULL;
  }

  /* copy vertex data */
  for  (i = 0; i < vertex_count; i++) {
    polygon->vertices[i] = vertices[i];
  }

  polygon->vertex_count = vertex_count;
  polygon->user_data = user_data;

  return polygon;

}

/*

         commc_polygon_destroy()
	       ---
	       destroys a polygon and frees its vertex memory.

*/

void commc_polygon_destroy(commc_polygon_t* polygon) {

  if  (!polygon) {
    return;
  }

  if  (polygon->vertices) {
    free(polygon->vertices);
  }

  free(polygon);

}

/*

         commc_polygon_split()
	       ---
	       splits a polygon by a plane into front and back portions.
	       simplified implementation for educational purposes.

*/

commc_error_t commc_polygon_split(const commc_polygon_t* polygon,
                                  const commc_plane_t* plane,
                                  commc_polygon_t** front_polygon,
                                  commc_polygon_t** back_polygon) {

  /* simplified implementation - for full BSP trees, this would need
     sophisticated polygon clipping algorithms */
     
  commc_point_classification_t classification;

  if  (!polygon || !plane || !front_polygon || !back_polygon) {
    return COMMC_ARGUMENT_ERROR;
  }

  /* for educational purposes, just classify the entire polygon
     based on its first vertex */
  
  classification = commc_plane_classify_point(plane, &polygon->vertices[0]);

  if  (classification == COMMC_POINT_IN_FRONT || classification == COMMC_POINT_ON_PLANE) {
    *front_polygon = commc_polygon_create(polygon->vertices, polygon->vertex_count, polygon->user_data);
    *back_polygon = NULL;
  } else {
    *front_polygon = NULL;
    *back_polygon = commc_polygon_create(polygon->vertices, polygon->vertex_count, polygon->user_data);
  }

  return COMMC_SUCCESS;

}

/*

         commc_bsp_tree_polygon_count()
	       ---
	       returns the total number of polygons in the BSP tree.

*/

size_t commc_bsp_tree_polygon_count(commc_bsp_tree_t* tree) {

  if  (!tree) {
    return 0;
  }

  return tree->polygon_count;

}

/*

         commc_bsp_tree_depth()
	       ---
	       calculates the maximum depth of the BSP tree.

*/

size_t commc_bsp_tree_depth(commc_bsp_tree_t* tree) {

  if  (!tree) {
    return 0;
  }

  return get_max_depth(tree->root);

}

/*

         commc_bsp_tree_clear()
	       ---
	       removes all polygons from the BSP tree.

*/

void commc_bsp_tree_clear(commc_bsp_tree_t* tree) {

  if  (!tree) {
    return;
  }

  destroy_node(tree->root);
  tree->root = NULL;
  tree->polygon_count = 0;

}

/*
	==================================
             --- EOF ---
	==================================
*/