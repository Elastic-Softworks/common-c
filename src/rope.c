/* 	
   ===================================
   C O M M C / R O P E . C
   ROPE DATA STRUCTURE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- ROPE IMPLEMENTATION ---

	    ropes use binary trees to represent strings, where each
	    leaf contains a string fragment and internal nodes define
	    the tree structure. this allows efficient concatenation
	    and splitting without copying entire strings.
	    
	    key insight: rope weight = length of left subtree.
	    this enables O(log n) indexing and substring operations.

*/

/* 
	==================================
             --- INCLUDES ---
	==================================
*/

#include "commc/rope.h"
#include <stdlib.h>         /* MALLOC, FREE */
#include <string.h>         /* STRLEN, STRCMP, STRNCMP, MEMCPY, STRSTR */

/* C89 compatibility - SIZE_MAX not defined in C89 */

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* 
	==================================
             --- HELPERS ---
	==================================
*/

/*

         rope_create_leaf_node()
	       ---
	       creates a leaf node containing string data.

*/

static commc_rope_node_t* rope_create_leaf_node(const char* str, size_t length) {

  commc_rope_node_t* node;
  
  if (!str || length == 0) {

    return NULL;
    
  }
  
  node = (commc_rope_node_t*)malloc(sizeof(commc_rope_node_t));
  
  if (!node) {

    return NULL;
    
  }
  
  node->data = (char*)malloc(length + 1);
  
  if (!node->data) {

    free(node);
    return NULL;
    
  }
  
  memcpy(node->data, str, length);
  node->data[length] = '\0';
  
  node->length = length;
  node->weight = length;
  node->left   = NULL;
  node->right  = NULL;
  
  return node;
  
}

/*

         rope_create_internal_node()
	       ---
	       creates internal node with left and right children.

*/

static commc_rope_node_t* rope_create_internal_node(commc_rope_node_t* left,
                                                     commc_rope_node_t* right) {

  commc_rope_node_t* node;
  
  if (!left && !right) {

    return NULL;
    
  }
  
  node = (commc_rope_node_t*)malloc(sizeof(commc_rope_node_t));
  
  if (!node) {

    return NULL;
    
  }
  
  node->data   = NULL;
  node->left   = left;
  node->right  = right;
  node->weight = left ? left->length : 0;
  node->length = node->weight + (right ? right->length : 0);
  
  return node;
  
}

/*

         rope_destroy_node()
	       ---
	       recursively destroys rope node and all children.

*/

static void rope_destroy_node(commc_rope_node_t* node) {

  if (!node) {

    return;
    
  }
  
  rope_destroy_node(node->left);
  rope_destroy_node(node->right);
  
  free(node->data);
  free(node);
  
}

/*

         rope_is_leaf()
	       ---
	       returns 1 if node is a leaf (contains data), 0 otherwise.

*/

static int rope_is_leaf(commc_rope_node_t* node) {

  return node && node->data != NULL;
  
}

/*

         rope_node_char_at()
	       ---
	       retrieves character at index within a rope node subtree.

*/

static char rope_node_char_at(commc_rope_node_t* node, size_t index) {

  if (!node) {

    return '\0';
    
  }
  
  if (rope_is_leaf(node)) {

    if (index >= node->length) {

      return '\0';
      
    }
    
    return node->data[index];
    
  }

  /* internal node - check weight to decide subtree */
  
  if (index < node->weight) {

    return rope_node_char_at(node->left, index);
    
  } else {

    return rope_node_char_at(node->right, index - node->weight);
    
  }
  
}

/*

         rope_node_to_string()
	       ---
	       converts rope node subtree to string.
	       writes into provided buffer starting at offset.

*/

static void rope_node_to_string(commc_rope_node_t* node, char* buffer, size_t* offset) {

  if (!node || !buffer) {

    return;
    
  }
  
  if (rope_is_leaf(node)) {

    memcpy(buffer + *offset, node->data, node->length);
    *offset += node->length;
    
  } else {

    if (node->left) {

      rope_node_to_string(node->left, buffer, offset);
      
    }
    
    if (node->right) {

      rope_node_to_string(node->right, buffer, offset);
      
    }
    
  }
  
}

/*

         rope_node_split()
	       ---
	       splits rope node at specified index.
	       returns left part, modifies node to be right part.
	       NOTE: Currently unused - kept for potential future optimization

*/

#if 0  /* DISABLED - UNUSED FUNCTION */

static commc_rope_node_t* rope_node_split(commc_rope_node_t** node, size_t index) {

  commc_rope_node_t* left_part;
  commc_rope_node_t* right_part;
  commc_rope_node_t* temp;
  
  if (!node || !*node) {

    return NULL;
    
  }
  
  if (rope_is_leaf(*node)) {

    if (index == 0) {

      return NULL; /* no left part */
      
    }
    
    if (index >= (*node)->length) {

      /* return entire node as left part */
      
      temp = *node;
      *node = NULL;
      return temp;
      
    }

    /* split leaf node */
    
    left_part = rope_create_leaf_node((*node)->data, index);
    
    if (!left_part) {

      return NULL;
      
    }

    /* modify original node to be right part */
    
    memmove((*node)->data, (*node)->data + index, (*node)->length - index);
    (*node)->length -= index;
    (*node)->weight = (*node)->length;
    (*node)->data[(*node)->length] = '\0';
    
    return left_part;
    
  }

  /* internal node */
  
  if (index <= (*node)->weight) {

    /* split within left subtree */
    
    left_part = rope_node_split(&((*node)->left), index);
    
    if (index == (*node)->weight) {

      /* clean split at boundary */
      
      temp = (*node)->left;
      (*node)->left = NULL;
      (*node)->weight = 0;
      (*node)->length = (*node)->right ? (*node)->right->length : 0;
      
      if (left_part && temp) {

        right_part = rope_create_internal_node(left_part, temp);
        return right_part;
        
      } else if (left_part) {

        return left_part;
        
      } else if (temp) {

        return temp;
        
      }
      
    } else if (left_part) {

      /* partial split in left subtree */
      
      (*node)->weight = (*node)->left ? (*node)->left->length : 0;
      (*node)->length = (*node)->weight + ((*node)->right ? (*node)->right->length : 0);
      
      return left_part;
      
    }
    
  } else {

    /* split within right subtree */
    
    left_part = rope_node_split(&((*node)->right), index - (*node)->weight);
    
    temp = (*node)->left;
    (*node)->left = NULL;
    (*node)->weight = 0;
    (*node)->length = (*node)->right ? (*node)->right->length : 0;
    
    if (temp && left_part) {

      right_part = rope_create_internal_node(temp, left_part);
      return right_part;
      
    } else if (temp) {

      return temp;
      
    } else if (left_part) {

      return left_part;
      
    }
    
  }
  
  return NULL;
  
}

#endif  /* DISABLED - UNUSED FUNCTION */

/*

         rope_node_depth()
	       ---
	       calculates maximum depth of rope node subtree.

*/

static size_t rope_node_depth(commc_rope_node_t* node) {

  size_t left_depth;
  size_t right_depth;
  
  if (!node) {

    return 0;
    
  }
  
  if (rope_is_leaf(node)) {

    return 1;
    
  }
  
  left_depth  = rope_node_depth(node->left);
  right_depth = rope_node_depth(node->right);
  
  return 1 + (left_depth > right_depth ? left_depth : right_depth);
  
}

/*

         rope_node_leaf_count()
	       ---
	       counts number of leaf nodes in subtree.

*/

static size_t rope_node_leaf_count(commc_rope_node_t* node) {

  if (!node) {

    return 0;
    
  }
  
  if (rope_is_leaf(node)) {

    return 1;
    
  }
  
  return rope_node_leaf_count(node->left) + rope_node_leaf_count(node->right);
  
}

/*

         rope_node_memory_usage()
	       ---
	       calculates memory usage of subtree.

*/

static size_t rope_node_memory_usage(commc_rope_node_t* node) {

  size_t usage;
  
  if (!node) {

    return 0;
    
  }
  
  usage = sizeof(commc_rope_node_t);
  
  if (rope_is_leaf(node)) {

    usage += node->length + 1; /* +1 for null terminator */
    
  }
  
  usage += rope_node_memory_usage(node->left);
  usage += rope_node_memory_usage(node->right);
  
  return usage;
  
}

/* 
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_rope_create()
	       ---
	       creates a new empty rope.

*/

commc_rope_t* commc_rope_create(void) {

  return commc_rope_create_with_threshold(COMMC_ROPE_DEFAULT_LEAF_THRESHOLD);
  
}

/*

         commc_rope_create_with_threshold()
	       ---
	       creates rope with custom leaf size threshold.

*/

commc_rope_t* commc_rope_create_with_threshold(size_t leaf_threshold) {

  commc_rope_t* rope;
  
  rope = (commc_rope_t*)malloc(sizeof(commc_rope_t));
  
  if (!rope) {

    return NULL;
    
  }
  
  rope->root           = NULL;
  rope->total_length   = 0;
  rope->leaf_threshold = leaf_threshold > 0 ? leaf_threshold : COMMC_ROPE_DEFAULT_LEAF_THRESHOLD;
  
  return rope;
  
}

/*

         commc_rope_create_from_string()
	       ---
	       creates rope from existing string.

*/

commc_rope_t* commc_rope_create_from_string(const char* str) {

  commc_rope_t* rope;
  size_t        len;
  
  if (!str) {

    return NULL;
    
  }
  
  len = strlen(str);
  
  if (len == 0) {

    return commc_rope_create();
    
  }
  
  rope = commc_rope_create();
  
  if (!rope) {

    return NULL;
    
  }
  
  rope->root = rope_create_leaf_node(str, len);
  
  if (!rope->root) {

    free(rope);
    return NULL;
    
  }
  
  rope->total_length = len;
  
  return rope;
  
}

/*

         commc_rope_destroy()
	       ---
	       destroys rope and frees all associated memory.

*/

void commc_rope_destroy(commc_rope_t* rope) {

  if (!rope) {

    return;
    
  }
  
  rope_destroy_node(rope->root);
  free(rope);
  
}

/*

         commc_rope_length()
	       ---
	       returns the total length of the rope string.

*/

size_t commc_rope_length(commc_rope_t* rope) {

  return rope ? rope->total_length : 0;
  
}

/*

         commc_rope_is_empty()
	       ---
	       returns 1 if rope is empty, 0 otherwise.

*/

int commc_rope_is_empty(commc_rope_t* rope) {

  return rope ? (rope->total_length == 0) : 1;
  
}

/* 
	==================================
             --- ACCESS API ---
	==================================
*/

/*

         commc_rope_char_at()
	       ---
	       retrieves character at specified position.

*/

char commc_rope_char_at(commc_rope_t* rope, size_t index) {

  if (!rope || index >= rope->total_length) {

    return '\0';
    
  }
  
  return rope_node_char_at(rope->root, index);
  
}

/*

         commc_rope_substring()
	       ---
	       extracts substring from rope as new string.

*/

char* commc_rope_substring(commc_rope_t* rope, size_t start, size_t length) {

  char*  result;
  size_t i;
  size_t end;
  
  if (!rope || start >= rope->total_length || length == 0) {

    return NULL;
    
  }
  
  end = start + length;
  
  if (end > rope->total_length) {

    end = rope->total_length;
    length = end - start;
    
  }
  
  result = (char*)malloc(length + 1);
  
  if (!result) {

    return NULL;
    
  }
  
  for (i = 0; i < length; i++) {

    result[i] = rope_node_char_at(rope->root, start + i);
    
  }
  
  result[length] = '\0';
  
  return result;
  
}

/*

         commc_rope_to_string()
	       ---
	       converts entire rope to null-terminated string.

*/

char* commc_rope_to_string(commc_rope_t* rope) {

  char*  result;
  size_t offset;
  
  if (!rope || rope->total_length == 0) {

    result = (char*)malloc(1);
    
    if (result) {

      result[0] = '\0';
      
    }
    
    return result;
    
  }
  
  result = (char*)malloc(rope->total_length + 1);
  
  if (!result) {

    return NULL;
    
  }
  
  offset = 0;
  rope_node_to_string(rope->root, result, &offset);
  result[rope->total_length] = '\0';
  
  return result;
  
}

/* 
	==================================
             --- MODIFICATION API ---
	==================================
*/

/*

         commc_rope_concat()
	       ---
	       concatenates two ropes, creating a new rope.

*/

commc_rope_t* commc_rope_concat(commc_rope_t* left, commc_rope_t* right) {

  commc_rope_t* result;
  
  if (!left && !right) {

    return NULL;
    
  }
  
  if (!left) {

    /* copy right rope */
    
    if (right->total_length == 0) {

      return commc_rope_create();
      
    }
    
    /* for simplicity, convert to string and back */
    
    {
      char* str;
      
      str = commc_rope_to_string(right);
      
      if (!str) {

        return NULL;
        
      }
      
      result = commc_rope_create_from_string(str);
      free(str);
      
      return result;
    }
    
  }
  
  if (!right) {

    /* copy left rope */
    
    if (left->total_length == 0) {

      return commc_rope_create();
      
    }
    
    {
      char* str;
      
      str = commc_rope_to_string(left);
      
      if (!str) {

        return NULL;
        
      }
      
      result = commc_rope_create_from_string(str);
      free(str);
      
      return result;
    }
    
  }
  
  result = commc_rope_create();
  
  if (!result) {

    return NULL;
    
  }
  
  if (left->total_length == 0 && right->total_length == 0) {

    return result;
    
  }

  /* handle cases where one rope is empty */
  
  if (left->total_length == 0) {

    /* left is empty, copy right */
    
    char* str = commc_rope_to_string(right);
    
    if (!str) {

      commc_rope_destroy(result);
      return NULL;
      
    }
    
    commc_rope_destroy(result);
    result = commc_rope_create_from_string(str);
    free(str);
    
    return result;
    
  }
  
  if (right->total_length == 0) {

    /* right is empty, copy left */
    
    char* str = commc_rope_to_string(left);
    
    if (!str) {

      commc_rope_destroy(result);
      return NULL;
      
    }
    
    commc_rope_destroy(result);
    result = commc_rope_create_from_string(str);
    free(str);
    
    return result;
    
  }

  /* create internal node joining left and right roots */
  
  /* for simplicity, create copies of the root nodes */
  
  {
    char* left_str;
    char* right_str;
    commc_rope_node_t* left_node;
    commc_rope_node_t* right_node;
    
    left_str  = commc_rope_to_string(left);
    right_str = commc_rope_to_string(right);
    
    if (!left_str || !right_str) {

      free(left_str);
      free(right_str);
      commc_rope_destroy(result);
      return NULL;
      
    }
    
    left_node  = rope_create_leaf_node(left_str, left->total_length);
    right_node = rope_create_leaf_node(right_str, right->total_length);
    
    free(left_str);
    free(right_str);
    
    if (!left_node || !right_node) {

      rope_destroy_node(left_node);
      rope_destroy_node(right_node);
      commc_rope_destroy(result);
      return NULL;
      
    }
    
    result->root = rope_create_internal_node(left_node, right_node);
    
    if (!result->root) {

      rope_destroy_node(left_node);
      rope_destroy_node(right_node);
      commc_rope_destroy(result);
      return NULL;
      
    }
  }
  
  result->total_length = left->total_length + right->total_length;
  
  return result;
  
}

/*

         commc_rope_insert()
	       ---
	       inserts string at specified position.

*/

commc_error_t commc_rope_insert(commc_rope_t* rope, size_t index, const char* str) {

  commc_rope_t*  left_part;
  commc_rope_t*  right_part;
  commc_rope_t*  insert_rope;
  commc_rope_t*  temp1;
  commc_rope_t*  temp2;
  commc_error_t  result;
  size_t         str_len;
  
  if (!rope || !str) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  str_len = strlen(str);
  
  if (str_len == 0) {

    return COMMC_SUCCESS; /* nothing to insert */
    
  }
  
  if (index > rope->total_length) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  if (rope->total_length == 0) {

    /* inserting into empty rope */
    
    rope->root = rope_create_leaf_node(str, str_len);
    
    if (!rope->root) {

      return COMMC_MEMORY_ERROR;
      
    }
    
    rope->total_length = str_len;
    return COMMC_SUCCESS;
    
  }

  /* split rope at insertion point */
  
  result = commc_rope_split(rope, index, &left_part, &right_part);
  
  if (result != COMMC_SUCCESS) {

    return result;
    
  }

  /* create rope for inserted string */
  
  insert_rope = commc_rope_create_from_string(str);
  
  if (!insert_rope) {

    commc_rope_destroy(left_part);
    commc_rope_destroy(right_part);
    return COMMC_MEMORY_ERROR;
    
  }

  /* concatenate: left + insert + right */
  
  temp1 = commc_rope_concat(left_part, insert_rope);
  temp2 = commc_rope_concat(temp1, right_part);
  
  commc_rope_destroy(left_part);
  commc_rope_destroy(right_part);
  commc_rope_destroy(insert_rope);
  commc_rope_destroy(temp1);
  
  if (!temp2) {

    return COMMC_MEMORY_ERROR;
    
  }

  /* replace rope contents */
  
  rope_destroy_node(rope->root);
  rope->root = temp2->root;
  rope->total_length = temp2->total_length;
  temp2->root = NULL; /* prevent double-free */
  commc_rope_destroy(temp2);
  
  return COMMC_SUCCESS;
  
}

/*

         commc_rope_delete()
	       ---
	       removes characters from specified range.

*/

commc_error_t commc_rope_delete(commc_rope_t* rope, size_t start, size_t length) {

  commc_rope_t*  left_part;
  commc_rope_t*  middle_part;
  commc_rope_t*  right_part;
  commc_rope_t*  result_rope;
  commc_error_t  result;
  size_t         end;
  
  if (!rope) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  if (length == 0 || start >= rope->total_length) {

    return COMMC_SUCCESS; /* nothing to delete */
    
  }
  
  end = start + length;
  
  if (end > rope->total_length) {

    end = rope->total_length;
    
  }

  /* split into three parts: before, deleted, after */
  
  result = commc_rope_split(rope, start, &left_part, &middle_part);
  
  if (result != COMMC_SUCCESS) {

    return result;
    
  }
  
  result = commc_rope_split(middle_part, end - start, &middle_part, &right_part);
  
  if (result != COMMC_SUCCESS) {

    commc_rope_destroy(left_part);
    commc_rope_destroy(middle_part);
    return result;
    
  }

  /* concatenate left + right (skip middle) */
  
  result_rope = commc_rope_concat(left_part, right_part);
  
  commc_rope_destroy(left_part);
  commc_rope_destroy(middle_part); /* this is what we're deleting */
  commc_rope_destroy(right_part);
  
  if (!result_rope) {

    return COMMC_MEMORY_ERROR;
    
  }

  /* replace rope contents */
  
  rope_destroy_node(rope->root);
  rope->root = result_rope->root;
  rope->total_length = result_rope->total_length;
  result_rope->root = NULL; /* prevent double-free */
  commc_rope_destroy(result_rope);
  
  return COMMC_SUCCESS;
  
}

/*

         commc_rope_split()
	       ---
	       splits rope at specified position into two ropes.

*/

commc_error_t commc_rope_split(commc_rope_t* rope, size_t index,
                               commc_rope_t** left, commc_rope_t** right) {

  if (!rope || !left || !right) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  if (index > rope->total_length) {

    return COMMC_ARGUMENT_ERROR;
    
  }

  /* create result ropes */
  
  *left = commc_rope_create();
  *right = commc_rope_create();
  
  if (!*left || !*right) {

    commc_rope_destroy(*left);
    commc_rope_destroy(*right);
    *left = NULL;
    *right = NULL;
    return COMMC_MEMORY_ERROR;
    
  }
  
  if (index == 0) {

    /* entire rope goes to right */
    
    (*left)->total_length = 0;
    
    if (rope->total_length > 0) {

      char* str;
      
      str = commc_rope_to_string(rope);
      
      if (!str) {

        commc_rope_destroy(*left);
        commc_rope_destroy(*right);
        *left = NULL;
        *right = NULL;
        return COMMC_MEMORY_ERROR;
        
      }
      
      (*right)->root = rope_create_leaf_node(str, rope->total_length);
      free(str);
      
      if (!(*right)->root) {

        commc_rope_destroy(*left);
        commc_rope_destroy(*right);
        *left = NULL;
        *right = NULL;
        return COMMC_MEMORY_ERROR;
        
      }
      
    }
    
    (*right)->total_length = rope->total_length;
    
    return COMMC_SUCCESS;
    
  }
  
  if (index >= rope->total_length) {

    /* entire rope goes to left */
    
    (*right)->total_length = 0;
    
    if (rope->total_length > 0) {

      char* str;
      
      str = commc_rope_to_string(rope);
      
      if (!str) {

        commc_rope_destroy(*left);
        commc_rope_destroy(*right);
        *left = NULL;
        *right = NULL;
        return COMMC_MEMORY_ERROR;
        
      }
      
      (*left)->root = rope_create_leaf_node(str, rope->total_length);
      free(str);
      
      if (!(*left)->root) {

        commc_rope_destroy(*left);
        commc_rope_destroy(*right);
        *left = NULL;
        *right = NULL;
        return COMMC_MEMORY_ERROR;
        
      }
      
    }
    
    (*left)->total_length = rope->total_length;
    
    return COMMC_SUCCESS;
    
  }

  /* use simple approach: convert to strings and recreate */
  
  {
    char* left_str;
    char* right_str;
    
    left_str = commc_rope_substring(rope, 0, index);
    right_str = commc_rope_substring(rope, index, rope->total_length - index);
    
    if (!left_str || !right_str) {

      free(left_str);
      free(right_str);
      commc_rope_destroy(*left);
      commc_rope_destroy(*right);
      *left = NULL;
      *right = NULL;
      return COMMC_MEMORY_ERROR;
      
    }
    
    if (index > 0) {

      (*left)->root = rope_create_leaf_node(left_str, index);
      
      if (!(*left)->root) {

        free(left_str);
        free(right_str);
        commc_rope_destroy(*left);
        commc_rope_destroy(*right);
        *left = NULL;
        *right = NULL;
        return COMMC_MEMORY_ERROR;
        
      }
      
    }
    
    if (rope->total_length - index > 0) {

      (*right)->root = rope_create_leaf_node(right_str, rope->total_length - index);
      
      if (!(*right)->root) {

        free(left_str);
        free(right_str);
        commc_rope_destroy(*left);
        commc_rope_destroy(*right);
        *left = NULL;
        *right = NULL;
        return COMMC_MEMORY_ERROR;
        
      }
      
    }
    
    (*left)->total_length = index;
    (*right)->total_length = rope->total_length - index;
    
    free(left_str);
    free(right_str);
  }
  
  return COMMC_SUCCESS;
  
}

/* 
	==================================
             --- SEARCH API ---
	==================================
*/

/*

         commc_rope_find()
	       ---
	       finds first occurrence of substring in rope.

*/

size_t commc_rope_find(commc_rope_t* rope, const char* needle, size_t start) {

  char*  haystack;
  char*  found;
  size_t result;
  
  if (!rope || !needle || start >= rope->total_length) {

    return SIZE_MAX;
    
  }
  
  haystack = commc_rope_to_string(rope);
  
  if (!haystack) {

    return SIZE_MAX;
    
  }
  
  found = strstr(haystack + start, needle);
  
  if (found) {

    result = found - haystack;
    
  } else {

    result = SIZE_MAX;
    
  }
  
  free(haystack);
  
  return result;
  
}

/*

         commc_rope_find_char()
	       ---
	       finds first occurrence of character in rope.

*/

size_t commc_rope_find_char(commc_rope_t* rope, char ch, size_t start) {

  size_t i;
  
  if (!rope || start >= rope->total_length) {

    return SIZE_MAX;
    
  }
  
  for (i = start; i < rope->total_length; i++) {

    if (commc_rope_char_at(rope, i) == ch) {

      return i;
      
    }
    
  }
  
  return SIZE_MAX;
  
}

/*

         commc_rope_count_lines()
	       ---
	       counts number of newline characters in rope.

*/

size_t commc_rope_count_lines(commc_rope_t* rope) {

  size_t count;
  size_t i;
  
  if (!rope) {

    return 0;
    
  }
  
  count = 0;
  
  for (i = 0; i < rope->total_length; i++) {

    if (commc_rope_char_at(rope, i) == '\n') {

      count++;
      
    }
    
  }
  
  return count;
  
}

/* 
	==================================
             --- ITERATION API ---
	==================================
*/

/*

         commc_rope_iterator_begin()
	       ---
	       creates iterator starting from beginning of rope.

*/

commc_rope_iterator_t commc_rope_iterator_begin(commc_rope_t* rope) {

  commc_rope_iterator_t iterator;
  
  iterator.rope         = rope;
  iterator.current_node = NULL;
  iterator.position     = 0;
  iterator.global_pos   = 0;
  
  if (rope && rope->root && rope->total_length > 0) {

    /* find first leaf node */
    
    commc_rope_node_t* current = rope->root;
    
    while (current && !rope_is_leaf(current)) {

      current = current->left ? current->left : current->right;
      
    }
    
    iterator.current_node = current;
    
  }
  
  return iterator;
  
}

/*

         commc_rope_iterator_at()
	       ---
	       creates iterator starting at specified position.

*/

commc_rope_iterator_t commc_rope_iterator_at(commc_rope_t* rope, size_t position) {

  commc_rope_iterator_t iterator = commc_rope_iterator_begin(rope);
  size_t                i;
  
  /* advance to desired position */
  
  for (i = 0; i < position && commc_rope_iterator_has_next(&iterator); i++) {

    commc_rope_iterator_next(&iterator);
    
  }
  
  return iterator;
  
}

/*

         commc_rope_iterator_next()
	       ---
	       advances iterator to next character.

*/

commc_error_t commc_rope_iterator_next(commc_rope_iterator_t* iterator) {

  if (!iterator || !iterator->current_node) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  iterator->position++;
  iterator->global_pos++;
  
  if (iterator->position >= iterator->current_node->length) {

    /* need to find next leaf node - simplified approach */
    
    iterator->current_node = NULL;
    iterator->position = 0;
    
  }
  
  return COMMC_SUCCESS;
  
}

/*

         commc_rope_iterator_get_char()
	       ---
	       retrieves current character from iterator.

*/

char commc_rope_iterator_get_char(commc_rope_iterator_t* iterator) {

  if (!iterator || !iterator->current_node || 
      iterator->position >= iterator->current_node->length) {

    return '\0';
    
  }
  
  return iterator->current_node->data[iterator->position];
  
}

/*

         commc_rope_iterator_get_position()
	       ---
	       retrieves current global position of iterator.

*/

size_t commc_rope_iterator_get_position(commc_rope_iterator_t* iterator) {

  return iterator ? iterator->global_pos : 0;
  
}

/*

         commc_rope_iterator_has_next()
	       ---
	       returns 1 if iterator has more characters.

*/

int commc_rope_iterator_has_next(commc_rope_iterator_t* iterator) {

  return (iterator && iterator->rope && 
          iterator->global_pos < iterator->rope->total_length) ? 1 : 0;
  
}

/* 
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_rope_balance()
	       ---
	       rebalances rope tree for optimal performance.

*/

commc_error_t commc_rope_balance(commc_rope_t* rope) {

  char* str;
  
  if (!rope) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  if (rope->total_length == 0) {

    return COMMC_SUCCESS;
    
  }

  /* simple rebalancing: convert to string and recreate */
  
  str = commc_rope_to_string(rope);
  
  if (!str) {

    return COMMC_MEMORY_ERROR;
    
  }
  
  rope_destroy_node(rope->root);
  rope->root = rope_create_leaf_node(str, rope->total_length);
  
  free(str);
  
  if (!rope->root) {

    rope->total_length = 0;
    return COMMC_MEMORY_ERROR;
    
  }
  
  return COMMC_SUCCESS;
  
}

/*

         commc_rope_is_balanced()
	       ---
	       checks if rope tree is reasonably balanced.

*/

int commc_rope_is_balanced(commc_rope_t* rope) {

  size_t depth;
  size_t max_depth;
  
  if (!rope || rope->total_length == 0) {

    return 1;
    
  }
  
  depth = rope_node_depth(rope->root);
  max_depth = COMMC_ROPE_BALANCE_THRESHOLD;
  
  return depth <= max_depth;
  
}

/*

         commc_rope_depth()
	       ---
	       calculates maximum depth of rope tree.

*/

size_t commc_rope_depth(commc_rope_t* rope) {

  return rope ? rope_node_depth(rope->root) : 0;
  
}

/*

         commc_rope_leaf_count()
	       ---
	       counts number of leaf nodes in rope tree.

*/

size_t commc_rope_leaf_count(commc_rope_t* rope) {

  return rope ? rope_node_leaf_count(rope->root) : 0;
  
}

/*

         commc_rope_memory_usage()
	       ---
	       estimates total memory usage in bytes.

*/

size_t commc_rope_memory_usage(commc_rope_t* rope) {

  if (!rope) {

    return 0;
    
  }
  
  return sizeof(commc_rope_t) + rope_node_memory_usage(rope->root);
  
}

/* 
	==================================
             --- COMPARISON API ---
	==================================
*/

/*

         commc_rope_compare()
	       ---
	       compares two ropes lexicographically.

*/

int commc_rope_compare(commc_rope_t* rope1, commc_rope_t* rope2) {

  char* str1;
  char* str2;
  int   result;
  
  if (!rope1 && !rope2) {

    return 0;
    
  }
  
  if (!rope1) {

    return -1;
    
  }
  
  if (!rope2) {

    return 1;
    
  }
  
  str1 = commc_rope_to_string(rope1);
  str2 = commc_rope_to_string(rope2);
  
  if (!str1 || !str2) {

    free(str1);
    free(str2);
    return 0;
    
  }
  
  result = strcmp(str1, str2);
  
  free(str1);
  free(str2);
  
  return result;
  
}

/*

         commc_rope_equals()
	       ---
	       tests if two ropes contain identical strings.

*/

int commc_rope_equals(commc_rope_t* rope1, commc_rope_t* rope2) {

  return commc_rope_compare(rope1, rope2) == 0;
  
}

/*

         commc_rope_starts_with()
	       ---
	       tests if rope starts with given string.

*/

int commc_rope_starts_with(commc_rope_t* rope, const char* prefix) {

  char* str;
  int   result;
  
  if (!rope || !prefix) {

    return 0;
    
  }
  
  if (strlen(prefix) > rope->total_length) {

    return 0;
    
  }
  
  str = commc_rope_substring(rope, 0, strlen(prefix));
  
  if (!str) {

    return 0;
    
  }
  
  result = (strcmp(str, prefix) == 0);
  
  free(str);
  
  return result;
  
}

/*

         commc_rope_ends_with()
	       ---
	       tests if rope ends with given string.

*/

int commc_rope_ends_with(commc_rope_t* rope, const char* suffix) {

  char*  str;
  int    result;
  size_t suffix_len;
  size_t start;
  
  if (!rope || !suffix) {

    return 0;
    
  }
  
  suffix_len = strlen(suffix);
  
  if (suffix_len > rope->total_length) {

    return 0;
    
  }
  
  start = rope->total_length - suffix_len;
  str = commc_rope_substring(rope, start, suffix_len);
  
  if (!str) {

    return 0;
    
  }
  
  result = (strcmp(str, suffix) == 0);
  
  free(str);
  
  return result;
  
}

/*
	==================================
             --- EOF ---
	==================================
*/