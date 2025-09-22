/*
   ===================================
   C O M M O N - C
   GRAPH MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- GRAPH MODULE ---

    this module provides graph data structure implementations
    supporting both directed and undirected graphs with two
    internal representation methods:

    1. adjacency list - efficient for sparse graphs
       - space: O(V + E) where V=vertices, E=edges  
       - edge queries: O(degree of vertex)
       - ideal for most real-world graphs

    2. adjacency matrix - efficient for dense graphs
       - space: O(V²) regardless of edge count
       - edge queries: O(1) constant time
       - ideal when edge density is high

    vertices are identified by integer indices from 0 to (V-1).
    edges can have optional weights for shortest-path algorithms.
    both weighted and unweighted graph variants are supported.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef  COMMC_GRAPH_H
#define  COMMC_GRAPH_H

#include  <stddef.h>
#include  "error.h"

/*
	==================================
             --- ENUMS ---
	==================================
*/

/*

         commc_graph_type_t
	       ---
	       specifies whether the graph is directed or undirected.
	       this affects how edges are stored and traversed.

*/

typedef enum {

  COMMC_GRAPH_UNDIRECTED = 0,  /* edges work both ways */
  COMMC_GRAPH_DIRECTED   = 1   /* edges have direction */

} commc_graph_type_t;

/*

         commc_graph_representation_t
	       ---
	       specifies the internal storage method for the graph.
	       each has different space/time trade-offs.

*/

typedef enum {

  COMMC_GRAPH_ADJACENCY_LIST   = 0,  /* list of neighbors per vertex */
  COMMC_GRAPH_ADJACENCY_MATRIX = 1   /* 2D matrix of connections */

} commc_graph_representation_t;

/*
	==================================
             --- STRUCTS ---
	==================================
*/

typedef struct commc_graph_t commc_graph_t;

/*

         commc_graph_edge_t
	       ---
	       represents a single edge in the graph with optional weight.
	       used for returning edge information and iteration.

*/

typedef struct {

  size_t from;      /* source vertex index */
  size_t to;        /* destination vertex index */
  double weight;    /* edge weight (1.0 for unweighted graphs) */

} commc_graph_edge_t;

/*

         commc_graph_iterator_t
	       ---
	       iterator for traversing graph edges or neighbors.
	       provides consistent iteration interface regardless
	       of internal representation.

*/

typedef struct commc_graph_iterator_t commc_graph_iterator_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_graph_create()
	       ---
	       creates a new graph with specified number of vertices,
	       directedness, and internal representation method.

*/

commc_graph_t* commc_graph_create(size_t vertex_count, commc_graph_type_t type, commc_graph_representation_t representation);

/*

         commc_graph_destroy()
	       ---
	       frees all memory associated with the graph structure.

*/

void commc_graph_destroy(commc_graph_t* graph);

/*

         commc_graph_add_edge()
	       ---
	       adds an edge between two vertices with optional weight.
	       for undirected graphs, creates bidirectional connection.

*/

commc_error_t commc_graph_add_edge(commc_graph_t* graph, size_t from, size_t to, double weight);

/*

         commc_graph_add_edge_unweighted()
	       ---
	       convenience function for adding unweighted edges.
	       automatically sets weight to 1.0.

*/

commc_error_t commc_graph_add_edge_unweighted(commc_graph_t* graph, size_t from, size_t to);

/*

         commc_graph_remove_edge()
	       ---
	       removes an edge between two vertices.
	       for undirected graphs, removes both directions.

*/

commc_error_t commc_graph_remove_edge(commc_graph_t* graph, size_t from, size_t to);

/*

         commc_graph_has_edge()
	       ---
	       checks if an edge exists between two vertices.
	       returns 1 if edge exists, 0 otherwise.

*/

int commc_graph_has_edge(commc_graph_t* graph, size_t from, size_t to);

/*

         commc_graph_get_edge_weight()
	       ---
	       retrieves the weight of an edge between two vertices.
	       returns -1.0 if edge does not exist.

*/

double commc_graph_get_edge_weight(commc_graph_t* graph, size_t from, size_t to);

/*

         commc_graph_set_edge_weight()
	       ---
	       updates the weight of an existing edge.
	       creates the edge if it doesn't exist.

*/

commc_error_t commc_graph_set_edge_weight(commc_graph_t* graph, size_t from, size_t to, double weight);

/*

         commc_graph_get_vertex_count()
	       ---
	       returns the number of vertices in the graph.

*/

size_t commc_graph_get_vertex_count(commc_graph_t* graph);

/*

         commc_graph_get_edge_count()
	       ---
	       returns the total number of edges in the graph.
	       for undirected graphs, counts each connection once.

*/

size_t commc_graph_get_edge_count(commc_graph_t* graph);

/*

         commc_graph_get_vertex_degree()
	       ---
	       returns the degree (number of connections) of a vertex.
	       for directed graphs, returns out-degree.

*/

size_t commc_graph_get_vertex_degree(commc_graph_t* graph, size_t vertex);

/*

         commc_graph_get_vertex_in_degree()
	       ---
	       returns the in-degree of a vertex in directed graphs.
	       for undirected graphs, same as regular degree.

*/

size_t commc_graph_get_vertex_in_degree(commc_graph_t* graph, size_t vertex);

/*

         commc_graph_is_directed()
	       ---
	       returns 1 if graph is directed, 0 if undirected.

*/

int commc_graph_is_directed(commc_graph_t* graph);

/*

         commc_graph_get_representation()
	       ---
	       returns the internal representation type being used.

*/

commc_graph_representation_t commc_graph_get_representation(commc_graph_t* graph);

/*

         commc_graph_get_neighbors()
	       ---
	       creates an iterator for traversing neighbors of a vertex.
	       caller must free the iterator when done.

*/

commc_graph_iterator_t* commc_graph_get_neighbors(commc_graph_t* graph, size_t vertex);

/*

         commc_graph_get_edges()
	       ---
	       creates an iterator for traversing all edges in the graph.
	       caller must free the iterator when done.

*/

commc_graph_iterator_t* commc_graph_get_edges(commc_graph_t* graph);

/*

         commc_graph_iterator_has_next()
	       ---
	       returns 1 if iterator has more items, 0 otherwise.

*/

int commc_graph_iterator_has_next(commc_graph_iterator_t* iterator);

/*

         commc_graph_iterator_next()
	       ---
	       returns the next edge from the iterator.
	       returns NULL if no more items exist.

*/

commc_graph_edge_t* commc_graph_iterator_next(commc_graph_iterator_t* iterator);

/*

         commc_graph_iterator_destroy()
	       ---
	       frees memory associated with a graph iterator.

*/

void commc_graph_iterator_destroy(commc_graph_iterator_t* iterator);

/*

         commc_graph_clear()
	       ---
	       removes all edges from the graph while keeping vertices.
	       graph structure remains valid for reuse.

*/

void commc_graph_clear(commc_graph_t* graph);

/*

         commc_graph_copy()
	       ---
	       creates a deep copy of the graph with same structure.
	       allows changing representation during copying.

*/

commc_graph_t* commc_graph_copy(commc_graph_t* source, commc_graph_representation_t new_representation);

/*
	==================================
             --- ALGORITHMS ---
	==================================
*/

/*

         commc_graph_bfs()
	       ---
	       breadth-first search traversal from starting vertex.
	       visits vertices level by level, useful for finding 
	       shortest paths in unweighted graphs and connected components.

*/

commc_error_t commc_graph_bfs(commc_graph_t* graph, size_t start_vertex, 
                              void (*visit_func)(size_t vertex, void* user_data), 
                              void* user_data);

/*

         commc_graph_dfs()
	       ---
	       depth-first search traversal from starting vertex.
	       explores as far as possible along each branch before
	       backtracking, useful for cycle detection and topological sorting.

*/

commc_error_t commc_graph_dfs(commc_graph_t* graph, size_t start_vertex, 
                              void (*visit_func)(size_t vertex, void* user_data), 
                              void* user_data);

/*

         commc_graph_dijkstra()
	       ---
	       finds shortest paths from source vertex to all other vertices
	       using dijkstra's algorithm. requires non-negative edge weights.
	       returns array of distances, caller must free.

*/

double* commc_graph_dijkstra(commc_graph_t* graph, size_t source);

#endif /* COMMC_GRAPH_H */

/*
	==================================
             --- EOF ---
	==================================
*/