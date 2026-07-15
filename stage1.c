#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


//===================================
// Begin Lexer
//
//==================================

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
  TOK_UNKNOWN = -1,
};

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
  case TOK_UNKNOWN:
    printf("Unknown value: %s\n", *buf);
    break;
  default:
    printf("Unimplemented token: %s\n", *buf);
    break;
  }
}

/*
 * Token Structure for parsing
 */
typedef struct {
  int tok;
  char * val;
} TOKEN;


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

//====================
// PARSER
//
//====================

  case TOK_I32:
  default:
//====================
// Main Program
//====================

/*
 * This is our main program function
 */
int main(int argc, char ** argv) {
  printf("openzrc stage 1 compiler\n");


  // read a zirco file.
  char * file = "./example.zr";
  if (argc > 1) {
    file = argv[1];
  }
  FILE * fptr;
  fptr = fopen(file, "r");
  if (fptr == NULL) {
    printf("Error reading file: %s\n", file);
    return 1;
  }

  // allocate a bunch of tokens to read. this number is low for testing.
  int alloc_tokens = 32;
  TOKEN * tokens = malloc(sizeof(TOKEN) * alloc_tokens);

  // run the lexer and stop reading from the file.
  int num_tokens = lexer(fptr, &tokens, alloc_tokens);
  fclose(fptr);

  for (int i = 0; i < num_tokens; i++) {
    free(tokens[i].val);
  }
  free(tokens);
  
  return 0;
}
