#include "lexer.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>

/*
 * Checks single-character tokens
 */
int check_char_toks(int ch) {
  unsigned char ch2 = (unsigned char) ch;
  if (ch == EOF) return TOK_EOF;
  switch (ch2) {
  case ':':
    return TOK_COLON;
    break;
  case ';':
    return TOK_SEMICOLON;
    break;
  case '=':
    return TOK_EQ;
    break;
  case '+':
    return TOK_PLUS;
    break;
  case '-':
    return TOK_MINUS;
  case '{':
    return TOK_OPEN_CURLY;
    break;
  case '}':
    return TOK_CLOSE_CURLY;
  default:
    return TOK_UNKNOWN;
  }
  return TOK_UNKNOWN;
}

/*
 * Gathers the next token
 */
  
int next_tok(FILE * fptr, char ** out) {
  // Create buffer for temporary storage
  char buf[255];
  int idx = 0;
  int ch = fgetc(fptr);
  char ch2 = (char) ch;

  // Remove any whitespace before next token
  while (ch2 == ' ' || ch2 == '\t' || ch2 == '\n') {
    ch = fgetc(fptr);
    ch2 = (char) ch;
  }

  // Check for single-character values
  int val = check_char_toks(ch);
  if (val != TOK_UNKNOWN) return val;

  // Check for alphabetical
  while ((ch2 >= 'A' && ch2 <= 'Z') || (ch2 >= 'a' && ch2 <= 'z')) {
    buf[idx] = ch2;
    idx++;
    ch = fgetc(fptr);
    ch2 = (char) ch;
  } 

  // check for keywords (only let is found here for now...)
  buf[idx] = 0;
  if (strcmp(buf, "let") == 0) return TOK_LET;
  if (strcmp(buf, "return") == 0) return TOK_RETURN;
  // this is a case to find things that start with numbers
  int start = 0;
  if (idx > 0) start = 1;

  // check for numbers
  while (ch2 >= '0' && ch2 <= '9') {
    buf[idx] = ch2;
    idx++;
    ch = fgetc(fptr);
    ch2 = (char) ch;
  }

  // go back a step as we sometimes go past where we need to when checking
  buf[idx] = 0;
  if (idx > 0) fseek(fptr, -1, SEEK_CUR);

  // set output buffer
  strcpy(*out, buf);
  // check for type definitions (only i32 for now...)
  if (strcmp(buf, "i32") == 0) return TOK_I32;

  // this is either an identifier or a literal of some sort...
  if (idx > 0) {
    // if it starts with a number, it's always a literal.
    if (start == 0) {
      return TOK_LITERAL;
    }
    return TOK_IDENTIFIER;
  }

  // write our single-character unknowns here...
  snprintf(*out, 2, "%c", ch2);
  return TOK_UNKNOWN;
}

/*
 * Prints a token nicely...
 */
void print_token(int tok, char** buf) {
  switch (tok) {
  case TOK_EOF:
    printf("EOF\n");
    break;
  case TOK_COLON:
    printf(":\n");
    break;
  case TOK_LET:
    printf("let\n");
    break;
  case TOK_SEMICOLON:
    printf(";\n");
    break;
  case TOK_MINUS:
    printf("-\n");
    break;
  case TOK_I32:
    printf("i32\n");
    break;
  case TOK_IDENTIFIER:
    printf("identifier: %s\n", *buf);
    break;
  case TOK_LITERAL:
    printf("literal: %s\n", *buf);
    break;
  case TOK_EQ:
    printf("=\n");
    break;
  case TOK_PLUS:
    printf("+\n");
    break;
  case TOK_OPEN_CURLY:
    printf("{\n");
    break;
  case TOK_CLOSE_CURLY:
    printf("}\n");
    break;
  case TOK_RETURN:
    printf("return\n");
    break;
  case TOK_UNKNOWN:
    printf("Unknown value: %s\n", *buf);
    break;
  default:
    {
    char error[128];
    snprintf(error, 128, "Unimplemented token value in print_tok: %d", tok);
    quit(1, error);
    }
    break;
  }
}



int lexer(FILE * fptr, TOKEN ** tokens, int tok_len) {
  int val;
  //allocate first token
  int i = 0;
  (tokens[0][i].val) = malloc(255);

  // allocate rest of tokens that are found
  while ((val = next_tok(fptr, &(tokens[0][i].val))) != 0 && i < tok_len) {
    tokens[0][i].tok = val;
    i++;
    (tokens[0][i].val) = malloc(255);

  }

  // output last token value
  tokens[0][i].tok = val;

  // this is the final size...
  i++;
  return i;
}
