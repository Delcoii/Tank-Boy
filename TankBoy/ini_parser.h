#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// INI 엔트리 구조체
typedef struct {
    char* section;
    char* key;
    char* value;
} IniEntry;

typedef struct {
    IniEntry* entries;
    int count;
    int capacity;
} IniParser;

// 생성/소멸
IniParser* ini_parser_create(void);
void ini_parser_destroy(IniParser* parser);

// 파일 로딩
bool ini_parser_load_file(IniParser* parser, const char* filename);

// 값 추출
const char* ini_parser_get_string(IniParser* parser, const char* section, const char* key, const char* default_value);
int ini_parser_get_int(IniParser* parser, const char* section, const char* key, int default_value);
float ini_parser_get_float(IniParser* parser, const char* section, const char* key, float default_value);
double ini_parser_get_double(IniParser* parser, const char* section, const char* key, double default_value);
bool ini_parser_get_bool(IniParser* parser, const char* section, const char* key, bool default_value);

// 편의 함수
bool ini_parser_load_with_defaults(const char* filename, void* config_struct, 
                                   void (*init_defaults)(void*), 
                                   void (*load_values)(IniParser*, void*));

// 경로 해석
void ini_parser_resolve_path(const char* source_file, const char* config_file, char* full_path, size_t path_size);

#endif // INI_PARSER_H
