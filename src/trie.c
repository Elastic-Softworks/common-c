/*

    ===================================
    T R I E   I M P L E M E N T A T I O N
    ELASTIC SOFTWORKS 2025
    ===================================

*/

/*

         trie.c - prefix tree implementation for string operations
           ---
           this implementation provides a complete trie (prefix tree) data
           structure optimized for string storage, retrieval, and prefix-based
           operations. the trie excels at dictionary applications, auto-complete
           systems, and any scenario requiring efficient prefix matching.
           
           algorithmic characteristics:
           - insertion: O(m) where m is string length
           - search: O(m) where m is string length  
           - deletion: O(m) where m is string length
           - space: O(alphabet_size * n * m) worst case, but shared prefixes
             provide significant memory savings in practice
           
           the implementation uses direct ASCII indexing for character mapping,
           trading memory overhead for simplicity and consistent performance.
           each node maintains a 256-element array of child pointers, allowing
           O(1) character transitions during traversal operations.

*/

/* 
	==================================
             --- INCLUDES ---
	==================================
*/

#include "commc/trie.h"
#include "commc/error.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* 
	==================================
             --- PRIVATE HELPERS ---
	==================================
*/

/*

         create_node()
           ---
           allocates and initializes a new trie node.
           
           creates a new node with all child pointers set to NULL and
           the end_of_word flag set to false. this represents an intermediate
           node in the trie that may become a word ending or branch point
           as strings are inserted.
           
           returns NULL if memory allocation fails, otherwise returns
           a pointer to the initialized node ready for insertion into the trie.

*/

static commc_trie_node_t*  create_node(void) {

  commc_trie_node_t*  node;
  int                 i;

  node = (commc_trie_node_t*)malloc(sizeof(commc_trie_node_t));

  if  (node == NULL) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
    
  }

  /* initialize all child pointers to NULL and mark as non-terminal */
  
  for  (i = 0; i < COMMC_TRIE_ASCII_SIZE; i++) {

    node->children[i] = NULL;
    
  }

  node->is_end_of_word = false;

  return node;
  
}

/*

         destroy_node()
           ---
           recursively destroys a trie node and all its descendants.
           
           performs a depth-first traversal to free all child nodes before
           freeing the current node. this ensures complete cleanup of the
           trie subtree rooted at the given node without memory leaks.
           
           handles NULL nodes safely, making it suitable for recursive
           cleanup of sparse trie structures where many child pointers
           may be NULL.

*/

static void  destroy_node(commc_trie_node_t* node) {

  int  i;

  if  (node == NULL) {

    return;
    
  }

  /* recursively destroy all child nodes first */
  
  for  (i = 0; i < COMMC_TRIE_ASCII_SIZE; i++) {

    if  (node->children[i] != NULL) {

      destroy_node(node->children[i]);
      
    }
    
  }

  /* free the current node after all children are destroyed */
  
  free(node);
  
}

/*

         has_children()
           ---
           checks whether a trie node has any child nodes.
           
           scans the children array to determine if the node has any
           non-NULL child pointers. used during deletion operations
           to determine if a node can be safely removed without
           affecting other stored strings.
           
           returns true if at least one child exists, false if the
           node is a leaf with no children. NULL nodes return false.

*/

static bool  has_children(const commc_trie_node_t* node) {

  int  i;

  if  (node == NULL) {

    return false;
    
  }

  for  (i = 0; i < COMMC_TRIE_ASCII_SIZE; i++) {

    if  (node->children[i] != NULL) {

      return true;
      
    }
    
  }

  return false;
  
}

/*

         delete_helper()
           ---
           recursive helper function for string deletion with cleanup.
           
           traverses the trie to find the target string, then unmarks
           the end_of_word flag. during the return path, removes nodes
           that are no longer needed (no children and not end of other words).
           
           this cleanup optimization prevents memory waste from deleted
           strings while preserving the trie structure needed for remaining
           strings. the recursive approach naturally handles the bottom-up
           cleanup required for proper node removal.
           
           returns true if the current node should be deleted by its parent,
           false if the node should be preserved.

*/

static bool  delete_helper(commc_trie_node_t* node, const char* string, int index) {

  unsigned char  c;
  bool           should_delete_child;

  if  (node == NULL) {

    return false;  /* STRING NOT FOUND */
    
  }

  /* base case: reached end of string */
  
  if  (string[index] == '\0') {

    /* unmark as end of word if it was marked */
    
    if  (node->is_end_of_word) {

      node->is_end_of_word = false;
      
    }

    /* return true if this node can be deleted (no children, not end of other word) */
    
    return !node->is_end_of_word && !has_children(node);
    
  }

  /* recursive case: continue traversal */
  
  c = (unsigned char)string[index];
  should_delete_child = delete_helper(node->children[c], string, index + 1);

  should_delete_child = delete_helper(node->children[c], string, index + 1);

  /* delete the child if it should be removed */
  
  if  (should_delete_child) {

    free(node->children[c]);
    node->children[c] = NULL;
    
  }

  /* return true if this node can also be deleted */
  
  return !node->is_end_of_word && !has_children(node) && node != NULL;
  
}

/*

         collect_completions()
           ---
           recursive helper for collecting all strings with a given prefix.
           
           performs depth-first traversal from the prefix location, building
           complete strings by combining the prefix with suffixes discovered
           during traversal. when a node marked as end_of_word is found,
           the current string is added to the completions array.
           
           uses a character buffer to build strings incrementally, avoiding
           repeated memory allocations during traversal. limits results
           to prevent memory exhaustion on large tries.
           
           the strings parameter is modified in-place to store results,
           and count is updated to reflect the number of completions found.

*/

static void  collect_completions(const commc_trie_node_t* node, 
                                 char* current_string, 
                                 int depth,
                                 char** strings, 
                                 size_t* count,
                                 size_t max_completions) {

  int  i;

  if  (node == NULL || *count >= max_completions) {

    return;
    
  }

  /* if this node marks end of word, add the current string to results */
  
  if  (node->is_end_of_word) {

    current_string[depth] = '\0';  /* NULL-TERMINATE THE STRING */
    strings[*count] = (char*)malloc((depth + 1) * sizeof(char));

    if  (strings[*count] != NULL) {

      strcpy(strings[*count], current_string);
      (*count)++;
      
    }
    
  }

  /* recursively traverse all children */
  
  for  (i = 0; i < COMMC_TRIE_ASCII_SIZE && *count < max_completions; i++) {

    if  (node->children[i] != NULL) {

      current_string[depth] = (char)i;  /* ADD CHARACTER TO CURRENT STRING */
      collect_completions(node->children[i], current_string, depth + 1, 
                         strings, count, max_completions);
                         
    }
    
  }
  
}

/* 
	==================================
             --- CORE OPERATIONS ---
	==================================
*/

/*

         commc_trie_create()
           ---
           creates and initializes a new empty trie data structure.
           
           allocates memory for the trie container and creates the root node
           that represents an empty string. initializes the size counter to
           zero as no strings are stored initially.
           
           the root node serves as the foundation for all string operations
           and must exist for the trie to function properly. if root creation
           fails, the entire trie creation fails to maintain consistency.

*/

commc_trie_t*  commc_trie_create(void) {

  commc_trie_t*  trie;

  trie = (commc_trie_t*)malloc(sizeof(commc_trie_t));

  if  (trie == NULL) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
    
  }

  /* create the root node */
  
  trie->root = create_node();

  if  (trie->root == NULL) {

    free(trie);
    return NULL;
    
  }

  trie->size = 0;

  return trie;
  
}

/*

         commc_trie_destroy()
           ---
           recursively destroys the trie and frees all allocated memory.
           
           uses the recursive destroy_node helper to free all nodes in
           the trie tree, then frees the trie container itself. this
           ensures complete cleanup without memory leaks.
           
           handles NULL trie pointers safely for defensive programming.

*/

void  commc_trie_destroy(commc_trie_t* trie) {

  if  (trie == NULL) {

    return;
    
  }

  destroy_node(trie->root);
  free(trie);
  
}

/* 
	==================================
             --- STRING OPERATIONS ---
	==================================
*/

/*

         commc_trie_insert()
           ---
           inserts a string into the trie, creating nodes as necessary.
           
           traverses the trie following the character path defined by the
           input string. creates new nodes for missing characters along
           the path, then marks the final node as end_of_word to indicate
           complete string storage.
           
           handles duplicate insertions gracefully - if a string already
           exists, the insertion succeeds without modification. this allows
           safe repeated insertions without error checking.

*/

commc_error_t  commc_trie_insert(commc_trie_t* trie, const char* string) {

  commc_trie_node_t*  current;
  int                 i;
  unsigned char       c;

  if  (trie == NULL || string == NULL) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
                              
  }

  current = trie->root;

  /* traverse/create the path for each character in the string */
  
  for  (i = 0; string[i] != '\0'; i++) {

    c = (unsigned char)string[i];

    /* create child node if it doesn't exist */
    
    if  (current->children[c] == NULL) {

      current->children[c] = create_node();

      if  (current->children[c] == NULL) {

        return COMMC_MEMORY_ERROR;  /* ERROR ALREADY REPORTED BY create_node() */
        
      }
      
    }

    current = current->children[c];
    
  }

  /* mark the final node as end of word and update size if new */
  
  if  (!current->is_end_of_word) {

    current->is_end_of_word = true;
    trie->size++;
    
  }

  return COMMC_SUCCESS;
  
}

/*

         commc_trie_search()
           ---
           searches for an exact string match within the trie.
           
           traverses the trie following the character path defined by
           the search string. returns true only if the complete path
           exists and the final node is marked as end_of_word.
           
           this exact matching distinguishes between stored strings and
           partial matches, which is crucial for dictionary applications.

*/

bool  commc_trie_search(const commc_trie_t* trie, const char* string) {

  const commc_trie_node_t*  current;
  int                       i;
  unsigned char             c;

  if  (trie == NULL || string == NULL) {

    return false;
    
  }

  current = trie->root;

  /* traverse the path for each character in the string */
  
  for  (i = 0; string[i] != '\0'; i++) {

    c = (unsigned char)string[i];

    if  (current->children[c] == NULL) {

      return false;  /* CHARACTER PATH DOESN'T EXIST */
      
    }

    current = current->children[c];
    
  }

  /* return true only if this represents a complete stored string */
  
  return current->is_end_of_word;
  
}

/*

         commc_trie_delete()
           ---
           removes a string from the trie, cleaning up unnecessary nodes.
           
           uses the recursive delete_helper to locate and remove the string,
           then performs bottom-up cleanup to remove nodes that are no longer
           needed. updates the size counter appropriately.
           
           deleting non-existent strings succeeds without error, making
           this function safe for defensive deletion patterns.

*/

commc_error_t  commc_trie_delete(commc_trie_t* trie, const char* string) {

  bool  string_existed;

  if  (trie == NULL || string == NULL) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
                              
  }

  /* check if string exists before attempting deletion */
  
  string_existed = commc_trie_search(trie, string);

  /* perform deletion with cleanup */
  
  delete_helper(trie->root, string, 0);

  /* update size if string was actually deleted */
  
  if  (string_existed) {

    trie->size--;
    
  }

  return COMMC_SUCCESS;
  
}

/* 
	==================================
             --- PREFIX OPERATIONS ---
	==================================
*/

/*

         commc_trie_has_prefix()
           ---
           checks whether any stored strings begin with the given prefix.
           
           traverses the trie following the prefix character path. returns
           true if the complete path exists, indicating that at least one
           stored string shares this prefix. the prefix itself need not
           be a complete stored string.

*/

bool  commc_trie_has_prefix(const commc_trie_t* trie, const char* prefix) {

  const commc_trie_node_t*  current;
  int                       i;
  unsigned char             c;

  if  (trie == NULL || prefix == NULL) {

    return false;
    
  }

  current = trie->root;

  /* traverse the path for each character in the prefix */
  
  for  (i = 0; prefix[i] != '\0'; i++) {

    c = (unsigned char)prefix[i];

    if  (current->children[c] == NULL) {

      return false;  /* PREFIX PATH DOESN'T EXIST */
      
    }

    current = current->children[c];
    
  }

  /* if we reached here, the prefix path exists */
  
  return true;
  
}

/*

         commc_trie_get_completions()
           ---
           finds all strings that begin with the specified prefix.
           
           first navigates to the prefix location in the trie, then uses
           the collect_completions helper to gather all complete strings
           extending from that point. combines the original prefix with
           discovered suffixes to form complete result strings.
           
           allocates memory for results which must be freed by the caller
           using commc_trie_free_completions(). limits results for safety.

*/

commc_trie_completions_t*  commc_trie_get_completions(const commc_trie_t* trie, const char* prefix) {

  commc_trie_completions_t*  completions;
  const commc_trie_node_t*   current;
  char*                      temp_string;
  char**                     strings;
  int                        i;
  unsigned char              c;
  size_t                     prefix_len;
  size_t                     count;

  if  (trie == NULL || prefix == NULL) {

    return NULL;
    
  }

  /* allocate completions structure */
  
  completions = (commc_trie_completions_t*)malloc(sizeof(commc_trie_completions_t));

  if  (completions == NULL) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
    
  }

  /* allocate array for result strings */
  
  strings = (char**)malloc(COMMC_TRIE_MAX_COMPLETIONS * sizeof(char*));

  if  (strings == NULL) {

    free(completions);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
    
  }

  /* navigate to the prefix location in the trie */
  
  current = trie->root;
  prefix_len = strlen(prefix);

  for  (i = 0; prefix[i] != '\0'; i++) {

    c = (unsigned char)prefix[i];

    if  (current->children[c] == NULL) {

      /* prefix doesn't exist - return empty results */
      
      completions->strings = strings;
      completions->count = 0;
      return completions;
      
    }

    current = current->children[c];
    
  }

  /* allocate temporary string buffer for building completions */
  
  temp_string = (char*)malloc(1000 * sizeof(char));  /* REASONABLE MAX STRING LENGTH */

  if  (temp_string == NULL) {

    free(strings);
    free(completions);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
    
  }

  /* copy prefix to temp string as starting point */
  
  strcpy(temp_string, prefix);
  count = 0;

  /* collect all completions starting from current prefix location */
  
  collect_completions(current, temp_string, (int)prefix_len, 
                     strings, &count, COMMC_TRIE_MAX_COMPLETIONS);

  free(temp_string);

  /* set up final results */
  
  completions->strings = strings;
  completions->count = count;

  return completions;
  
}

/*

         commc_trie_free_completions()
           ---
           frees memory allocated for completion results.
           
           releases all memory associated with a completions structure,
           including individual string allocations and the container arrays.
           essential for preventing memory leaks after completion operations.

*/

void  commc_trie_free_completions(commc_trie_completions_t* completions) {

  size_t  i;

  if  (completions == NULL) {

    return;
    
  }

  /* free each individual string */
  
  if  (completions->strings != NULL) {

    for  (i = 0; i < completions->count; i++) {

      if  (completions->strings[i] != NULL) {

        free(completions->strings[i]);
        
      }
      
    }

    free(completions->strings);
    
  }

  free(completions);
  
}

/* 
	==================================
             --- UTILITY OPERATIONS ---
	==================================
*/

/*

         commc_trie_size()
           ---
           returns the number of unique strings stored in the trie.
           
           provides O(1) access to the internally maintained count of
           distinct strings. this count is updated during insert and
           delete operations to remain accurate without traversal.

*/

size_t  commc_trie_size(const commc_trie_t* trie) {

  if  (trie == NULL) {

    return 0;
    
  }

  return trie->size;
  
}

/*

         commc_trie_is_empty()
           ---
           checks whether the trie contains any stored strings.
           
           convenience function equivalent to checking if size is zero,
           but provided for code clarity and consistent API patterns.

*/

bool  commc_trie_is_empty(const commc_trie_t* trie) {

  return commc_trie_size(trie) == 0;
  
}

/*

         commc_trie_clear()
           ---
           removes all strings from the trie while preserving structure.
           
           destroys all nodes except the root and creates a new empty root,
           effectively resetting the trie to initial state. more efficient
           than individual deletions for clearing large tries.

*/

commc_error_t  commc_trie_clear(commc_trie_t* trie) {

  if  (trie == NULL) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
                              
  }

  /* destroy existing tree structure */
  
  destroy_node(trie->root);

  /* create new empty root */
  
  trie->root = create_node();

  if  (trie->root == NULL) {

    return COMMC_MEMORY_ERROR;  /* ERROR ALREADY REPORTED BY create_node() */
    
  }

  trie->size = 0;

  return COMMC_SUCCESS;
  
}

/* 
	==================================
             --- EOF ---
	==================================
*/