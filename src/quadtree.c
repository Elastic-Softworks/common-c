/*
   ===================================
   C O M M O N - C
   QUADTREE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- QUADTREE MODULE ---

    implementation of the quadtree spatial data structure for
    efficient 2D spatial partitioning. see include/commc/quadtree.h
    for function prototypes and comprehensive documentation.

    quadtree subdivision strategy:
    - each node covers a rectangular region of 2D space
    - when node capacity is exceeded, subdivide into four quadrants:
      NE (northeast), NW (northwest), SE (southeast), SW (southwest)
    - points are stored in leaf nodes or pushed down during subdivision
    - spatial queries traverse only relevant quadrants for efficiency

    this implementation provides educational examples of:

    - recursive tree data structures
    - spatial indexing algorithms  
    - geometric intersection testing
    - divide-and-conquer problem solving

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include    "commc/quadtree.h"
#include    "commc/list.h"
#include    "commc/error.h"

#include    <stdlib.h>
#include    <string.h>
#include    <math.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal quadtree node structure. */

typedef struct commc_quadtree_node_t {

  commc_rectangle_t    boundary;     /* spatial region covered by this node */
  commc_list_t*        points;       /* points stored in this node (leaf) */
  size_t               capacity;     /* max points before subdivision */
  size_t               depth;        /* current depth in tree */
  size_t               max_depth;    /* maximum allowed depth */
  
  /* child quadrants (NULL if leaf node) */

  struct commc_quadtree_node_t* northeast;
  struct commc_quadtree_node_t* northwest;
  struct commc_quadtree_node_t* southeast;
  struct commc_quadtree_node_t* southwest;

} commc_quadtree_node_t;

/* main quadtree structure. */

struct commc_quadtree_t {

  commc_quadtree_node_t* root;        /* root node of the tree */
  size_t                 total_points; /* total points in entire tree */

};

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         create_node()
	     ---
	     creates a new quadtree node with specified boundary and parameters.
	     initializes as a leaf node with empty point list.

*/

static commc_quadtree_node_t* create_node(commc_rectangle_t boundary, 
                                          size_t capacity, 
                                          size_t depth, 
                                          size_t max_depth) {

  commc_quadtree_node_t* node;

  node = (commc_quadtree_node_t*)malloc(sizeof(commc_quadtree_node_t));

  if  (!node) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  node->boundary = boundary;
  node->capacity = capacity;
  node->depth = depth;
  node->max_depth = max_depth;
  node->points = commc_list_create();

  if  (!node->points) {

    free(node);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  /* initialize as leaf node */

  node->northeast = NULL;
  node->northwest = NULL;
  node->southeast = NULL;
  node->southwest = NULL;

  return node;

}

/*

         destroy_node()
	     ---
	     recursively destroys a quadtree node and all its children.
	     frees associated memory but not user data pointed to by points.

*/

static void destroy_node(commc_quadtree_node_t* node) {

  commc_point2d_t* point;

  if  (!node) {

    return;

  }

  /* recursively destroy children */

  if  (node->northeast) {

    destroy_node(node->northeast);

  }

  if  (node->northwest) {

    destroy_node(node->northwest);

  }

  if  (node->southeast) {

    destroy_node(node->southeast);

  }

  if  (node->southwest) {

    destroy_node(node->southwest);

  }

  /* free points list and node memory */

  if  (node->points) {

    /* free point structures but not user data */

    while  (!commc_list_is_empty(node->points)) {

      point = (commc_point2d_t*)commc_list_front(node->points);
      commc_list_pop_front(node->points);
      free(point);

    }

    commc_list_destroy(node->points);

  }

  free(node);

}

/*

         contains_point()
	     ---
	     checks if a point lies within a rectangular boundary.
	     uses standard bounding box intersection test.

*/

static int contains_point(commc_rectangle_t rect, double x, double y) {

  return (x >= rect.x && 
          x < rect.x + rect.width && 
          y >= rect.y && 
          y < rect.y + rect.height);

}

/*

         rectangles_intersect()
	       ---
	       checks if two rectangles overlap.
	       used for spatial query optimization.

*/

static int rectangles_intersect(commc_rectangle_t a, commc_rectangle_t b) {

  return (a.x < b.x + b.width && 
          a.x + a.width > b.x && 
          a.y < b.y + b.height && 
          a.y + a.height > b.y);

}

/*

         distance_squared()
	     ---
	     calculates squared euclidean distance between two points.
	     avoids sqrt() for performance in distance comparisons.

*/

static double distance_squared(double x1, double y1, double x2, double y2) {

  double dx;
  double dy;

  dx = x2 - x1;
  dy = y2 - y1;

  return dx * dx + dy * dy;

}

/*

         subdivide()
	       ---
	       subdivides a leaf node into four child quadrants.
	       redistributes existing points among child nodes.

*/

static commc_error_t subdivide(commc_quadtree_node_t* node) {

  commc_rectangle_t     ne_boundary;
  commc_rectangle_t     nw_boundary;
  commc_rectangle_t     se_boundary;
  commc_rectangle_t     sw_boundary;
  double                half_width;
  double                half_height;
  commc_list_iterator_t iterator;
  commc_point2d_t*      point;
  commc_point2d_t*      point_copy;

  if  (!node || node->northeast) {

    return COMMC_ARGUMENT_ERROR;        /* already subdivided or invalid */

  }

  half_width = node->boundary.width / 2.0;
  half_height = node->boundary.height / 2.0;

  /* define child quadrant boundaries */

  /* northeast quadrant */

  ne_boundary.x = node->boundary.x + half_width;
  ne_boundary.y = node->boundary.y + half_height;
  ne_boundary.width = half_width;
  ne_boundary.height = half_height;

  /* northwest quadrant */

  nw_boundary.x = node->boundary.x;
  nw_boundary.y = node->boundary.y + half_height;
  nw_boundary.width = half_width;
  nw_boundary.height = half_height;

  /* southeast quadrant */

  se_boundary.x = node->boundary.x + half_width;
  se_boundary.y = node->boundary.y;
  se_boundary.width = half_width;
  se_boundary.height = half_height;

  /* southwest quadrant */

  sw_boundary.x = node->boundary.x;
  sw_boundary.y = node->boundary.y;
  sw_boundary.width = half_width;
  sw_boundary.height = half_height;

  /* create child nodes */

  node->northeast = create_node(ne_boundary, node->capacity, 
                                node->depth + 1, node->max_depth);

  node->northwest = create_node(nw_boundary, node->capacity, 
                                node->depth + 1, node->max_depth);

  node->southeast = create_node(se_boundary, node->capacity, 
                                node->depth + 1, node->max_depth);

  node->southwest = create_node(sw_boundary, node->capacity, 
                                node->depth + 1, node->max_depth);

  if  (!node->northeast || !node->northwest || !node->southeast || !node->southwest) {

    /* cleanup partial creation */

    if  (node->northeast) destroy_node(node->northeast);
    if  (node->northwest) destroy_node(node->northwest);
    if  (node->southeast) destroy_node(node->southeast);
    if  (node->southwest) destroy_node(node->southwest);

    node->northeast = NULL;
    node->northwest = NULL;
    node->southeast = NULL;
    node->southwest = NULL;

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  /* redistribute existing points to child nodes */

  iterator = commc_list_begin(node->points);

  while  (commc_list_iterator_data(&iterator) != NULL) {

    point = (commc_point2d_t*)commc_list_iterator_data(&iterator);

    /* create copy of point for child insertion */

    point_copy = (commc_point2d_t*)malloc(sizeof(commc_point2d_t));

    if  (point_copy) {

      *point_copy = *point;

      /* insert into appropriate child quadrant */

      if  (contains_point(ne_boundary, point->x, point->y)) {

        commc_list_push_back(node->northeast->points, point_copy);

      } else if  (contains_point(nw_boundary, point->x, point->y)) {

        commc_list_push_back(node->northwest->points, point_copy);

      } else if  (contains_point(se_boundary, point->x, point->y)) {

        commc_list_push_back(node->southeast->points, point_copy);

      } else if  (contains_point(sw_boundary, point->x, point->y)) {

        commc_list_push_back(node->southwest->points, point_copy);

      } else {

        free(point_copy);       /* point doesn't fit in any child */

      }

    }

    commc_list_next(&iterator);

  }

  /* clear points from this node (now internal node) */

  while  (!commc_list_is_empty(node->points)) {

    point = (commc_point2d_t*)commc_list_front(node->points);
    commc_list_pop_front(node->points);
    free(point);

  }

  return COMMC_SUCCESS;

}

/*

         insert_into_node()
	     ---
	     recursively inserts a point into the appropriate node.
	     handles subdivision when capacity limits are exceeded.

*/

static commc_error_t insert_into_node(commc_quadtree_node_t* node, commc_point2d_t point) {

  commc_point2d_t* point_copy;
  commc_error_t    result;

  if  (!node) {

    return COMMC_ARGUMENT_ERROR;

  }

  /* check if point is within node boundary */

  if  (!contains_point(node->boundary, point.x, point.y)) {

    return COMMC_ARGUMENT_ERROR; /* point outside boundary */

  }

  /* if this is a leaf node */

  if  (!node->northeast) {

    /* check if we need to subdivide */

    if  (commc_list_size(node->points) < node->capacity || 
         node->depth >= node->max_depth) {

      /* add point to this node */
      point_copy = (commc_point2d_t*)malloc(sizeof(commc_point2d_t));

      if  (!point_copy) {

        commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
        return COMMC_MEMORY_ERROR;

      }

      *point_copy = point;
      commc_list_push_back(node->points, point_copy);

      return COMMC_SUCCESS;

    } else {

      /* subdivide this node */
      result = subdivide(node);

      if  (result != COMMC_SUCCESS) {

        return result;

      }

      /* fall through to insert into child */

    }

  }

  /* insert into appropriate child quadrant */

  if  (contains_point(node->northeast->boundary, point.x, point.y)) {

    return insert_into_node(node->northeast, point);

  } else if  (contains_point(node->northwest->boundary, point.x, point.y)) {

    return insert_into_node(node->northwest, point);

  } else if  (contains_point(node->southeast->boundary, point.x, point.y)) {

    return insert_into_node(node->southeast, point);

  } else if  (contains_point(node->southwest->boundary, point.x, point.y)) {

    return insert_into_node(node->southwest, point);

  }

  return COMMC_ARGUMENT_ERROR;      /* shouldn't reach here */

}

/*

         query_node_range()
	     ---
	     recursively searches a node for points within query range.
	     optimizes by skipping child nodes that don't intersect range.

*/

static commc_error_t query_node_range(commc_quadtree_node_t* node, 
                                      commc_rectangle_t range,
                                      commc_quadtree_query_callback_t callback, 
                                      void* user_data) {

  commc_list_iterator_t iterator;
  commc_point2d_t*      point;
  int                   result;

  if  (!node || !rectangles_intersect(node->boundary, range)) {

    return COMMC_SUCCESS;       /* no intersection */

  }

  /* if this is a leaf node, check all points */

  if  (!node->northeast) {

    iterator = commc_list_begin(node->points);

    while  (commc_list_iterator_data(&iterator) != NULL) {

      point = (commc_point2d_t*)commc_list_iterator_data(&iterator);

      if  (contains_point(range, point->x, point->y)) {

        result = callback(point, user_data);

        if  (result != 0) {

          return COMMC_SUCCESS; /* callback requested stop */

        }

      }

      commc_list_next(&iterator);

    }

  } else {

    /* recursively search child nodes */

    query_node_range(node->northeast, range, callback, user_data);
    query_node_range(node->northwest, range, callback, user_data);
    query_node_range(node->southeast, range, callback, user_data);
    query_node_range(node->southwest, range, callback, user_data);

  }

  return COMMC_SUCCESS;

}

/*

         get_max_depth()
	     ---
	     recursively finds the maximum depth in the tree.

*/

static size_t get_max_depth(commc_quadtree_node_t* node) {

  size_t max_child_depth;
  size_t ne_depth;
  size_t nw_depth;
  size_t se_depth;
  size_t sw_depth;

  if  (!node) {

    return 0;

  }

  if  (!node->northeast) {

    return node->depth; /* leaf node */

  }

  /* find maximum depth among children */

  ne_depth = get_max_depth(node->northeast);
  nw_depth = get_max_depth(node->northwest);
  se_depth = get_max_depth(node->southeast);
  sw_depth = get_max_depth(node->southwest);

  max_child_depth = ne_depth;

  if  (nw_depth > max_child_depth) max_child_depth = nw_depth;
  if  (se_depth > max_child_depth) max_child_depth = se_depth;
  if  (sw_depth > max_child_depth) max_child_depth = sw_depth;

  return max_child_depth;

}

/* circular query filter data */

struct circle_query_data {

  double center_x;
  double center_y;
  double radius_squared;
  commc_quadtree_query_callback_t original_callback;
  void* original_user_data;

};

/* nearest neighbor search data */

struct nearest_data {

  double x;
  double y;
  commc_point2d_t* nearest;
  double min_distance_squared;

};

/*

         circle_filter_callback()
	     ---
	     internal callback for circular filtering during range queries.

*/

static int circle_filter_callback(const commc_point2d_t* point, void* user_data) {

  struct circle_query_data* data = (struct circle_query_data*)user_data;
  double dist_squared = distance_squared(point->x, point->y, data->center_x, data->center_y);
  
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

static int nearest_callback(const commc_point2d_t* point, void* user_data) {

  struct nearest_data* data = (struct nearest_data*)user_data;
  double dist_squared = distance_squared(point->x, point->y, data->x, data->y);
  
  if  (!data->nearest || dist_squared < data->min_distance_squared) {

    data->nearest = (commc_point2d_t*)point;
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

         commc_quadtree_create()
	     ---
	     creates a quadtree with specified boundary and parameters.
	     initializes with a single root node covering the entire area.

*/

commc_quadtree_t* commc_quadtree_create(commc_rectangle_t boundary, 
                                        size_t capacity, 
                                        size_t max_depth) {

  commc_quadtree_t* quadtree;

  if  (capacity == 0 || max_depth == 0) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  quadtree = (commc_quadtree_t*)malloc(sizeof(commc_quadtree_t));

  if  (!quadtree) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  quadtree->root = create_node(boundary, capacity, 0, max_depth);

  if  (!quadtree->root) {

    free(quadtree);
    return NULL;

  }

  quadtree->total_points = 0;

  return quadtree;

}

/*

         commc_quadtree_destroy()
	     ---
	     frees all memory associated with the quadtree.

*/

void commc_quadtree_destroy(commc_quadtree_t* quadtree) {

  if  (!quadtree) {

    return;

  }

  destroy_node(quadtree->root);
  free(quadtree);

}

/*

         commc_quadtree_insert()
	     ---
	     inserts a point into the quadtree.

*/

commc_error_t commc_quadtree_insert(commc_quadtree_t* quadtree, commc_point2d_t point) {

  commc_error_t result;

  if  (!quadtree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  result = insert_into_node(quadtree->root, point);

  if  (result == COMMC_SUCCESS) {

    quadtree->total_points++;

  }

  return result;

}

/*

         commc_quadtree_query_range()
	     ---
	     finds all points within the specified rectangular region.

*/

commc_error_t commc_quadtree_query_range(commc_quadtree_t* quadtree, 
                                         commc_rectangle_t range,
                                         commc_quadtree_query_callback_t callback, 
                                         void* user_data) {

  if  (!quadtree || !callback) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  return query_node_range(quadtree->root, range, callback, user_data);

}

/*

         commc_quadtree_query_circle()
	     ---
	     finds all points within the specified circular region.

*/

commc_error_t commc_quadtree_query_circle(commc_quadtree_t* quadtree, 
                                          double center_x, double center_y, double radius,
                                          commc_quadtree_query_callback_t callback, 
                                          void* user_data) {

  commc_rectangle_t bounding_box;
  struct circle_query_data circle_data;

  if  (!quadtree || !callback || radius < 0) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* create bounding box for initial filtering */

  bounding_box.x = center_x - radius;
  bounding_box.y = center_y - radius;
  bounding_box.width = radius * 2.0;
  bounding_box.height = radius * 2.0;

  /* setup circle filtering */

  circle_data.center_x = center_x;
  circle_data.center_y = center_y;
  circle_data.radius_squared = radius * radius;
  circle_data.original_callback = callback;
  circle_data.original_user_data = user_data;

  return query_node_range(quadtree->root, bounding_box, circle_filter_callback, &circle_data);

}

/*

         commc_quadtree_point_count()
	       ---
	       returns the total number of points in the quadtree.

*/

size_t commc_quadtree_point_count(commc_quadtree_t* quadtree) {

  if  (!quadtree) {

    return 0;

  }

  return quadtree->total_points;

}

/*

         commc_quadtree_depth()
	       ---
	       returns the maximum depth of subdivision in the quadtree.

*/

size_t commc_quadtree_depth(commc_quadtree_t* quadtree) {

  if  (!quadtree) {

    return 0;

  }

  return get_max_depth(quadtree->root);

}

/*

         commc_quadtree_clear()
	       ---
	       removes all points from the quadtree.

*/

void commc_quadtree_clear(commc_quadtree_t* quadtree) {

  commc_rectangle_t boundary;
  size_t            capacity;
  size_t            max_depth;

  if  (!quadtree) {

    return;

  }

  /* preserve original parameters */

  boundary = quadtree->root->boundary;
  capacity = quadtree->root->capacity;
  max_depth = quadtree->root->max_depth;

  /* destroy and recreate root */

  destroy_node(quadtree->root);
  quadtree->root = create_node(boundary, capacity, 0, max_depth);
  quadtree->total_points = 0;

}

/*

         commc_quadtree_contains_point()
	       ---
	       checks if coordinates fall within the quadtree boundary.

*/

int commc_quadtree_contains_point(commc_quadtree_t* quadtree, double x, double y) {

  if  (!quadtree) {

    return 0;

  }

  return contains_point(quadtree->root->boundary, x, y);

}

/*

         commc_quadtree_get_boundary()
	       ---
	       retrieves the rectangular boundary of the quadtree.

*/

commc_rectangle_t commc_quadtree_get_boundary(commc_quadtree_t* quadtree) {

  commc_rectangle_t empty = {0, 0, 0, 0};

  if  (!quadtree) {

    return empty;

  }

  return quadtree->root->boundary;

}

/*

         commc_quadtree_nearest_neighbor()
	       ---
	       finds the point closest to the specified coordinates.
	       uses a simple brute-force approach for educational clarity.

*/

commc_point2d_t* commc_quadtree_nearest_neighbor(commc_quadtree_t* quadtree, 
                                                 double x, double y, 
                                                 double* distance) {

  struct nearest_data search_data;

  if  (!quadtree) {

    return NULL;

  }

  /* initialize search */

  search_data.x = x;
  search_data.y = y;
  search_data.nearest = NULL;
  search_data.min_distance_squared = 0;

  /* search entire quadtree */

  query_node_range(quadtree->root, quadtree->root->boundary, nearest_callback, &search_data);

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