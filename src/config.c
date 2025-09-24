/* 	
   ===================================
   C O N F I G . C
   CONFIGURATION FILE PARSER
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    this module implements configuration file parsing
	    with primary support for INI format, and basic
	    support for TOML and YAML-style formats.
	       
	    provides a robust foundation for application
	    configuration management while maintaining
	    C89 compliance and educational documentation.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "commc/config.h"
#include "commc/error.h"

/* 
	==================================
           --- CONSTANTS ---
	==================================
*/

#define INITIAL_ENTRY_CAPACITY     32    /* INITIAL ENTRY ALLOCATION */
#define ENTRY_GROWTH_FACTOR        2     /* GROWTH FACTOR FOR REALLOC */

/* 
	==================================
             --- HELPERS ---
	==================================
*/

/*

         trim_whitespace()
	       ---
	       removes leading and trailing whitespace from
	       a string, modifying the input buffer in place
	       and returning the trimmed length.

*/

static char* trim_whitespace(char* str) {

    char* end;
    
    if (!str) {
    
        return NULL;
        
    }
    
    /* Trim leading whitespace */
    
    while (isspace((unsigned char)*str)) {
    
        str++;
        
    }
    
    /* Handle empty string */
    
    if (*str == 0) {
    
        return str;
        
    }
    
    /* Trim trailing whitespace */
    
    end = str + strlen(str) - 1;
    
    while (end > str && isspace((unsigned char)*end)) {
    
        end--;
        
    }
    
    *(end + 1) = '\0';
    
    return str;
    
}

/*

         parse_boolean_value()
	       ---
	       converts various string representations to
	       boolean values, supporting common formats
	       like true/false, yes/no, 1/0.

*/

static int parse_boolean_value(const char* str) {

    char* lower_str;
    size_t len;
    size_t i;
    int result = 0;
    
    if (!str) {
    
        return 0;
        
    }
    
    len = strlen(str);
    lower_str = malloc(len + 1);
    
    if (!lower_str) {
    
        return 0;
        
    }
    
    /* Convert to lowercase */
    
    for (i = 0; i < len; i++) {
    
        lower_str[i] = (char)tolower((unsigned char)str[i]);
        
    }
    
    lower_str[len] = '\0';
    
    /* Check various boolean representations */
    
    if (strcmp(lower_str, "true") == 0 ||
        strcmp(lower_str, "yes") == 0 ||
        strcmp(lower_str, "on") == 0 ||
        strcmp(lower_str, "1") == 0) {
        
        result = 1;
        
    } else if (strcmp(lower_str, "false") == 0 ||
               strcmp(lower_str, "no") == 0 ||
               strcmp(lower_str, "off") == 0 ||
               strcmp(lower_str, "0") == 0) {
        
        result = 0;
        
    } else {
    
        /* Default to false for invalid values */
        
        result = 0;
        
    }
    
    free(lower_str);
    return result;
    
}

/*

         expand_entry_capacity()
	       ---
	       expands the entry storage capacity of a
	       configuration structure when more space
	       is needed for additional entries.

*/

static commc_error_t expand_entry_capacity(commc_config_t* config) {

    int new_capacity;
    commc_config_entry_t* new_entries;
    
    if (!config) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    new_capacity = config->entry_capacity * ENTRY_GROWTH_FACTOR;
    
    new_entries = realloc(config->entries, 
                         new_capacity * sizeof(commc_config_entry_t));
                         
    if (!new_entries) {
    
        return COMMC_MEMORY_ERROR;
        
    }
    
    config->entries = new_entries;
    config->entry_capacity = new_capacity;
    
    return COMMC_SUCCESS;
    
}

/*

         find_entry()
	       ---
	       finds a configuration entry by section and key,
	       returning the entry index or -1 if not found.

*/

static int find_entry(commc_config_t* config, 
                      const char*     section, 
                      const char*     key) {

    int i;
    
    if (!config || !key) {
    
        return -1;
        
    }
    
    for (i = 0; i < config->entry_count; i++) {
    
        if (strcmp(config->entries[i].key, key) == 0) {
        
            if (!section && config->entries[i].section[0] == '\0') {
            
                return i;
                
            } else if (section && strcmp(config->entries[i].section, section) == 0) {
            
                return i;
                
            }
            
        }
        
    }
    
    return -1;
    
}

/*

         parse_ini_line()
	       ---
	       parses a single line of INI format configuration,
	       handling sections, key-value pairs, and comments.

*/

static commc_error_t parse_ini_line(commc_config_t* config,
                                    char*           line,
                                    int             line_number,
                                    char*           current_section) {

    char* trimmed_line;
    char* equals_pos;
    char* key;
    char* value;
    commc_config_entry_t* entry;
    
    if (!config || !line || !current_section) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    trimmed_line = trim_whitespace(line);
    
    /* Skip empty lines and comments */
    
    if (strlen(trimmed_line) == 0 || 
        trimmed_line[0] == ';' || 
        trimmed_line[0] == '#') {
        
        return COMMC_SUCCESS;
        
    }
    
    /* Handle section headers */
    
    if (trimmed_line[0] == '[') {
    
        char* section_end = strchr(trimmed_line, ']');
        
        if (!section_end) {
        
            return COMMC_FORMAT_ERROR;
            
        }
        
        *section_end = '\0';
        
        if (strlen(trimmed_line + 1) >= COMMC_CONFIG_MAX_SECTION_LENGTH) {
        
            return COMMC_FORMAT_ERROR;
            
        }
        
        strcpy(current_section, trimmed_line + 1);
        return COMMC_SUCCESS;
        
    }
    
    /* Handle key-value pairs */
    
    equals_pos = strchr(trimmed_line, '=');
    
    if (!equals_pos) {
    
        return COMMC_FORMAT_ERROR;
        
    }
    
    *equals_pos = '\0';
    
    key = trim_whitespace(trimmed_line);
    value = trim_whitespace(equals_pos + 1);
    
    if (strlen(key) >= COMMC_CONFIG_MAX_KEY_LENGTH ||
        strlen(value) >= COMMC_CONFIG_MAX_VALUE_LENGTH) {
        
        return COMMC_FORMAT_ERROR;
        
    }
    
    /* Expand capacity if needed */
    
    if (config->entry_count >= config->entry_capacity) {
    
        commc_error_t expand_result = expand_entry_capacity(config);
        
        if (expand_result != COMMC_SUCCESS) {
        
            return expand_result;
            
        }
        
    }
    
    /* Add entry */
    
    entry = &config->entries[config->entry_count];
    
    strcpy(entry->section, current_section);
    strcpy(entry->key, key);
    
    entry->value.type = COMMC_CONFIG_TYPE_STRING;
    entry->value.data.string_value = malloc(strlen(value) + 1);
    
    if (!entry->value.data.string_value) {
    
        return COMMC_MEMORY_ERROR;
        
    }
    
    strcpy(entry->value.data.string_value, value);
    entry->line_number = line_number;
    
    config->entry_count++;
    
    return COMMC_SUCCESS;
    
}

/* 
	==================================
             --- CORE ---
	==================================
*/

/*

         commc_config_create()
	       ---
	       creates and initializes a new configuration
	       structure with default settings.

*/

commc_error_t commc_config_create(commc_config_t** config) {

    commc_config_t* new_config;
    
    if (!config) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    new_config = malloc(sizeof(commc_config_t));
    
    if (!new_config) {
    
        return COMMC_MEMORY_ERROR;
        
    }
    
    /* Initialize structure */
    
    memset(new_config, 0, sizeof(commc_config_t));
    
    new_config->format = COMMC_CONFIG_FORMAT_INI;
    new_config->entry_capacity = INITIAL_ENTRY_CAPACITY;
    
    new_config->entries = malloc(new_config->entry_capacity * sizeof(commc_config_entry_t));
    
    if (!new_config->entries) {
    
        free(new_config);
        return COMMC_MEMORY_ERROR;
        
    }
    
    *config = new_config;
    
    return COMMC_SUCCESS;
    
}

/*

         commc_config_destroy()
	       ---
	       destroys a configuration structure and frees
	       all allocated memory.

*/

void commc_config_destroy(commc_config_t* config) {

    int i;
    
    if (!config) {
    
        return;
        
    }
    
    /* Free entry data */
    
    for (i = 0; i < config->entry_count; i++) {
    
        if (config->entries[i].value.type == COMMC_CONFIG_TYPE_STRING &&
            config->entries[i].value.data.string_value) {
            
            free(config->entries[i].value.data.string_value);
            
        }
        
    }
    
    /* Free arrays */
    
    if (config->entries) {
    
        free(config->entries);
        
    }
    
    if (config->sections) {
    
        for (i = 0; i < config->section_count; i++) {
        
            if (config->sections[i]) {
            
                free(config->sections[i]);
                
            }
            
        }
        
        free(config->sections);
        
    }
    
    free(config);
    
}

/*

         commc_config_load()
	       ---
	       loads and parses a configuration file.

*/

commc_error_t commc_config_load(commc_config_t*       config,
                                const char*           filename,
                                commc_config_format_t format) {

    FILE* file;
    char* buffer;
    size_t file_size;
    size_t bytes_read;
    commc_error_t result;
    
    if (!config || !filename) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Open file */
    
    file = fopen(filename, "rb");
    
    if (!file) {
    
        return COMMC_IO_ERROR;
        
    }
    
    /* Get file size */
    
    if (fseek(file, 0, SEEK_END) != 0) {
    
        fclose(file);
        return COMMC_IO_ERROR;
        
    }
    
    file_size = (size_t)ftell(file);
    
    if (fseek(file, 0, SEEK_SET) != 0) {
    
        fclose(file);
        return COMMC_IO_ERROR;
        
    }
    
    /* Allocate buffer */
    
    buffer = malloc(file_size + 1);
    
    if (!buffer) {
    
        fclose(file);
        return COMMC_MEMORY_ERROR;
        
    }
    
    /* Read file */
    
    bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    
    /* Parse content */
    
    result = commc_config_load_string(config, buffer, bytes_read, format);
    
    if (result == COMMC_SUCCESS) {
    
        strncpy(config->filename, filename, sizeof(config->filename) - 1);
        config->filename[sizeof(config->filename) - 1] = '\0';
        
    }
    
    free(buffer);
    
    return result;
    
}

/*

         commc_config_load_string()
	       ---
	       parses configuration data from a string buffer.

*/

commc_error_t commc_config_load_string(commc_config_t*       config,
                                       const char*           data,
                                       size_t                data_size,
                                       commc_config_format_t format) {

    char* buffer;
    char* line;
    char* line_start;
    char* line_end;
    char current_section[COMMC_CONFIG_MAX_SECTION_LENGTH];
    int line_number = 1;
    commc_error_t result = COMMC_SUCCESS;
    
    if (!config || !data) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Auto-detect format if requested */
    
    if (format == COMMC_CONFIG_FORMAT_AUTO) {
    
        format = commc_config_format_detect(data, data_size);
        
    }
    
    config->format = format;
    
    /* Currently only INI format is fully supported */
    
    if (format != COMMC_CONFIG_FORMAT_INI) {
    
        return COMMC_NOT_IMPLEMENTED_ERROR;
        
    }
    
    /* Copy data to working buffer */
    
    buffer = malloc(data_size + 1);
    
    if (!buffer) {
    
        return COMMC_MEMORY_ERROR;
        
    }
    
    memcpy(buffer, data, data_size);
    buffer[data_size] = '\0';
    
    /* Initialize parsing state */
    
    current_section[0] = '\0';
    line_start = buffer;
    
    /* Parse line by line */
    
    while (line_start < buffer + data_size) {
    
        line_end = strchr(line_start, '\n');
        
        if (!line_end) {
        
            line_end = buffer + data_size;
            
        }
        
        /* Extract line */
        
        line = malloc((size_t)(line_end - line_start) + 1);
        
        if (!line) {
        
            result = COMMC_MEMORY_ERROR;
            break;
            
        }
        
        memcpy(line, line_start, (size_t)(line_end - line_start));
        line[line_end - line_start] = '\0';
        
        /* Remove carriage return if present */
        
        if (strlen(line) > 0 && line[strlen(line) - 1] == '\r') {
        
            line[strlen(line) - 1] = '\0';
            
        }
        
        /* Parse line */
        
        result = parse_ini_line(config, line, line_number, current_section);
        
        free(line);
        
        if (result != COMMC_SUCCESS) {
        
            break;
            
        }
        
        line_number++;
        line_start = line_end + 1;
        
    }
    
    free(buffer);
    
    return result;
    
}

/*

         commc_config_save()
	       ---
	       saves configuration data to a file.

*/

commc_error_t commc_config_save(commc_config_t*       config,
                                const char*           filename,
                                commc_config_format_t format) {

    FILE* file;
    int i;
    const char* current_section = NULL;
    
    if (!config || !filename) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Currently only INI format is supported for output */
    
    if (format != COMMC_CONFIG_FORMAT_INI) {
    
        return COMMC_NOT_IMPLEMENTED_ERROR;
        
    }
    
    file = fopen(filename, "w");
    
    if (!file) {
    
        return COMMC_IO_ERROR;
        
    }
    
    /* Write entries */
    
    for (i = 0; i < config->entry_count; i++) {
    
        /* Write section header if changed */
        
        if (!current_section || strcmp(current_section, config->entries[i].section) != 0) {
        
            if (strlen(config->entries[i].section) > 0) {
            
                if (i > 0) {
                
                    fprintf(file, "\n");
                    
                }
                
                fprintf(file, "[%s]\n", config->entries[i].section);
                
            }
            
            current_section = config->entries[i].section;
            
        }
        
        /* Write key-value pair */
        
        if (config->entries[i].value.type == COMMC_CONFIG_TYPE_STRING) {
        
            fprintf(file, "%s=%s\n", config->entries[i].key,
                   config->entries[i].value.data.string_value);
                   
        }
        
    }
    
    fclose(file);
    
    return COMMC_SUCCESS;
    
}

/* 
	==================================
           --- ACCESSORS ---
	==================================
*/

/*

         commc_config_get_string()
	       ---
	       retrieves a string value from the configuration.

*/

const char* commc_config_get_string(commc_config_t* config,
                                    const char*     section,
                                    const char*     key,
                                    const char*     default_value) {

    int entry_index;
    
    if (!config || !key) {
    
        return default_value;
        
    }
    
    entry_index = find_entry(config, section, key);
    
    if (entry_index < 0) {
    
        return default_value;
        
    }
    
    if (config->entries[entry_index].value.type == COMMC_CONFIG_TYPE_STRING) {
    
        return config->entries[entry_index].value.data.string_value;
        
    }
    
    return default_value;
    
}

/*

         commc_config_get_integer()
	       ---
	       retrieves an integer value from the configuration.

*/

int commc_config_get_integer(commc_config_t* config,
                             const char*     section,
                             const char*     key,
                             int             default_value) {

    const char* string_value;
    
    string_value = commc_config_get_string(config, section, key, NULL);
    
    if (!string_value) {
    
        return default_value;
        
    }
    
    return atoi(string_value);
    
}

/*

         commc_config_get_float()
	       ---
	       retrieves a floating-point value from the configuration.

*/

double commc_config_get_float(commc_config_t* config,
                              const char*     section,
                              const char*     key,
                              double          default_value) {

    const char* string_value;
    
    string_value = commc_config_get_string(config, section, key, NULL);
    
    if (!string_value) {
    
        return default_value;
        
    }
    
    return atof(string_value);
    
}

/*

         commc_config_get_boolean()
	       ---
	       retrieves a boolean value from the configuration.

*/

int commc_config_get_boolean(commc_config_t* config,
                             const char*     section,
                             const char*     key,
                             int             default_value) {

    const char* string_value;
    
    string_value = commc_config_get_string(config, section, key, NULL);
    
    if (!string_value) {
    
        return default_value;
        
    }
    
    return parse_boolean_value(string_value);
    
}

/* 
	==================================
            --- SETTERS ---
	==================================
*/

/*

         commc_config_set_string()
	       ---
	       sets a string value in the configuration.

*/

commc_error_t commc_config_set_string(commc_config_t* config,
                                      const char*     section,
                                      const char*     key,
                                      const char*     value) {

    int entry_index;
    commc_config_entry_t* entry;
    char* new_value;
    
    if (!config || !key || !value) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    entry_index = find_entry(config, section, key);
    
    if (entry_index >= 0) {
    
        /* Update existing entry */
        
        entry = &config->entries[entry_index];
        
        if (entry->value.type == COMMC_CONFIG_TYPE_STRING &&
            entry->value.data.string_value) {
            
            free(entry->value.data.string_value);
            
        }
        
    } else {
    
        /* Create new entry */
        
        if (config->entry_count >= config->entry_capacity) {
        
            commc_error_t expand_result = expand_entry_capacity(config);
            
            if (expand_result != COMMC_SUCCESS) {
            
                return expand_result;
                
            }
            
        }
        
        entry = &config->entries[config->entry_count];
        config->entry_count++;
        
        /* Set section and key */
        
        if (section) {
        
            if (strlen(section) >= COMMC_CONFIG_MAX_SECTION_LENGTH) {
            
                return COMMC_ERROR_BUFFER_TOO_SMALL;
                
            }
            
            strcpy(entry->section, section);
            
        } else {
        
            entry->section[0] = '\0';
            
        }
        
        if (strlen(key) >= COMMC_CONFIG_MAX_KEY_LENGTH) {
        
            return COMMC_ERROR_BUFFER_TOO_SMALL;
            
        }
        
        strcpy(entry->key, key);
        entry->line_number = 0;
        
    }
    
    /* Set value */
    
    new_value = malloc(strlen(value) + 1);
    
    if (!new_value) {
    
        return COMMC_MEMORY_ERROR;
        
    }
    
    strcpy(new_value, value);
    
    entry->value.type = COMMC_CONFIG_TYPE_STRING;
    entry->value.data.string_value = new_value;
    
    return COMMC_SUCCESS;
    
}

/* 
	==================================
           --- UTILITIES ---
	==================================
*/

/*

         commc_config_has_section()
	       ---
	       checks if a configuration section exists.

*/

int commc_config_has_section(commc_config_t* config,
                             const char*     section) {

    int i;
    
    if (!config || !section) {
    
        return 0;
        
    }
    
    for (i = 0; i < config->entry_count; i++) {
    
        if (strcmp(config->entries[i].section, section) == 0) {
        
            return 1;
            
        }
        
    }
    
    return 0;
    
}

/*

         commc_config_has_key()
	       ---
	       checks if a specific key exists within a section.

*/

int commc_config_has_key(commc_config_t* config,
                         const char*     section,
                         const char*     key) {

    return find_entry(config, section, key) >= 0;
    
}

/*

         commc_config_format_detect()
	       ---
	       automatically detects the configuration file format.

*/

commc_config_format_t commc_config_format_detect(const char* data,
                                                  size_t      data_size) {

    size_t i;
    
    /* This is a simplified detection algorithm */
    /* In practice, this would be more sophisticated */
    
    if (!data || data_size == 0) {
    
        return COMMC_CONFIG_FORMAT_INI;
        
    }
    
    /* Look for INI-style section headers */
    
    for (i = 0; i < data_size; i++) {
    
        if (data[i] == '[') {
        
            return COMMC_CONFIG_FORMAT_INI;
            
        }
        
    }
    
    /* Default to INI format */
    
    return COMMC_CONFIG_FORMAT_INI;
    
}

/* 
	==================================
             --- EOF ---
	==================================
*/