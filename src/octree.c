/* 	
   ===================================
   C O M M C / O C T R E E . C
   OCTREE 3D SPATIAL PARTITIONING IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- OCTREE IMPLEMENTATION ---

	    this implementation provides a complete octree system for
	    3D spatial partitioning. octrees extend quadtree concepts
	    to three dimensions, recursively subdividing 3D space into
	    eight octants (cubes) for efficient spatial indexing.
	    
	    the implementation handles automatic subdivision, 3D geometric
	    queries including bounding box and spherical searches, and
	    maintains strict C89 compliance throughout.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/octree.h"   /* OCTREE API DECLARATIONS */
#include "commc/error.h"    /* ERROR HANDLING */
#include "commc/list.h"     /* DYNAMIC LISTS */
#include <math.h>           /* MATHEMATICAL FUNCTIONS */
#include <stdlib.h>         /* STANDARD LIBRARY FUNCTIONS */

/* 
	==================================
             --- HELPERS ---
	==================================
*/

/*

         create_node()
	       ---
	       creates a new octree node with specified boundary.
	       initializes all child pointers to null and creates
	       an empty point list.

*/

static commc_octree_node_t* create_node(commc_bounding_box_t boundary) {

  commc_octree_node_t* node;
  
  node = (commc_octree_node_t*)malloc(sizeof(commc_octree_node_t));
  
  if  (!node) {
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  node->boundary = boundary;
  node->points = commc_list_create();
  node->nne = NULL;
  node->nnw = NULL;
  node->nse = NULL;
  node->nsw = NULL;
  node->sne = NULL;
  node->snw = NULL;
  node->sse = NULL;
  node->ssw = NULL;
  
  if  (!node->points) {
    free(node);
    return NULL;
  }

  return node;

}

/*

         destroy_node()
	       ---
	       recursively destroys an octree node and all its children.
	       frees all point storage and node memory.

*/

static void destroy_node(commc_octree_node_t* node) {

  commc_point3d_t* point;

  if  (!node) {
    return;
  }

  /* free all points in this node */
  while  (commc_list_size(node->points) > 0) {

    point = (commc_point3d_t*)commc_list_front(node->points);

    if  (point) {
      free(point);
    }

    commc_list_pop_front(node->points);

  }

  commc_list_destroy(node->points);

  /* recursively destroy all eight children */
  destroy_node(node->nne);
  destroy_node(node->nnw);
  destroy_node(node->nse);
  destroy_node(node->nsw);
  destroy_node(node->sne);
  destroy_node(node->snw);
  destroy_node(node->sse);
  destroy_node(node->ssw);

  free(node);

}

/*

         contains_point()
	       ---
	       checks if a 3D point lies within a bounding box.
	       returns 1 if contained, 0 otherwise.

*/

static int contains_point(commc_bounding_box_t box, double x, double y, double z) {

  return (x >= box.x && x < (box.x + box.width) &&
          y >= box.y && y < (box.y + box.height) &&
          z >= box.z && z < (box.z + box.depth));

}

/*

         boxes_intersect()
	       ---
	       checks if two 3D bounding boxes intersect.
	       uses separating axis theorem - if boxes are separated
	       along any axis, they don't intersect.

*/

static int boxes_intersect(commc_bounding_box_t box1, commc_bounding_box_t box2) {

  return !(box1.x + box1.width <= box2.x ||
           box2.x + box2.width <= box1.x ||
           box1.y + box1.height <= box2.y ||
           box2.y + box2.height <= box1.y ||
           box1.z + box1.depth <= box2.z ||
           box2.z + box2.depth <= box1.z);

}

/*

         distance_squared()
	       ---
	       calculates squared 3D distance between two points.
	       avoids expensive square root calculation when only
	       comparing relative distances.

*/

static double distance_squared(double x1, double y1, double z1, 
                               double x2, double y2, double z2) {

  double dx = x2 - x1;
  double dy = y2 - y1;
  double dz = z2 - z1;

  return dx * dx + dy * dy + dz * dz;

}

/*

         subdivide()
	       ---
	       subdivides a node into eight child octants and redistributes
	       points among them. creates eight equal-sized child boxes
	       representing the eight octants of 3D space.

*/

static commc_error_t subdivide(commc_octree_node_t* node, size_t current_depth, size_t max_depth) {

  double half_width;
  double half_height;
  double half_depth;
  commc_bounding_box_t nne_box, nnw_box, nse_box, nsw_box;
  commc_bounding_box_t sne_box, snw_box, sse_box, ssw_box;
  commc_point3d_t* point;

  if  (!node || current_depth >= max_depth) {
    return COMMC_ARGUMENT_ERROR;
  }

  /* calculate dimensions for child octants */
  half_width = node->boundary.width / 2.0;
  half_height = node->boundary.height / 2.0;
  half_depth = node->boundary.depth / 2.0;

  /* define eight octant boundaries */

  /* north octants (higher Y values) */
  nne_box.x = node->boundary.x + half_width;
  nne_box.y = node->boundary.y + half_height;
  nne_box.z = node->boundary.z + half_depth;
  nne_box.width = half_width;
  nne_box.height = half_height;
  nne_box.depth = half_depth;

  nnw_box.x = node->boundary.x;
  nnw_box.y = node->boundary.y + half_height;
  nnw_box.z = node->boundary.z + half_depth;
  nnw_box.width = half_width;
  nnw_box.height = half_height;
  nnw_box.depth = half_depth;

  nse_box.x = node->boundary.x + half_width;
  nse_box.y = node->boundary.y + half_height;
  nse_box.z = node->boundary.z;
  nse_box.width = half_width;
  nse_box.height = half_height;
  nse_box.depth = half_depth;

  nsw_box.x = node->boundary.x;
  nsw_box.y = node->boundary.y + half_height;
  nsw_box.z = node->boundary.z;
  nsw_box.width = half_width;
  nsw_box.height = half_height;
  nsw_box.depth = half_depth;

  /* south octants (lower Y values) */
  sne_box.x = node->boundary.x + half_width;
  sne_box.y = node->boundary.y;
  sne_box.z = node->boundary.z + half_depth;
  sne_box.width = half_width;
  sne_box.height = half_height;
  sne_box.depth = half_depth;

  snw_box.x = node->boundary.x;
  snw_box.y = node->boundary.y;
  snw_box.z = node->boundary.z + half_depth;
  snw_box.width = half_width;
  snw_box.height = half_height;
  snw_box.depth = half_depth;

  sse_box.x = node->boundary.x + half_width;
  sse_box.y = node->boundary.y;
  sse_box.z = node->boundary.z;
  sse_box.width = half_width;
  sse_box.height = half_height;
  sse_box.depth = half_depth;

  ssw_box.x = node->boundary.x;
  ssw_box.y = node->boundary.y;
  ssw_box.z = node->boundary.z;
  ssw_box.width = half_width;
  ssw_box.height = half_height;
  ssw_box.depth = half_depth;

  /* create eight child nodes */
  node->nne = create_node(nne_box);
  node->nnw = create_node(nnw_box);
  node->nse = create_node(nse_box);
  node->nsw = create_node(nsw_box);
  node->sne = create_node(sne_box);
  node->snw = create_node(snw_box);
  node->sse = create_node(sse_box);
  node->ssw = create_node(ssw_box);

  if  (!node->nne || !node->nnw || !node->nse || !node->nsw ||
       !node->sne || !node->snw || !node->sse || !node->ssw) {

    /* cleanup on failure */
    destroy_node(node->nne);
    destroy_node(node->nnw);
    destroy_node(node->nse);
    destroy_node(node->nsw);
    destroy_node(node->sne);
    destroy_node(node->snw);
    destroy_node(node->sse);
    destroy_node(node->ssw);

    node->nne = node->nnw = node->nse = node->nsw = NULL;
    node->sne = node->snw = node->sse = node->ssw = NULL;

    return COMMC_MEMORY_ERROR;

  }

  /* redistribute points to appropriate child octants */

  while  (commc_list_size(node->points) > 0) {

    point = (commc_point3d_t*)commc_list_front(node->points);
    commc_list_pop_front(node->points);

    if  (point) {

      /* determine which octant contains this point */

      if  (contains_point(node->nne->boundary, point->x, point->y, point->z)) {
        commc_list_push_back(node->nne->points, point);
      } else if  (contains_point(node->nnw->boundary, point->x, point->y, point->z)) {
        commc_list_push_back(node->nnw->points, point);
      } else if  (contains_point(node->nse->boundary, point->x, point->y, point->z)) {
        commc_list_push_back(node->nse->points, point);
      } else if  (contains_point(node->nsw->boundary, point->x, point->y, point->z)) {
        commc_list_push_back(node->nsw->points, point);
      } else if  (contains_point(node->sne->boundary, point->x, point->y, point->z)) {
        commc_list_push_back(node->sne->points, point);
      } else if  (contains_point(node->snw->boundary, point->x, point->y, point->z)) {
        commc_list_push_back(node->snw->points, point);
      } else if  (contains_point(node->sse->boundary, point->x, point->y, point->z)) {
        commc_list_push_back(node->sse->points, point);
      } else if  (contains_point(node->ssw->boundary, point->x, point->y, point->z)) {
        commc_list_push_back(node->ssw->points, point);
      } else {

        /* point doesn't fit in any octant - shouldn't happen */

        free(point);

      }

    }

  }

  return COMMC_SUCCESS;

}

/*

         insert_into_node()
	       ---
	       inserts a point into the specified node.
	       handles subdivision when capacity is exceeded.

*/

static commc_error_t insert_into_node(commc_octree_node_t* node, 
                                      commc_point3d_t* point, 
                                      size_t capacity,
                                      size_t current_depth,
                                      size_t max_depth) {

  commc_error_t result;

  if  (!node || !point) {
    return COMMC_ARGUMENT_ERROR;
  }

  /* if this node has children, find appropriate child */

  if  (node->nne) {

    if  (contains_point(node->nne->boundary, point->x, point->y, point->z)) {
      return insert_into_node(node->nne, point, capacity, current_depth + 1, max_depth);
    } else if  (contains_point(node->nnw->boundary, point->x, point->y, point->z)) {
      return insert_into_node(node->nnw, point, capacity, current_depth + 1, max_depth);
    } else if  (contains_point(node->nse->boundary, point->x, point->y, point->z)) {
      return insert_into_node(node->nse, point, capacity, current_depth + 1, max_depth);
    } else if  (contains_point(node->nsw->boundary, point->x, point->y, point->z)) {
      return insert_into_node(node->nsw, point, capacity, current_depth + 1, max_depth);
    } else if  (contains_point(node->sne->boundary, point->x, point->y, point->z)) {
      return insert_into_node(node->sne, point, capacity, current_depth + 1, max_depth);
    } else if  (contains_point(node->snw->boundary, point->x, point->y, point->z)) {
      return insert_into_node(node->snw, point, capacity, current_depth + 1, max_depth);
    } else if  (contains_point(node->sse->boundary, point->x, point->y, point->z)) {
      return insert_into_node(node->sse, point, capacity, current_depth + 1, max_depth);
    } else if  (contains_point(node->ssw->boundary, point->x, point->y, point->z)) {
      return insert_into_node(node->ssw, point, capacity, current_depth + 1, max_depth);
    } else {

      return COMMC_ARGUMENT_ERROR;

    }

  } else {

    /* leaf node - add point here */

    commc_list_push_back(node->points, point);

    /* subdivide if capacity exceeded and depth allows */

    if  (commc_list_size(node->points) > capacity && current_depth < max_depth) {

      result = subdivide(node, current_depth, max_depth);

      if  (result != COMMC_SUCCESS) {
        return result;
      }

    }

  }

  return COMMC_SUCCESS;

}

/*

         query_node_range()
	       ---
	       recursively searches a node for points within query range.
	       optimizes by skipping child nodes that don't intersect range.

*/

static commc_error_t query_node_range(commc_octree_node_t* node, 
                                      commc_bounding_box_t range,
                                      commc_octree_query_callback_t callback, 
                                      void* user_data) {

  commc_list_iterator_t iterator;
  commc_point3d_t*      point;
  int                   result;

  if  (!node || !boxes_intersect(node->boundary, range)) {

    return COMMC_SUCCESS; /* no intersection */

  }

  /* if this is a leaf node, check all points */

  if  (!node->nne) {

    iterator = commc_list_begin(node->points);

    while  (commc_list_iterator_data(&iterator) != NULL) {

      point = (commc_point3d_t*)commc_list_iterator_data(&iterator);

      if  (point && contains_point(range, point->x, point->y, point->z)) {

        result = callback(point, user_data);

        if  (result != 0) {
          return COMMC_SUCCESS; /* early termination requested */
        }

      }

      commc_list_next(&iterator);

    }

  } else {

    /* internal node - recursively search children */

    query_node_range(node->nne, range, callback, user_data);
    query_node_range(node->nnw, range, callback, user_data);
    query_node_range(node->nse, range, callback, user_data);
    query_node_range(node->nsw, range, callback, user_data);
    query_node_range(node->sne, range, callback, user_data);
    query_node_range(node->snw, range, callback, user_data);
    query_node_range(node->sse, range, callback, user_data);
    query_node_range(node->ssw, range, callback, user_data);

  }

  return COMMC_SUCCESS;

}

/*

         get_max_depth()
	       ---
	       recursively finds the maximum depth in the tree.

*/

static size_t get_max_depth(commc_octree_node_t* node) {

  size_t max_child_depth;
  size_t nne_depth, nnw_depth, nse_depth, nsw_depth;
  size_t sne_depth, snw_depth, sse_depth, ssw_depth;

  if  (!node) {
    return 0;
  }

  if  (!node->nne) {
    /* leaf node */
    return 1;
  } else {

    /* internal node - find maximum child depth */

    nne_depth = get_max_depth(node->nne);
    nnw_depth = get_max_depth(node->nnw);
    nse_depth = get_max_depth(node->nse);
    nsw_depth = get_max_depth(node->nsw);
    sne_depth = get_max_depth(node->sne);
    snw_depth = get_max_depth(node->snw);
    sse_depth = get_max_depth(node->sse);
    ssw_depth = get_max_depth(node->ssw);

    max_child_depth = nne_depth;
    if (nnw_depth > max_child_depth) max_child_depth = nnw_depth;
    if (nse_depth > max_child_depth) max_child_depth = nse_depth;
    if (nsw_depth > max_child_depth) max_child_depth = nsw_depth;
    if (sne_depth > max_child_depth) max_child_depth = sne_depth;
    if (snw_depth > max_child_depth) max_child_depth = snw_depth;
    if (sse_depth > max_child_depth) max_child_depth = sse_depth;
    if (ssw_depth > max_child_depth) max_child_depth = ssw_depth;

    return 1 + max_child_depth;

  }

}

/* spherical query filter data */

struct sphere_query_data {

  double center_x;
  double center_y;
  double center_z;
  double radius_squared;
  commc_octree_query_callback_t original_callback;
  void* original_user_data;

};

/* nearest neighbor search data */

struct nearest_data {

  double x;
  double y;
  double z;
  commc_point3d_t* nearest;
  double min_distance_squared;

};

/*

         sphere_filter_callback()
	       ---
	       internal callback for spherical filtering during range queries.

*/

static int sphere_filter_callback(const commc_point3d_t* point, void* user_data) {

  struct sphere_query_data* data = (struct sphere_query_data*)user_data;
  double dist_squared = distance_squared(point->x, point->y, point->z, data->center_x, data->center_y, data->center_z);
  
  if  (dist_squared <= data->radius_squared) {

    return data->original_callback(point, data->original_user_data);

  }
  
  return 0; /* continue searching */

}

/*

         nearest_callback()
	       ---
	       internal callback for nearest neighbor searches.

*/

static int nearest_callback(const commc_point3d_t* point, void* user_data) {

  struct nearest_data* data = (struct nearest_data*)user_data;
  double dist_squared = distance_squared(point->x, point->y, point->z, data->x, data->y, data->z);
  
  if  (!data->nearest || dist_squared < data->min_distance_squared) {

    data->nearest = (commc_point3d_t*)point;
    data->min_distance_squared = dist_squared;

  }
  
  return 0; /* continue searching */

}

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_octree_create()
	       ---
	       creates a new octree with specified 3D boundary and capacity.

*/

commc_octree_t* commc_octree_create(commc_bounding_box_t boundary,
                                    size_t capacity,
                                    size_t max_depth) {

  commc_octree_t* octree;

  if  (capacity == 0 || max_depth == 0) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  octree = (commc_octree_t*)malloc(sizeof(commc_octree_t));
  
  if  (!octree) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  octree->root = create_node(boundary);
  octree->capacity = capacity;
  octree->total_points = 0;
  octree->max_depth = max_depth;

  if  (!octree->root) {

    free(octree);
    return NULL;

  }

  return octree;

}

/*

         commc_octree_destroy()
	       ---
	       destroys the octree and frees all associated memory.

*/

void commc_octree_destroy(commc_octree_t* octree) {

  if  (!octree) {

    return;

  }

  destroy_node(octree->root);
  free(octree);

}

/*

         commc_octree_insert()
	       ---
	       inserts a 3D point into the octree at the appropriate location.

*/

commc_error_t commc_octree_insert(commc_octree_t* octree, commc_point3d_t point) {

  commc_point3d_t* new_point;
  commc_error_t    result;

  if  (!octree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  if  (!contains_point(octree->root->boundary, point.x, point.y, point.z)) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* create copy of point */

  new_point = (commc_point3d_t*)malloc(sizeof(commc_point3d_t));
  
  if  (!new_point) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  *new_point = point;

  result = insert_into_node(octree->root, new_point, octree->capacity, 0, octree->max_depth);

  if  (result == COMMC_SUCCESS) {

    octree->total_points++;

  }

  return result;

}

/*

         commc_octree_query_range()
	       ---
	       finds all points within the specified 3D bounding box.

*/

commc_error_t commc_octree_query_range(commc_octree_t* octree,
                                       commc_bounding_box_t* range,
                                       commc_octree_query_callback_t callback,
                                       void* user_data) {

  if  (!octree || !range || !callback) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  return query_node_range(octree->root, *range, callback, user_data);

}

/*

         commc_octree_query_sphere()
	       ---
	       finds all points within the specified spherical region.

*/

commc_error_t commc_octree_query_sphere(commc_octree_t* octree,
                                        double center_x, double center_y, double center_z, double radius,
                                        commc_octree_query_callback_t callback,
                                        void* user_data) {

  commc_bounding_box_t bounding_box;
  struct sphere_query_data sphere_data;

  if  (!octree || !callback || radius < 0) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* create bounding box for initial filtering */

  bounding_box.x = center_x - radius;
  bounding_box.y = center_y - radius;
  bounding_box.z = center_z - radius;
  bounding_box.width = radius * 2.0;
  bounding_box.height = radius * 2.0;
  bounding_box.depth = radius * 2.0;

  /* setup sphere filtering */

  sphere_data.center_x = center_x;
  sphere_data.center_y = center_y;
  sphere_data.center_z = center_z;
  sphere_data.radius_squared = radius * radius;
  sphere_data.original_callback = callback;
  sphere_data.original_user_data = user_data;

  return query_node_range(octree->root, bounding_box, sphere_filter_callback, &sphere_data);

}

/*

         commc_octree_point_count()
	       ---
	       returns the total number of points in the octree.

*/

size_t commc_octree_point_count(commc_octree_t* octree) {

  if  (!octree) {
    return 0;
  }

  return octree->total_points;

}

/*

         commc_octree_depth()
	       ---
	       calculates the maximum depth of the octree.

*/

size_t commc_octree_depth(commc_octree_t* octree) {

  if  (!octree) {

    return 0;

  }

  return get_max_depth(octree->root);

}

/*

         commc_octree_clear()
	       ---
	       removes all points from the octree and resets to initial state.

*/

void commc_octree_clear(commc_octree_t* octree) {

  commc_bounding_box_t boundary;

  if  (!octree) {
    return;
  }

  boundary = octree->root->boundary;

  destroy_node(octree->root);

  octree->root = create_node(boundary);
  octree->total_points = 0;

}

/*

         commc_octree_contains_point()
	       ---
	       checks if coordinates fall within the octree boundary.

*/

int commc_octree_contains_point(commc_octree_t* octree, double x, double y, double z) {

  if  (!octree) {

    return 0;

  }

  return contains_point(octree->root->boundary, x, y, z);

}

/*

         commc_octree_get_boundary()
	       ---
	       retrieves the 3D bounding box that defines the octree boundary.

*/

commc_bounding_box_t commc_octree_get_boundary(commc_octree_t* octree) {

  commc_bounding_box_t empty = {0, 0, 0, 0, 0, 0};

  if  (!octree) {

    return empty;

  }

  return octree->root->boundary;

}

/*

         commc_octree_nearest_neighbor()
	       ---
	       finds the point closest to the specified 3D coordinates.

*/

commc_point3d_t* commc_octree_nearest_neighbor(commc_octree_t* octree,
                                               double x, double y, double z,
                                               double* distance) {

  struct nearest_data search_data;

  if  (!octree) {

    return NULL;

  }

  /* initialize search */

  search_data.x = x;
  search_data.y = y;
  search_data.z = z;
  search_data.nearest = NULL;
  search_data.min_distance_squared = 0;

  /* search entire octree */

  query_node_range(octree->root, octree->root->boundary, nearest_callback, &search_data);

  /* set distance if requested */
  
  if  (distance && search_data.nearest) {

    *distance = sqrt(search_data.min_distance_squared);

  }

  return search_data.nearest;

}

/*
	==================================
             --- EOF ---
	==================================
*/