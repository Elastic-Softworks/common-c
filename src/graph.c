/*
   ===================================
   C O M M O N - C
   GRAPH IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- GRAPH MODULE ---

    implementation of the graph data structure supporting
    both adjacency list and adjacency matrix representations.
    see include/commc/graph.h for function prototypes
    and comprehensive documentation.

    this implementation provides educational examples of
    fundamental graph theory concepts and efficient
    algorithms for different graph densities.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/graph.h"
#include "commc/list.h"
#include "commc/error.h"
#include "commc/queue.h"
#include "commc/stack.h"
#include "commc/priorityqueue.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* adjacency list edge node for sparse representation. */

typedef struct commc_adjacency_edge_t {

  size_t to;       /* destination vertex */
  double weight;   /* edge weight */

} commc_adjacency_edge_t;

/* main graph structure with dual representation support. */

struct commc_graph_t {

  size_t                         vertex_count;    /* number of vertices */
  size_t                         edge_count;      /* total edges */
  commc_graph_type_t             type;            /* directed/undirected */
  commc_graph_representation_t   representation;  /* list/matrix */

  /* adjacency list representation (sparse graphs) */
  commc_list_t**                 adjacency_lists; /* array of lists */

  /* adjacency matrix representation (dense graphs) */
  double**                       adjacency_matrix; /* 2D weight matrix */
  char**                         edge_exists;     /* 2D boolean matrix */

};

/* iterator structure for graph traversal. */

struct commc_graph_iterator_t {

  commc_graph_t*        graph;           /* source graph */
  size_t                current_vertex;  /* current vertex index */
  commc_list_iterator_t list_iterator;   /* for adjacency list */
  size_t                matrix_row;      /* for adjacency matrix */
  size_t                matrix_col;      /* for adjacency matrix */
  int                   iterating_edges; /* 1 for edges, 0 for neighbors */
  commc_graph_edge_t    current_edge;    /* current edge data */

};

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         create_adjacency_lists()
	       ---
	       initializes the adjacency list representation.
	       creates an array of linked lists, one per vertex.

*/

static commc_error_t create_adjacency_lists(commc_graph_t* graph) {

  size_t i;

  graph->adjacency_lists = (commc_list_t**)malloc(sizeof(commc_list_t*) * graph->vertex_count);

  if  (!graph->adjacency_lists) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  for  (i = 0; i < graph->vertex_count; i++) {

    graph->adjacency_lists[i] = commc_list_create();

    if  (!graph->adjacency_lists[i]) {

      size_t j;

      commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);

      /* cleanup partial initialization */

      for  (j = 0; j < i; j++) {
        commc_list_destroy(graph->adjacency_lists[j]);
      }
      free(graph->adjacency_lists);
      return COMMC_MEMORY_ERROR;

    }

  }

  return COMMC_SUCCESS;

}

/*

         create_adjacency_matrix()
	       ---
	       initializes the adjacency matrix representation.
	       creates a 2D matrix for O(1) edge lookups.

*/

static commc_error_t create_adjacency_matrix(commc_graph_t* graph) {

  size_t i;

  /* allocate weight matrix */
  graph->adjacency_matrix = (double**)malloc(sizeof(double*) * graph->vertex_count);

  if  (!graph->adjacency_matrix) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  /* allocate existence matrix */
  graph->edge_exists = (char**)malloc(sizeof(char*) * graph->vertex_count);

  if  (!graph->edge_exists) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    free(graph->adjacency_matrix);
    return COMMC_MEMORY_ERROR;

  }

  /* allocate and initialize matrix rows */

  for  (i = 0; i < graph->vertex_count; i++) {

    graph->adjacency_matrix[i] = (double*)calloc(graph->vertex_count, sizeof(double));
    graph->edge_exists[i] = (char*)calloc(graph->vertex_count, sizeof(char));

    if  (!graph->adjacency_matrix[i] || !graph->edge_exists[i]) {

      size_t j;

      commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);

      /* cleanup partial initialization */

      for  (j = 0; j < i; j++) {
        free(graph->adjacency_matrix[j]);
        free(graph->edge_exists[j]);
      }

      if  (graph->adjacency_matrix[i]) {
        free(graph->adjacency_matrix[i]);
      }
      if  (graph->edge_exists[i]) {
        free(graph->edge_exists[i]);
      }

      free(graph->adjacency_matrix);
      free(graph->edge_exists);
      return COMMC_MEMORY_ERROR;

    }

  }

  return COMMC_SUCCESS;

}

/*

         add_edge_to_list()
	       ---
	       adds an edge to the adjacency list representation.
	       handles weighted edges and duplicate checking.

*/

static commc_error_t add_edge_to_list(commc_graph_t* graph, size_t from, size_t to, double weight) {

  commc_adjacency_edge_t* edge;
  commc_list_node_t*      current;

  /* check for existing edge and update weight if found */
  current = graph->adjacency_lists[from]->head;

  while  (current) {

    edge = (commc_adjacency_edge_t*)current->data;

    if  (edge->to == to) {

      edge->weight = weight; /* update existing edge */
      return COMMC_SUCCESS;

    }

    current = current->next;

  }

  /* create new edge */
  edge = (commc_adjacency_edge_t*)malloc(sizeof(commc_adjacency_edge_t));

  if  (!edge) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  edge->to = to;
  edge->weight = weight;

  if  (!commc_list_push_back(graph->adjacency_lists[from], edge)) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    free(edge);
    return COMMC_MEMORY_ERROR;

  }

  return COMMC_SUCCESS;

}

/*

         remove_edge_from_list()
	       ---
	       removes an edge from the adjacency list representation.

*/

static void remove_edge_from_list(commc_graph_t* graph, size_t from, size_t to) {

  commc_list_node_t* current = graph->adjacency_lists[from]->head;
  commc_list_node_t* prev = NULL;

  while  (current) {

    commc_adjacency_edge_t* edge = (commc_adjacency_edge_t*)current->data;

    if  (edge->to == to) {

      /* remove this edge */

      if  (prev) {
        prev->next = current->next;
      } else {
        graph->adjacency_lists[from]->head = current->next;
      }

      if  (current->next) {
        current->next->prev = prev;
      } else {
        graph->adjacency_lists[from]->tail = prev;
      }

      graph->adjacency_lists[from]->size--;
      free(edge);
      free(current);
      break;

    }

    prev = current;
    current = current->next;

  }

}

/*

         find_edge_in_list()
	       ---
	       searches for an edge in the adjacency list representation.

*/

static commc_adjacency_edge_t* find_edge_in_list(commc_graph_t* graph, size_t from, size_t to) {

  commc_list_node_t* current = graph->adjacency_lists[from]->head;

  while  (current) {

    commc_adjacency_edge_t* edge = (commc_adjacency_edge_t*)current->data;

    if  (edge->to == to) {

      return edge;

    }

    current = current->next;

  }

  return NULL; /* edge not found */

}

/*

         dijkstra_compare()
	       ---
	       comparison function for dijkstra's algorithm priority queue.
	       returns negative if a has smaller distance than b.

*/

static int dijkstra_compare(const void* a, const void* b) {

  typedef struct {
    size_t vertex;
    double distance;
  } dijkstra_entry_t;

  const dijkstra_entry_t* entry_a;
  const dijkstra_entry_t* entry_b;

  entry_a = (const dijkstra_entry_t*)a;
  entry_b = (const dijkstra_entry_t*)b;

  if  (entry_a->distance < entry_b->distance) {
    return -1;
  }

  if  (entry_a->distance > entry_b->distance) {
    return 1;
  }

  return 0;

}

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_graph_create()
	       ---
	       allocates and initializes a graph with the specified
	       properties and internal representation.

*/

commc_graph_t* commc_graph_create(size_t vertex_count, commc_graph_type_t type, commc_graph_representation_t representation) {

  commc_graph_t* graph;
  commc_error_t  result;

  if  (vertex_count == 0) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  graph = (commc_graph_t*)malloc(sizeof(commc_graph_t));

  if  (!graph) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  graph->vertex_count      = vertex_count;
  graph->edge_count        = 0;
  graph->type              = type;
  graph->representation    = representation;
  graph->adjacency_lists   = NULL;
  graph->adjacency_matrix  = NULL;
  graph->edge_exists       = NULL;

  /* initialize chosen representation */

  if  (representation == COMMC_GRAPH_ADJACENCY_LIST) {

    result = create_adjacency_lists(graph);

  } else {

    result = create_adjacency_matrix(graph);

  }

  if  (result != COMMC_SUCCESS) {

    free(graph);
    return NULL;

  }

  return graph;

}

/*

         commc_graph_destroy()
	       ---
	       frees all memory associated with the graph.

*/

void commc_graph_destroy(commc_graph_t* graph) {

  size_t i;

  if  (!graph) {

    return;

  }

  /* cleanup adjacency list representation */

  if  (graph->adjacency_lists) {

    for  (i = 0; i < graph->vertex_count; i++) {

      commc_list_node_t* current = graph->adjacency_lists[i]->head;

      while  (current) {

        free(current->data); /* free edge structure */
        current = current->next;

      }

      commc_list_destroy(graph->adjacency_lists[i]);

    }

    free(graph->adjacency_lists);

  }

  /* cleanup adjacency matrix representation */

  if  (graph->adjacency_matrix) {

    for  (i = 0; i < graph->vertex_count; i++) {

      free(graph->adjacency_matrix[i]);

    }

    free(graph->adjacency_matrix);

  }

  if  (graph->edge_exists) {

    for  (i = 0; i < graph->vertex_count; i++) {

      free(graph->edge_exists[i]);

    }

    free(graph->edge_exists);

  }

  free(graph);

}

/*

         commc_graph_add_edge()
	       ---
	       adds a weighted edge between two vertices.
	       handles both directed and undirected graphs.

*/

commc_error_t commc_graph_add_edge(commc_graph_t* graph, size_t from, size_t to, double weight) {

  commc_error_t result = COMMC_SUCCESS;
  int           was_new_edge = 0;

  if  (!graph || from >= graph->vertex_count || to >= graph->vertex_count) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* add edge based on representation */

  if  (graph->representation == COMMC_GRAPH_ADJACENCY_LIST) {

    size_t old_edge_count = graph->edge_count;

    result = add_edge_to_list(graph, from, to, weight);

    if  (result != COMMC_SUCCESS) {

      return result;

    }

    /* check if this was a new edge */
    was_new_edge = (find_edge_in_list(graph, from, to) != NULL);

    /* for undirected graphs, add reverse edge */

    if  (graph->type == COMMC_GRAPH_UNDIRECTED && from != to) {

      result = add_edge_to_list(graph, to, from, weight);

      if  (result != COMMC_SUCCESS) {

        /* cleanup forward edge on failure */
        remove_edge_from_list(graph, from, to);
        return result;

      }

    }

    /* increment edge count only for new edges */

    if  (was_new_edge && graph->edge_count == old_edge_count) {

      graph->edge_count++;

    }

  } else {

    /* adjacency matrix representation */
    was_new_edge = !graph->edge_exists[from][to];

    graph->adjacency_matrix[from][to] = weight;
    graph->edge_exists[from][to] = 1;

    /* for undirected graphs, set reverse edge */

    if  (graph->type == COMMC_GRAPH_UNDIRECTED && from != to) {

      graph->adjacency_matrix[to][from] = weight;
      graph->edge_exists[to][from] = 1;

    }

    if  (was_new_edge) {

      graph->edge_count++;

    }

  }

  return COMMC_SUCCESS;

}

/*

         commc_graph_add_edge_unweighted()
	       ---
	       convenience function for adding unweighted edges.

*/

commc_error_t commc_graph_add_edge_unweighted(commc_graph_t* graph, size_t from, size_t to) {

  return commc_graph_add_edge(graph, from, to, 1.0);

}

/*

         commc_graph_remove_edge()
	       ---
	       removes an edge between two vertices.

*/

commc_error_t commc_graph_remove_edge(commc_graph_t* graph, size_t from, size_t to) {

  int had_edge;

  if  (!graph || from >= graph->vertex_count || to >= graph->vertex_count) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  if  (graph->representation == COMMC_GRAPH_ADJACENCY_LIST) {

    had_edge = (find_edge_in_list(graph, from, to) != NULL);

    remove_edge_from_list(graph, from, to);

    /* for undirected graphs, remove reverse edge */

    if  (graph->type == COMMC_GRAPH_UNDIRECTED && from != to) {

      remove_edge_from_list(graph, to, from);

    }

  } else {

    /* adjacency matrix representation */
    had_edge = graph->edge_exists[from][to];

    graph->edge_exists[from][to] = 0;
    graph->adjacency_matrix[from][to] = 0.0;

    /* for undirected graphs, clear reverse edge */

    if  (graph->type == COMMC_GRAPH_UNDIRECTED && from != to) {

      graph->edge_exists[to][from] = 0;
      graph->adjacency_matrix[to][from] = 0.0;

    }

  }

  if  (had_edge) {

    graph->edge_count--;

  }

  return COMMC_SUCCESS;

}

/*

         commc_graph_has_edge()
	       ---
	       checks if an edge exists between two vertices.

*/

int commc_graph_has_edge(commc_graph_t* graph, size_t from, size_t to) {

  if  (!graph || from >= graph->vertex_count || to >= graph->vertex_count) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

  if  (graph->representation == COMMC_GRAPH_ADJACENCY_LIST) {

    return (find_edge_in_list(graph, from, to) != NULL);

  } else {

    return graph->edge_exists[from][to];

  }

}

/*

         commc_graph_get_edge_weight()
	       ---
	       retrieves the weight of an edge if it exists.

*/

double commc_graph_get_edge_weight(commc_graph_t* graph, size_t from, size_t to) {

  commc_adjacency_edge_t* edge;

  if  (!graph || from >= graph->vertex_count || to >= graph->vertex_count) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return -1.0;

  }

  if  (graph->representation == COMMC_GRAPH_ADJACENCY_LIST) {

    edge = find_edge_in_list(graph, from, to);
    return edge ? edge->weight : -1.0;

  } else {

    return graph->edge_exists[from][to] ? graph->adjacency_matrix[from][to] : -1.0;

  }

}

/*

         commc_graph_set_edge_weight()
	       ---
	       updates the weight of an existing edge or creates it.

*/

commc_error_t commc_graph_set_edge_weight(commc_graph_t* graph, size_t from, size_t to, double weight) {

  return commc_graph_add_edge(graph, from, to, weight);

}

/*

         commc_graph_get_vertex_count()
	       ---
	       returns the number of vertices in the graph.

*/

size_t commc_graph_get_vertex_count(commc_graph_t* graph) {

  return graph ? graph->vertex_count : 0;

}

/*

         commc_graph_get_edge_count()
	       ---
	       returns the total number of edges in the graph.

*/

size_t commc_graph_get_edge_count(commc_graph_t* graph) {

  return graph ? graph->edge_count : 0;

}

/*

         commc_graph_get_vertex_degree()
	       ---
	       returns the out-degree of a vertex.

*/

size_t commc_graph_get_vertex_degree(commc_graph_t* graph, size_t vertex) {

  size_t degree = 0;
  size_t i;

  if  (!graph || vertex >= graph->vertex_count) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

  if  (graph->representation == COMMC_GRAPH_ADJACENCY_LIST) {

    return commc_list_size(graph->adjacency_lists[vertex]);

  } else {

    /* count edges in matrix row */

    for  (i = 0; i < graph->vertex_count; i++) {

      if  (graph->edge_exists[vertex][i]) {

        degree++;

      }

    }

  }

  return degree;

}

/*

         commc_graph_get_vertex_in_degree()
	       ---
	       returns the in-degree of a vertex.

*/

size_t commc_graph_get_vertex_in_degree(commc_graph_t* graph, size_t vertex) {

  size_t in_degree = 0;
  size_t i;

  if  (!graph || vertex >= graph->vertex_count) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

  /* for undirected graphs, in-degree equals out-degree */

  if  (graph->type == COMMC_GRAPH_UNDIRECTED) {

    return commc_graph_get_vertex_degree(graph, vertex);

  }

  if  (graph->representation == COMMC_GRAPH_ADJACENCY_LIST) {

    /* count incoming edges by scanning all adjacency lists */

    for  (i = 0; i < graph->vertex_count; i++) {

      if  (find_edge_in_list(graph, i, vertex) != NULL) {

        in_degree++;

      }

    }

  } else {

    /* count edges in matrix column */

    for  (i = 0; i < graph->vertex_count; i++) {

      if  (graph->edge_exists[i][vertex]) {

        in_degree++;

      }

    }

  }

  return in_degree;

}

/*

         commc_graph_is_directed()
	       ---
	       returns 1 if graph is directed, 0 if undirected.

*/

int commc_graph_is_directed(commc_graph_t* graph) {

  return graph ? (graph->type == COMMC_GRAPH_DIRECTED) : 0;

}

/*

         commc_graph_get_representation()
	       ---
	       returns the internal representation type.

*/

commc_graph_representation_t commc_graph_get_representation(commc_graph_t* graph) {

  return graph ? graph->representation : COMMC_GRAPH_ADJACENCY_LIST;

}

/*

         commc_graph_clear()
	       ---
	       removes all edges while preserving graph structure.

*/

void commc_graph_clear(commc_graph_t* graph) {

  size_t i, j;

  if  (!graph) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  if  (graph->representation == COMMC_GRAPH_ADJACENCY_LIST) {

    for  (i = 0; i < graph->vertex_count; i++) {

      commc_list_clear(graph->adjacency_lists[i]);

    }

  } else {

    for  (i = 0; i < graph->vertex_count; i++) {

      for  (j = 0; j < graph->vertex_count; j++) {

        graph->edge_exists[i][j] = 0;
        graph->adjacency_matrix[i][j] = 0.0;

      }

    }

  }

  graph->edge_count = 0;

}

/*

         commc_graph_get_neighbors()
	       ---
	       creates an iterator for traversing neighbors of a vertex.

*/

commc_graph_iterator_t* commc_graph_get_neighbors(commc_graph_t* graph, size_t vertex) {

  commc_graph_iterator_t* iterator;

  if  (!graph || vertex >= graph->vertex_count) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  iterator = (commc_graph_iterator_t*)malloc(sizeof(commc_graph_iterator_t));

  if  (!iterator) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  iterator->graph = graph;
  iterator->current_vertex = vertex;
  iterator->iterating_edges = 0; /* neighbors mode */
  iterator->matrix_row = vertex;
  iterator->matrix_col = 0;

  if  (graph->representation == COMMC_GRAPH_ADJACENCY_LIST) {

    iterator->list_iterator = commc_list_begin(graph->adjacency_lists[vertex]);

  }

  return iterator;

}

/*

         commc_graph_get_edges()
	       ---
	       creates an iterator for traversing all edges in the graph.

*/

commc_graph_iterator_t* commc_graph_get_edges(commc_graph_t* graph) {

  commc_graph_iterator_t* iterator;

  if  (!graph) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  iterator = (commc_graph_iterator_t*)malloc(sizeof(commc_graph_iterator_t));

  if  (!iterator) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  iterator->graph = graph;
  iterator->current_vertex = 0;
  iterator->iterating_edges = 1; /* edges mode */
  iterator->matrix_row = 0;
  iterator->matrix_col = 0;

  if  (graph->representation == COMMC_GRAPH_ADJACENCY_LIST && graph->vertex_count > 0) {

    iterator->list_iterator = commc_list_begin(graph->adjacency_lists[0]);

  }

  return iterator;

}

/*

         commc_graph_iterator_has_next()
	       ---
	       checks if iterator has more items to return.

*/

int commc_graph_iterator_has_next(commc_graph_iterator_t* iterator) {

  if  (!iterator || !iterator->graph) {

    return 0;

  }

  if  (iterator->graph->representation == COMMC_GRAPH_ADJACENCY_LIST) {

    if  (iterator->iterating_edges) {

      /* check current list iterator first */

      if  (commc_list_iterator_data(&iterator->list_iterator) != NULL) {

        return 1;

      }

      /* check if there are more vertices to iterate */

      while  (iterator->current_vertex + 1 < iterator->graph->vertex_count) {

        iterator->current_vertex++;
        
        iterator->list_iterator = commc_list_begin(iterator->graph->adjacency_lists[iterator->current_vertex]);

        if  (commc_list_iterator_data(&iterator->list_iterator) != NULL) {

          return 1;

        }

      }

      return 0;

    } else {

      /* neighbor iteration */
      return commc_list_iterator_data(&iterator->list_iterator) != NULL;

    }

  } else {

    /* adjacency matrix representation */

    if  (iterator->iterating_edges) {

      /* find next edge in matrix */

      while  (iterator->matrix_row < iterator->graph->vertex_count) {

        while  (iterator->matrix_col < iterator->graph->vertex_count) {

          if  (iterator->graph->edge_exists[iterator->matrix_row][iterator->matrix_col]) {

            return 1;

          }

          iterator->matrix_col++;

        }

        iterator->matrix_row++;
        iterator->matrix_col = 0;

      }

      return 0;

    } else {

      /* neighbor iteration */

      while  (iterator->matrix_col < iterator->graph->vertex_count) {

        if  (iterator->graph->edge_exists[iterator->matrix_row][iterator->matrix_col]) {

          return 1;

        }

        iterator->matrix_col++;

      }

      return 0;

    }

  }

}

/*

         commc_graph_iterator_next()
	       ---
	       returns the next edge from the iterator.

*/

commc_graph_edge_t* commc_graph_iterator_next(commc_graph_iterator_t* iterator) {

  commc_adjacency_edge_t* list_edge;

  if  (!iterator || !iterator->graph || !commc_graph_iterator_has_next(iterator)) {

    return NULL;

  }

  if  (iterator->graph->representation == COMMC_GRAPH_ADJACENCY_LIST) {

    list_edge = (commc_adjacency_edge_t*)commc_list_iterator_data(&iterator->list_iterator);

    if  (list_edge) {

      iterator->current_edge.from = iterator->current_vertex;
      iterator->current_edge.to = list_edge->to;
      iterator->current_edge.weight = list_edge->weight;

      /* advance iterator */
      commc_list_next(&iterator->list_iterator);

      return &iterator->current_edge;

    }

  } else {

    /* adjacency matrix representation */

    iterator->current_edge.from = iterator->matrix_row;
    iterator->current_edge.to = iterator->matrix_col;
    iterator->current_edge.weight = iterator->graph->adjacency_matrix[iterator->matrix_row][iterator->matrix_col];

    iterator->matrix_col++; /* advance to next position */

    return &iterator->current_edge;

  }

  return NULL;

}

/*

         commc_graph_iterator_destroy()
	       ---
	       frees memory associated with a graph iterator.

*/

void commc_graph_iterator_destroy(commc_graph_iterator_t* iterator) {

  if  (!iterator) {

    return;

  }

  /* no need to free list_iterator - it's now a value type */

  free(iterator);

}

/*

         commc_graph_copy()
	       ---
	       creates a deep copy of the graph with optional representation change.

*/

commc_graph_t* commc_graph_copy(commc_graph_t* source, commc_graph_representation_t new_representation) {

  commc_graph_t*          new_graph;
  commc_graph_iterator_t* edge_iterator;
  commc_graph_edge_t*     edge;

  if  (!source) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  /* create new graph with desired representation */
  new_graph = commc_graph_create(source->vertex_count, source->type, new_representation);

  if  (!new_graph) {

    return NULL;

  }

  /* copy all edges from source to new graph */
  edge_iterator = commc_graph_get_edges(source);

  if  (!edge_iterator) {

    commc_graph_destroy(new_graph);
    return NULL;

  }

  while  (commc_graph_iterator_has_next(edge_iterator)) {

    edge = commc_graph_iterator_next(edge_iterator);

    if  (edge) {

      commc_error_t result = commc_graph_add_edge(new_graph, edge->from, edge->to, edge->weight);

      if  (result != COMMC_SUCCESS) {

        commc_graph_iterator_destroy(edge_iterator);
        commc_graph_destroy(new_graph);
        return NULL;

      }

    }

  }

  commc_graph_iterator_destroy(edge_iterator);

  return new_graph;

}

/*
	==================================
             --- ALGORITHMS ---
	==================================
*/

/*

         commc_graph_bfs()
	       ---
	       breadth-first search (BFS) is a fundamental graph
	       traversal algorithm that explores vertices level by level.
	       
	       it uses a queue to maintain the frontier of vertices to visit,
	       ensuring that vertices closer to the source are visited first.
	       this property makes BFS ideal for finding shortest paths in
	       unweighted graphs and detecting connected components.

*/

commc_error_t commc_graph_bfs(commc_graph_t* graph, size_t start_vertex, 
                              void (*visit_func)(size_t vertex, void* user_data), 
                              void* user_data) {

  char*           visited;
  commc_queue_t*  queue;
  size_t*         vertex_ptr;
  size_t          current_vertex;
  commc_graph_iterator_t* neighbor_iter;
  commc_graph_edge_t*     edge;

  if  (!graph || !visit_func || start_vertex >= graph->vertex_count) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* create visited tracking array */
  visited = (char*)calloc(graph->vertex_count, sizeof(char));

  if  (!visited) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  /* create BFS queue */
  queue = commc_queue_create();

  if  (!queue) {

    free(visited);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  /* enqueue starting vertex */
  vertex_ptr = (size_t*)malloc(sizeof(size_t));

  if  (!vertex_ptr) {

    free(visited);
    commc_queue_destroy(queue);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  *vertex_ptr = start_vertex;
  commc_queue_enqueue(queue, vertex_ptr);
  visited[start_vertex] = 1;

  /* main BFS loop */

  while  (!commc_queue_is_empty(queue)) {

    vertex_ptr = (size_t*)commc_queue_dequeue(queue);
    current_vertex = *vertex_ptr;
    free(vertex_ptr);

    /* visit current vertex */
    visit_func(current_vertex, user_data);

    /* examine all neighbors */
    neighbor_iter = commc_graph_get_neighbors(graph, current_vertex);

    if  (neighbor_iter) {

      while  (commc_graph_iterator_has_next(neighbor_iter)) {

        edge = commc_graph_iterator_next(neighbor_iter);

        if  (edge && !visited[edge->to]) {

          visited[edge->to] = 1;

          vertex_ptr = (size_t*)malloc(sizeof(size_t));

          if  (vertex_ptr) {

            *vertex_ptr = edge->to;
            commc_queue_enqueue(queue, vertex_ptr);

          }

        }

      }

      commc_graph_iterator_destroy(neighbor_iter);

    }

  }

  free(visited);
  commc_queue_destroy(queue);

  return COMMC_SUCCESS;

}

/*

         commc_graph_dfs()
	       ---
	       depth-first search (DFS) is a graph traversal algorithm
	       that explores as far as possible along each branch before
	       backtracking. unlike BFS which uses a queue, DFS typically
	       uses a stack (or recursion).
	       
	       DFS is particularly useful for:
	       - detecting cycles in graphs
	       - topological sorting of directed acyclic graphs
	       - finding strongly connected components
	       - maze solving and pathfinding with obstacles

*/

commc_error_t commc_graph_dfs(commc_graph_t* graph, size_t start_vertex, 
                              void (*visit_func)(size_t vertex, void* user_data), 
                              void* user_data) {

  char*           visited;
  commc_stack_t*  stack;
  size_t*         vertex_ptr;
  size_t          current_vertex;
  commc_graph_iterator_t* neighbor_iter;
  commc_graph_edge_t*     edge;

  if  (!graph || !visit_func || start_vertex >= graph->vertex_count) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* create visited tracking array */
  visited = (char*)calloc(graph->vertex_count, sizeof(char));

  if  (!visited) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  /* create DFS stack */
  stack = commc_stack_create();

  if  (!stack) {

    free(visited);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  /* push starting vertex */
  vertex_ptr = (size_t*)malloc(sizeof(size_t));

  if  (!vertex_ptr) {

    free(visited);
    commc_stack_destroy(stack);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  *vertex_ptr = start_vertex;
  commc_stack_push(stack, vertex_ptr);

  /* main DFS loop */

  while  (!commc_stack_is_empty(stack)) {

    vertex_ptr = (size_t*)commc_stack_pop(stack);
    current_vertex = *vertex_ptr;
    free(vertex_ptr);

    if  (!visited[current_vertex]) {

      visited[current_vertex] = 1;

      /* visit current vertex */
      visit_func(current_vertex, user_data);

      /* push all unvisited neighbors onto stack */
      neighbor_iter = commc_graph_get_neighbors(graph, current_vertex);

      if  (neighbor_iter) {

        while  (commc_graph_iterator_has_next(neighbor_iter)) {

          edge = commc_graph_iterator_next(neighbor_iter);

          if  (edge && !visited[edge->to]) {

            vertex_ptr = (size_t*)malloc(sizeof(size_t));

            if  (vertex_ptr) {

              *vertex_ptr = edge->to;
              commc_stack_push(stack, vertex_ptr);

            }

          }

        }

        commc_graph_iterator_destroy(neighbor_iter);

      }

    }

  }

  free(visited);
  commc_stack_destroy(stack);

  return COMMC_SUCCESS;

}

/*

         commc_graph_dijkstra()
	       ---
	       dijkstra's algorithm finds the shortest paths from a source
	       vertex to all other vertices in a weighted graph with
	       non-negative edge weights.
	       
	       the algorithm maintains a priority queue of vertices,
	       always processing the vertex with the smallest tentative
	       distance first. this greedy approach guarantees optimal
	       solutions for graphs without negative weights.
	       
	       time complexity: O((V + E) log V) with a binary heap
	       space complexity: O(V)
	       
	       returns an array of distances where index represents
	       the vertex and value represents shortest distance from source.

*/

double* commc_graph_dijkstra(commc_graph_t* graph, size_t source) {

  double*                 distances;
  char*                   visited;
  commc_priority_queue_t* pq;
  size_t                  i;
  size_t                  current_vertex;
  double                  current_distance;
  commc_graph_iterator_t* neighbor_iter;
  commc_graph_edge_t*     edge;
  double                  new_distance;
  
  /* dijkstra vertex entry for priority queue */
  typedef struct {
    size_t vertex;
    double distance;
  } dijkstra_entry_t;

  dijkstra_entry_t* entry;
  dijkstra_entry_t* current_entry;

  if  (!graph || source >= graph->vertex_count) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  /* initialize distance array */
  distances = (double*)malloc(graph->vertex_count * sizeof(double));

  if  (!distances) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  /* initialize visited array */
  visited = (char*)calloc(graph->vertex_count, sizeof(char));

  if  (!visited) {

    free(distances);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  /* create priority queue with distance comparison */
  pq = commc_priority_queue_create(16, dijkstra_compare);

  if  (!pq) {

    free(distances);
    free(visited);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  /* initialize all distances to infinity except source */

  for  (i = 0; i < graph->vertex_count; i++) {

    distances[i] = (i == source) ? 0.0 : HUGE_VAL;

  }

  /* add source vertex to priority queue */
  entry = (dijkstra_entry_t*)malloc(sizeof(dijkstra_entry_t));

  if  (!entry) {

    free(distances);
    free(visited);
    commc_priority_queue_destroy(pq);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  entry->vertex = source;
  entry->distance = 0.0;
  
  commc_priority_queue_insert(pq, entry);

  /* main dijkstra loop */

  while  (!commc_priority_queue_is_empty(pq)) {

    current_entry = (dijkstra_entry_t*)commc_priority_queue_extract(pq);
    current_vertex = current_entry->vertex;
    current_distance = current_entry->distance;
    free(current_entry);

    if  (visited[current_vertex]) {

      continue; /* already processed this vertex */

    }

    visited[current_vertex] = 1;

    /* examine all neighbors */
    neighbor_iter = commc_graph_get_neighbors(graph, current_vertex);

    if  (neighbor_iter) {

      while  (commc_graph_iterator_has_next(neighbor_iter)) {

        edge = commc_graph_iterator_next(neighbor_iter);

        if  (edge && !visited[edge->to]) {

          new_distance = current_distance + edge->weight;

          if  (new_distance < distances[edge->to]) {

            distances[edge->to] = new_distance;

            /* add updated vertex to priority queue */
            entry = (dijkstra_entry_t*)malloc(sizeof(dijkstra_entry_t));

            if  (entry) {

              entry->vertex = edge->to;
              entry->distance = new_distance;
              commc_priority_queue_insert(pq, entry);

            }

          }

        }

      }

      commc_graph_iterator_destroy(neighbor_iter);

    }

  }

  free(visited);
  commc_priority_queue_destroy(pq);

  return distances;

}

/*
	==================================
             --- EOF ---
	==================================
*/