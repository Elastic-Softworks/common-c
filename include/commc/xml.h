/* 	
   ===================================
   X M L  P A R S E R  A N D  P R O C E S S O R
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- XML ETHOS ---

	    XML (Extensible Markup Language) is a markup language
	    that defines rules for encoding documents in a format
	    that is both human-readable and machine-readable.
	    
	    this implementation provides both DOM (Document Object Model)
	    and SAX (Simple API for XML) parsing modes, namespace
	    support, validation capabilities, and detailed error
	    reporting for malformed documents.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_XML_H
#define COMMC_XML_H

#include  <stddef.h>   /* for size_t */
#include  <stdint.h>   /* for uint8_t */

#include  "commc/error.h"

/*
	==================================
             --- CONSTANTS ---
	==================================
*/

/* XML parsing limits */

#define COMMC_XML_MAX_DEPTH                128     /* maximum nesting depth */
#define COMMC_XML_MAX_TAG_NAME_LENGTH      256     /* maximum tag name length */
#define COMMC_XML_MAX_ATTRIBUTE_NAME_LENGTH 128    /* maximum attribute name length */
#define COMMC_XML_MAX_TEXT_LENGTH          65536   /* maximum text content length */
#define COMMC_XML_MAX_ATTRIBUTES           64      /* maximum attributes per element */

/* XML node types */

#define COMMC_XML_NODE_ELEMENT             1       /* element node */
#define COMMC_XML_NODE_TEXT                2       /* text content node */
#define COMMC_XML_NODE_COMMENT             3       /* comment node */
#define COMMC_XML_NODE_CDATA               4       /* CDATA section */
#define COMMC_XML_NODE_PROCESSING_INSTRUCTION 5    /* processing instruction */
#define COMMC_XML_NODE_DOCUMENT            6       /* document root */

/* XML parsing modes */

#define COMMC_XML_PARSE_DOM                0       /* build complete DOM tree */
#define COMMC_XML_PARSE_SAX                1       /* event-driven parsing */
#define COMMC_XML_PARSE_STREAMING          2       /* streaming parser */

/* XML validation modes */

#define COMMC_XML_VALIDATE_NONE            0       /* no validation */
#define COMMC_XML_VALIDATE_WELL_FORMED     1       /* well-formedness only */
#define COMMC_XML_VALIDATE_DTD             2       /* DTD validation */

/* XML encoding types */

#define COMMC_XML_ENCODING_UTF8            0       /* UTF-8 encoding */
#define COMMC_XML_ENCODING_UTF16           1       /* UTF-16 encoding */
#define COMMC_XML_ENCODING_ASCII           2       /* ASCII encoding */

/*
	==================================
             --- DATA TYPES ---
	==================================
*/

/*

         commc_xml_attribute_t
	       ---
	       represents an XML element attribute with
	       name, value, and namespace information.

*/

typedef struct {

  char* name;                       /* attribute name */
  char* value;                      /* attribute value */
  char* namespace_uri;              /* namespace URI (optional) */
  char* prefix;                     /* namespace prefix (optional) */

} commc_xml_attribute_t;

/*

         commc_xml_namespace_t
	       ---
	       represents a namespace declaration with
	       prefix and URI mapping.

*/

typedef struct {

  char* prefix;                     /* namespace prefix */
  char* uri;                        /* namespace URI */

} commc_xml_namespace_t;

/*

         commc_xml_node_t
	       ---
	       represents a node in the XML document tree
	       with type-specific data and hierarchical structure.

*/

typedef struct commc_xml_node {

  int type;                         /* node type */
  char* name;                       /* element name or processing instruction target */
  char* content;                    /* text content */
  
  /* element-specific data */
  
  commc_xml_attribute_t* attributes;    /* element attributes */
  size_t attribute_count;               /* number of attributes */
  size_t attribute_capacity;            /* allocated attribute capacity */
  
  /* namespace information */
  
  char* namespace_uri;                  /* element namespace URI */
  char* prefix;                         /* element namespace prefix */
  commc_xml_namespace_t* namespaces;    /* namespace declarations */
  size_t namespace_count;               /* number of namespace declarations */
  
  /* tree structure */
  
  struct commc_xml_node* parent;        /* parent node */
  struct commc_xml_node* first_child;   /* first child node */
  struct commc_xml_node* last_child;    /* last child node */
  struct commc_xml_node* next_sibling;  /* next sibling node */
  struct commc_xml_node* prev_sibling;  /* previous sibling node */

} commc_xml_node_t;

/*

         commc_xml_document_t
	       ---
	       represents a complete XML document with
	       root element and document-level metadata.

*/

typedef struct {

  commc_xml_node_t* root;           /* document root element */
  
  char* version;                    /* XML version */
  char* encoding;                   /* document encoding */
  int standalone;                   /* standalone declaration */
  
  char* doctype_name;               /* DOCTYPE name */
  char* doctype_public_id;          /* DOCTYPE public ID */
  char* doctype_system_id;          /* DOCTYPE system ID */

} commc_xml_document_t;

/*

         commc_xml_parser_config_t
	       ---
	       configuration options for XML parsing
	       behavior and validation settings.

*/

typedef struct {

  int parse_mode;                   /* parsing mode (DOM/SAX/streaming) */
  int validation_mode;              /* validation level */
  int encoding;                     /* expected encoding */
  
  int preserve_whitespace;          /* preserve whitespace nodes */
  int resolve_entities;             /* resolve entity references */
  int validate_namespaces;          /* validate namespace declarations */
  
  size_t max_depth;                 /* maximum nesting depth */
  size_t buffer_size;               /* parsing buffer size */

} commc_xml_parser_config_t;

/*

         commc_xml_error_t
	       ---
	       detailed error information for XML parsing
	       failures with location and context.

*/

typedef struct {

  int code;                         /* error code */
  char* message;                    /* error description */
  
  size_t line;                      /* error line number */
  size_t column;                    /* error column number */
  size_t position;                  /* byte position in input */
  
  char* context;                    /* surrounding text context */
  char* element_path;               /* path to current element */

} commc_xml_error_t;

/*

         commc_xml_parser_t
	       ---
	       XML parser context maintaining state for
	       document processing and error tracking.

*/

typedef struct {

  const char* input;                /* input XML text */
  size_t input_size;                /* size of input */
  size_t position;                  /* current parse position */
  
  size_t line;                      /* current line number */
  size_t column;                    /* current column number */
  
  commc_xml_parser_config_t config; /* parser configuration */
  commc_xml_error_t error;          /* last error information */
  
  /* parsing state */
  
  commc_xml_node_t* current_node;   /* current node being parsed */
  int current_depth;                /* current nesting depth */
  
  /* namespace context stack */
  
  commc_xml_namespace_t** namespace_stack;  /* namespace context stack */
  size_t namespace_stack_depth;             /* current stack depth */
  size_t namespace_stack_capacity;          /* allocated stack capacity */

} commc_xml_parser_t;

/*

         commc_xml_sax_handler_t
	       ---
	       callback functions for SAX-style event-driven
	       XML parsing with element and content events.

*/

typedef struct {

  void* user_data;                  /* user-provided context data */
  
  /* element events */
  
  void (*start_element)(void* user_data, 
                        const char* name,
                        const commc_xml_attribute_t* attributes,
                        size_t attribute_count);
                        
  void (*end_element)(void* user_data, 
                      const char* name);
                      
  void (*characters)(void* user_data,
                     const char* text,
                     size_t length);
                     
  void (*comment)(void* user_data,
                  const char* text);
                  
  void (*processing_instruction)(void* user_data,
                                 const char* target,
                                 const char* data);
                                 
  /* error handling */
  
  void (*error)(void* user_data,
                const commc_xml_error_t* error);

} commc_xml_sax_handler_t;

/*
	==================================
             --- PROTOTYPES ---
	==================================
*/

/* document functions */

commc_xml_document_t* commc_xml_document_create(void);

void commc_xml_document_destroy(commc_xml_document_t* document);

commc_xml_document_t* commc_xml_document_clone(const commc_xml_document_t* source);

/* node functions */

commc_xml_node_t* commc_xml_node_create(int type);

void commc_xml_node_destroy(commc_xml_node_t* node);

commc_xml_node_t* commc_xml_node_clone(const commc_xml_node_t* source);

/* node manipulation */

int commc_xml_node_add_child(commc_xml_node_t* parent, commc_xml_node_t* child);

int commc_xml_node_remove_child(commc_xml_node_t* parent, commc_xml_node_t* child);

int commc_xml_node_insert_before(commc_xml_node_t* parent, 
                                 commc_xml_node_t* new_child,
                                 commc_xml_node_t* reference_child);

/* attribute functions */

int commc_xml_node_set_attribute(commc_xml_node_t* node,
                                 const char* name,
                                 const char* value);

const char* commc_xml_node_get_attribute(const commc_xml_node_t* node,
                                         const char* name);

int commc_xml_node_remove_attribute(commc_xml_node_t* node,
                                    const char* name);

/* namespace functions */

int commc_xml_node_set_namespace(commc_xml_node_t* node,
                                 const char* uri,
                                 const char* prefix);

const char* commc_xml_node_get_namespace_uri(const commc_xml_node_t* node);

/* parser configuration */

commc_xml_parser_t* commc_xml_parser_create(void);

void commc_xml_parser_destroy(commc_xml_parser_t* parser);

void commc_xml_parser_set_config(commc_xml_parser_t* parser,
                                 const commc_xml_parser_config_t* config);

void commc_xml_parser_config_init_default(commc_xml_parser_config_t* config);

/* DOM parsing */

commc_xml_document_t* commc_xml_parse_document(const char* xml_text);

commc_xml_document_t* commc_xml_parse_document_with_parser(commc_xml_parser_t* parser,
                                                           const char* xml_text,
                                                           size_t xml_size);

/* SAX parsing */

int commc_xml_parse_sax(commc_xml_parser_t* parser,
                        const char* xml_text,
                        size_t xml_size,
                        const commc_xml_sax_handler_t* handler);

/* streaming parsing */

int commc_xml_parse_chunk(commc_xml_parser_t* parser,
                          const char* chunk,
                          size_t chunk_size);

int commc_xml_parse_finish(commc_xml_parser_t* parser);

/* validation */

int commc_xml_validate_document(const commc_xml_document_t* document);

int commc_xml_validate_well_formed(const char* xml_text);

/* serialization */

char* commc_xml_serialize_document(const commc_xml_document_t* document);

int commc_xml_serialize_to_buffer(const commc_xml_document_t* document,
                                  char* buffer,
                                  size_t buffer_size,
                                  size_t* output_size);

/* query functions */

commc_xml_node_t* commc_xml_find_element(const commc_xml_node_t* root,
                                          const char* name);

commc_xml_node_t** commc_xml_find_elements(const commc_xml_node_t* root,
                                            const char* name,
                                            size_t* count);

commc_xml_node_t* commc_xml_xpath_query(const commc_xml_node_t* root,
                                         const char* xpath);

/* utility functions */

char* commc_xml_escape_text(const char* text);

char* commc_xml_unescape_text(const char* escaped_text);

int commc_xml_is_valid_name(const char* name);

/* error handling */

const commc_xml_error_t* commc_xml_parser_get_error(const commc_xml_parser_t* parser);

void commc_xml_error_destroy(commc_xml_error_t* error);

/* type checking functions */

int commc_xml_is_element(const commc_xml_node_t* node);

int commc_xml_is_text(const commc_xml_node_t* node);

int commc_xml_is_comment(const commc_xml_node_t* node);

/* traversal functions */

commc_xml_node_t* commc_xml_node_next_element(const commc_xml_node_t* node);

commc_xml_node_t* commc_xml_node_prev_element(const commc_xml_node_t* node);

commc_xml_node_t* commc_xml_node_first_child_element(const commc_xml_node_t* node);

commc_xml_node_t* commc_xml_node_last_child_element(const commc_xml_node_t* node);

/* 
	==================================
             --- EOF ---
	==================================
*/

#endif /* COMMC_XML_H */