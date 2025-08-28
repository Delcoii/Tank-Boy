#include "ini_parser.h"


#define INITIAL_CAPACITY 32
#define MAX_LINE_LENGTH 256

// Create a new INI parser
IniParser* ini_parser_create(void) {
    IniParser* parser = malloc(sizeof(IniParser));
    if (!parser) return NULL;
    
    parser->capacity = INITIAL_CAPACITY;
    parser->count = 0;
    parser->entries = malloc(sizeof(IniEntry) * parser->capacity);
    
    if (!parser->entries) {
        free(parser);
        return NULL;
    }
    
    return parser;
}

// Destroy INI parser and free memory
void ini_parser_destroy(IniParser* parser) {
    if (!parser) return;
    
    for (int i = 0; i < parser->count; i++) {
        parser->entries[i].section = NULL;
        parser->entries[i].key = NULL;
        parser->entries[i].value = NULL;

        free(parser->entries[i].section);
        free(parser->entries[i].key);
        free(parser->entries[i].value);
    }
    
    free(parser->entries);
    free(parser);
}

// Expand parser capacity if needed
static bool expand_capacity(IniParser* parser) {
    int new_capacity = parser->capacity * 2;
    IniEntry* new_entries = realloc(parser->entries, sizeof(IniEntry) * new_capacity);
    
    if (!new_entries) return false;
    
    parser->entries = new_entries;
    parser->capacity = new_capacity;
    return true;
}

// Add a new entry to the parser
static bool add_entry(IniParser* parser, const char* section, const char* key, const char* value) {
    if (parser->count >= parser->capacity) {
        if (!expand_capacity(parser)) return false;
    }
    
    parser->entries[parser->count].section = _strdup(section);
    parser->entries[parser->count].key = _strdup(key);
    parser->entries[parser->count].value = _strdup(value);
    
    if (!parser->entries[parser->count].section || !parser->entries[parser->count].key || !parser->entries[parser->count].value) {
        free(parser->entries[parser->count].section);
        free(parser->entries[parser->count].key);
        free(parser->entries[parser->count].value);
        return false;
    }
    
    parser->count++;
    return true;
}

// Trim whitespace from string
static char* trim(char* str) {
    char* end;
    
    // Trim leading space
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
    return str;
}

// Load INI file
bool ini_parser_load_file(IniParser* parser, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;
    
    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    char* current_section = _strdup(""); // Default section
    
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        char* trimmed_line = trim(line);
        
        // Skip empty lines and comments
        if (strlen(trimmed_line) == 0 || trimmed_line[0] == '#') {
            continue;
        }
        
        // Check for section header [section_name]
        if (trimmed_line[0] == '[') {
            char* end_bracket = strchr(trimmed_line, ']');
            if (end_bracket) {
                *end_bracket = '\0';
                char* section_name = trim(trimmed_line + 1);
                free(current_section);
                current_section = _strdup(section_name);
                continue;
            }
        }
        
        // Parse key=value
        char* separator = strchr(trimmed_line, '=');
        if (!separator) {
            printf("Warning: Invalid line %d in %s: %s\n", line_number, filename, trimmed_line);
            continue;
        }
        
        *separator = '\0';
        char* key = trim(trimmed_line);
        char* value = trim(separator + 1);
        
                    if (strlen(key) == 0) {
                continue;
            }
        
        if (!add_entry(parser, current_section, key, value)) {
            free(current_section);
            fclose(file);
            return false;
        }
    }
    
    free(current_section);
    fclose(file);
    return true;
}

// Get string value
const char* ini_parser_get_string(IniParser* parser, const char* section, const char* key, const char* default_value) {
    if (!parser || !section || !key) return default_value;
    
    for (int i = 0; i < parser->count; i++) {
        if (strcmp(parser->entries[i].section, section) == 0 && strcmp(parser->entries[i].key, key) == 0) {
            return parser->entries[i].value;
        }
    }
    
    return default_value;
}

// Get integer value
int ini_parser_get_int(IniParser* parser, const char* section, const char* key, int default_value) {
    const char* value = ini_parser_get_string(parser, section, key, NULL);
    if (!value) return default_value;
    
    char* endptr;
    int result = strtol(value, &endptr, 10);
    
    if (*endptr != '\0') return default_value;
    return result;
}

// Get float value
float ini_parser_get_float(IniParser* parser, const char* section, const char* key, float default_value) {
    const char* value = ini_parser_get_string(parser, section, key, NULL);
    if (!value) return default_value;
    
    char* endptr;
    float result = strtof(value, &endptr);
    
    if (*endptr != '\0') return default_value;
    return result;
}

// Get double value
double ini_parser_get_double(IniParser* parser, const char* section, const char* key, double default_value) {
    const char* value = ini_parser_get_string(parser, section, key, NULL);
    if (!value) return default_value;
    
    char* endptr;
    double result = strtod(value, &endptr);
    
    if (*endptr != '\0') return default_value;
    return result;
}

// Path resolution helper function
void ini_parser_resolve_path(const char* source_file, const char* config_file, char* full_path, size_t path_size) {
    // Find the last directory separator
    const char* last_slash = strrchr(source_file, '\\');
    if (!last_slash) last_slash = strrchr(source_file, '/');
    
    if (last_slash) {
        // Copy directory part
        size_t dir_len = last_slash - source_file + 1;
        if (dir_len < path_size) {
#pragma warning(push)
#pragma warning(disable: 4996)
            strncpy(full_path, source_file, dir_len);
            full_path[dir_len] = '\0';
            
            // Append config filename
            strcat(full_path, config_file);
#pragma warning(pop)
        } else {
            strcpy_s(full_path, path_size, config_file);
        }
    } else {
        // No directory found, use relative path
        strcpy_s(full_path, path_size, config_file);
    }
}

// Get boolean value
bool ini_parser_get_bool(IniParser* parser, const char* section, const char* key, bool default_value) {
    const char* value = ini_parser_get_string(parser, section, key, NULL);
    if (!value) return default_value;
    
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "yes") == 0) {
        return true;
    }
    if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0 || strcmp(value, "no") == 0) {
        return false;
    }
    
    return default_value;
}

bool ini_parser_load_with_defaults(const char* filename, void* config_struct,
    void (*init_defaults)(void*),
    void (*load_values)(IniParser*, void*)) {
    if (init_defaults) init_defaults(config_struct);
    IniParser* parser = ini_parser_create();
    if (!parser) return false;
    if (!ini_parser_load_file(parser, filename)) {
        ini_parser_destroy(parser);
        return false;
    }
    if (load_values) load_values(parser, config_struct);
    ini_parser_destroy(parser);
    return true;
}