#pragma once
#include <lexer.h>
#include <stdio.h>

#define FOREACH_PARSER_EXPR_TYPE(PT) \
        PT(ET_UNKNOWN) \
        PT(ET_LITERAL) \
        PT(ET_FN_CALL) \
        PT(ET_RETURN) \
        PT(ET_SCOPE) \
        PT(ET_DEFINE) \
        PT(ET_LABEL)

#define FOREACH_ROOT_TYPE(RT) \
        RT(RT_FN) \


#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum parse_expr_type {
    FOREACH_PARSER_EXPR_TYPE(GENERATE_ENUM)
} parse_expr_type_t;

static const char *parse_expr_names[] = {
    FOREACH_PARSER_EXPR_TYPE(GENERATE_STRING)
};

typedef enum parse_root_type {
    FOREACH_ROOT_TYPE(GENERATE_ENUM)
} parse_root_type_t;

static const char *parse_root_names[] = {
    FOREACH_ROOT_TYPE(GENERATE_STRING)
};

typedef struct parse_expr {
    parse_expr_type_t type;
    void * data;
} parse_expr_t;

typedef struct function_call {
    token * name;
    parse_expr_t args[32];
    int num_args;
} parse_fn_call_t;

typedef struct type {
    int num_ptr;
    token * name;
} parse_type_t;

typedef struct define {

    token * name;
    parse_type_t * typ;
} parse_define_t;

typedef struct arg {
    int typ;
    parse_define_t def;
} parse_arg_t;

typedef struct function {
    token * name;
    parse_arg_t args[32];
    int num_args;
    parse_type_t ret;
    parse_expr_t body;
} parse_fn_t;

typedef struct scope {
    parse_expr_t expr[512];
    int num_expr;
} parse_scope_t;

typedef struct root_item {
    parse_root_type_t typ;
    void* data;
} parse_root_item_t;

typedef struct root {
    parse_root_item_t item[256];
    int num_items;
} parse_root_t;


/**
 * Parses a new expression from tokens and a number of them;
 */
parse_root_t * parse(token * tokens, int num_tokens);

/**
 * Prints the tree to the output.
 */
void print_tree(FILE * out, parse_root_t t);