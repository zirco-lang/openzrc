#ifndef __LEXER_H
#define __LEXER_H

#include <stdio.h>

/**
 List of supported tokens
*/
enum ZR_TOK {
  TOK_EOF = 0,
  TOK_LET,
  TOK_COLON,
  TOK_SEMICOLON,
  TOK_I32,
  TOK_IDENTIFIER,
  TOK_LITERAL,
  TOK_EQ,
  TOK_PLUS,
  TOK_OPEN_CURLY,
  TOK_CLOSE_CURLY,
  TOK_RETURN,
  TOK_UNKNOWN = -1,
};

/*
 * Token Structure for parsing
 */
typedef struct {
  int tok;
  char * val;
} TOKEN;


void print_token(int tok, char** buf);

int lexer(FILE * fptr, TOKEN ** tokens, int tok_len);

#endif
