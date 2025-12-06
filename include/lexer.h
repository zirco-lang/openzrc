#pragma once
#include <stdio.h>

#define FOREACH_LEXER_TYPE(LT) \
        LT(LT_UNKNOWN) \
        LT(LT_LABEL) \
        LT(LT_SEMICOLON) \
        LT(LT_COLON) \
        LT(LT_OPEN_PAREN) \
        LT(LT_CLOSE_PAREN) \
        LT(LT_OPEN_BRACKET) \
        LT(LT_CLOSE_BRACKET) \
        LT(LT_OPEN_CURLY) \
        LT(LT_CLOSE_CURLY) \
        LT(LT_INT_LITERAL) \
        LT(LT_STRING_LITERAL) \
        LT(LT_LITERAL) \
        LT(LT_ASTERISK) \
        LT(LT_EQUALS) \
        LT(LT_COMMA) \
        LT(LT_HYP) \
        LT(LT_GREATER) \
        LT(LT_LESSER) \
        LT(LT_DOT) \


#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum lexer_type {
    FOREACH_LEXER_TYPE(GENERATE_ENUM)
} lexer_type;

static const char *lexer_type_names[] = {
    FOREACH_LEXER_TYPE(GENERATE_STRING)
};

typedef struct token {
  lexer_type type;
  void* data;
  int data_len;
} token;

/**
 * Prints a single token.
 * out - output file
 * token - token
 */
void print_token(FILE * out, token token);

/**
 * Prints a number of tokens
 * out - output files
 * tokens - tokens array
 * num_tokens - number of tokens
 */
void print_tokens(FILE* out, token * tokens, int * num_tokens);

/**
 * Converts a buffer to a series of tokens
 * tokens - token output
 * num_tokens - number of tokens
 * buffer - input buffer
 */
void tokenize(token * tokens, int * num_tokens, char * buffer);