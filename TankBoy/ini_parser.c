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
static bool add_entry(IniParser* parser, const char* key, const char* value) {
    if (parser->count >= parser->capacity) {
        if (!expand_capacity(parser)) return false;
    }
    
    parser->entries[parser->count].key = _strdup(key);
    parser->entries[parser->count].value = _strdup(value);
    
    if (!parser->entries[parser->count].key || !parser->entries[parser->count].value) {
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
    
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        char* trimmed_line = trim(line);
        
        // Skip empty lines and comments
        if (strlen(trimmed_line) == 0 || trimmed_line[0] == '#' || trimmed_line[0] == '[') {
            continue;
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
            printf("Warning: Empty key at line %d in %s\n", line_number, filename);
            continue;
        }
        
        if (!add_entry(parser, key, value)) {
            fclose(file);
            return false;
        }
    }
    
    fclose(file);
    return true;
}

// Get string value
const char* ini_parser_get_string(IniParser* parser, const char* key, const char* default_value) {
    if (!parser || !key) return default_value;
    
    for (int i = 0; i < parser->count; i++) {
        if (strcmp(parser->entries[i].key, key) == 0) {
            return parser->entries[i].value;
        }
    }
    
    return default_value;
}

// Get integer value
int ini_parser_get_int(IniParser* parser, const char* key, int default_value) {
    const char* value = ini_parser_get_string(parser, key, NULL);
    if (!value) return default_value;
    
    char* endptr;
    int result = strtol(value, &endptr, 10);
    
    if (*endptr != '\0') return default_value;
    return result;
}

// Get float value
float ini_parser_get_float(IniParser* parser, const char* key, float default_value) {
    const char* value = ini_parser_get_string(parser, key, NULL);
    if (!value) return default_value;
    
    char* endptr;
    float result = strtof(value, &endptr);
    
    if (*endptr != '\0') return default_value;
    return result;
}

// Get boolean value
bool ini_parser_get_bool(IniParser* parser, const char* key, bool default_value) {
    const char* value = ini_parser_get_string(parser, key, NULL);
    if (!value) return default_value;
    
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "yes") == 0) {
        return true;
    }
    if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0 || strcmp(value, "no") == 0) {
        return false;
    }
    
    return default_value;
}