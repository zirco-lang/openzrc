#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.h"
/*
 * Types of values in a parser
 */
enum parser_type {
  PARSER_STMT = 0,
  PARSER_EXPR,
  PARSER_LET_DECL,
  PARSER_RETURN,
  PARSER_BINARY_OP,
  PARSER_TOKEN,
  PARSER_LIST,
  PARSER_UNKNOWN = -1,
};

/*
 * Grammar items
 */
typedef struct {
  int typ;
  void * val;
} GRAMMAR_T;

/*
 * Linked List of Grammar_ts
 */
typedef struct grammar_list {
  GRAMMAR_T * val;
  struct grammar_list * next;  
} GRAMMAR_LIST;

/*
 * Special case for `let` statements
 */
typedef struct {
  TOKEN * identifier;
  TOKEN * typ;
  GRAMMAR_T * val;
} LET_DECL;

/*
 * Case for unary operators
 */
typedef struct {
  GRAMMAR_T * lhs;
  GRAMMAR_T * rhs;
  TOKEN * typ;
} BINARY_OP;

int free_parser(GRAMMAR_T * out);

int parser(TOKEN ** tokens, int alloc_tokens, GRAMMAR_T * out);

void print_tree(GRAMMAR_T * out);

#endif

