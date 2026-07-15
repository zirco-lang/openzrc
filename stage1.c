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

/*
 * Types of values in a parser
 */
enum parser_type {
  PARSER_STMT = 0,
  PARSER_EXPR,
  PARSER_LET_DECL,
  PARSER_UNARY_OP,
  PARSER_TOKEN,
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
 * Special case for `let` statements
 */
typedef struct {
  TOKEN * identifier;
  TOKEN * typ;
  GRAMMAR_T * val;
} LET_DECL;

/*
 * Case for unary operators
 * TODO: actually implement this...
 */
typedef struct {
  GRAMMAR_T * lhs;
  GRAMMAR_T * rhs;
  TOKEN * typ;
} UNARY_OP;
/*
 * Parse type definitions
 */
int parse_type(TOKEN ** tokens, int alloc_tokens, int idx) {
  switch (tokens[0][idx].tok) {
  case TOK_I32:
    return TOK_I32;
    break;
    //TODO: more types
  default:
    return -1;
  }
}

/*
 * Parse an expression
 */
int parse_expr(TOKEN ** tokens, int alloc_tokens, GRAMMAR_T * out, int idx) {
  if (idx+1 < alloc_tokens && tokens[0][idx+1].tok == TOK_PLUS) {
    UNARY_OP * unary = malloc(sizeof(UNARY_OP));
    GRAMMAR_T * lhs = malloc(sizeof(GRAMMAR_T));
    GRAMMAR_T * rhs = malloc(sizeof(GRAMMAR_T));
    
    lhs->typ = PARSER_TOKEN;
    lhs->val = (void*)(*tokens+idx);

    rhs->typ = PARSER_TOKEN;
    rhs->val = (void*)(*tokens+idx+2);

    unary->lhs = lhs;
    unary->rhs = rhs;
    unary->typ = (*tokens+idx+1);

    out->typ = PARSER_UNARY_OP;
    out->val = (void*)unary;
    return 3;
  }
  
  out->typ = PARSER_TOKEN;
  out->val = (void*)(*tokens+idx);
  
  return 1;
}

/*
  Parse a `let` declaration
 */
int parse_let_decl(TOKEN ** tokens, int alloc_tokens, LET_DECL * out, int idx) {

  // child value is unknown until needed
  GRAMMAR_T * val = malloc(sizeof(GRAMMAR_T));
  val->typ = PARSER_UNKNOWN;

  // check for valid definition
  if (tokens[0][idx+1].tok != TOK_IDENTIFIER ||
      tokens[0][idx+2].tok != TOK_COLON ||
      parse_type(tokens, alloc_tokens, idx+3) == -1
      ) {
    return -1;
  }

  // initialize output
  out->identifier = (*tokens)+idx+1;
  out->typ = (*tokens)+idx+3;
  int amt = 4;

  // if there is an equal sign, we are also defining. check for an expression afterwards.
  if (tokens[0][idx+4].tok == TOK_EQ) {
    amt += parse_expr(tokens, alloc_tokens, val, idx+5);
  }
  
  out->val = val;
  return amt;
}

/*
 * Parse a statement
 */
int parse_stmt(TOKEN ** tokens, int alloc_tokens, GRAMMAR_T * out, int idx) {
  TOKEN current = tokens[0][idx];
  // an empty semicolon is a valid statement
  if (current.tok == TOK_SEMICOLON) {
    out->typ = PARSER_TOKEN;
    out->val = (void*)((*tokens)+idx);
    return 1;
  }
  // check for a let identifier
  if (current.tok == TOK_LET) {
    out->typ = PARSER_LET_DECL;
    LET_DECL * val = malloc(sizeof(LET_DECL));
    out->val = (void *) val;
    return parse_let_decl(tokens, alloc_tokens, val, idx) + 1;
  }
  // in this case we don't know what we just hit, so check it.
  out->typ = PARSER_UNKNOWN;
  return 1;
  
}
/*
 * This frees the memory allocated by the parser
 */
int free_parser(GRAMMAR_T * out) {
  if (out == 0) return 0;
  switch (out->typ) {
    // statements always have a value
  case PARSER_STMT:
  case PARSER_EXPR:
    {
      free_parser((GRAMMAR_T*)(out->val));
    }
    break;
    // let decls have their own objects
  case PARSER_LET_DECL:
    {
      LET_DECL * decl = (LET_DECL*)(out->val);
      free_parser((GRAMMAR_T*)(decl->val));
      free(decl);
    }
    break;
    // same with unary operators
  case PARSER_UNARY_OP:
    {
      UNARY_OP * decl = (UNARY_OP*)(out->val);
      free_parser((GRAMMAR_T*)(decl->rhs));
      free_parser((GRAMMAR_T*)(decl->lhs));
      free(decl);
    }
    break;
  case PARSER_LIST:
  }

  // this actually frees the current object
  free(out);
  return 0;
}

/*
 * This is for early development, and prints a parse tree
 */
void print_tree(GRAMMAR_T * out) {
  switch (out->typ) {
  case PARSER_STMT:
    printf("Begin Statement\n");
    print_tree((GRAMMAR_T*)(out->val));
    printf("End Statement\n");
    break;
  case PARSER_EXPR:
    printf("Begin EXPR\n");
    print_tree((GRAMMAR_T*)(out->val));
    printf("End EXPR\n");
    break;
  case PARSER_LET_DECL:
    {
      printf("Begin Let Decl:\n");
      LET_DECL * val = (LET_DECL*)(out->val);
      printf("  identifier: %s\n", val->identifier->val);
      printf("  typ: %s\n", val->typ->val);
      print_tree(val->val);
      printf("End let decl\n");
    }
    break;
  case PARSER_UNARY_OP:
    {
      UNARY_OP * val = (UNARY_OP*)(out->val);
      printf("Begin Unary OP\n");
      printf("typ: \n  ");
      print_token(val->typ->tok, &val->typ->val);
      printf("begin lhs\n");
      print_tree(val->lhs);
      printf("end lhs\n");
      printf("begin rhs\n");
      print_tree(val->rhs);
      printf("end rhs\n");
      printf("End unary op\n");
    }
    break;
  case PARSER_TOKEN:
    {
      TOKEN * tok = (TOKEN*)(out->val);
      printf("Token: \n  ");
      print_token(tok->tok, &tok->val);
    }
    break;
  }
}

/*
 * This is the main parser
 */
int parser(TOKEN ** tokens, int alloc_tokens, GRAMMAR_T * out) {
  // since we aren't ready for a full implementation, we are just parsing a statement.
  out->typ = PARSER_STMT;
  int idx = 0;
  GRAMMAR_T * val = malloc(sizeof(GRAMMAR_T));
  out->val = (void*) val;
  int tmp = parse_stmt(tokens, alloc_tokens, val, idx);
  if (tmp < 0) {
    return tmp;
  }
  idx += tmp;
  return idx;
}

  }
  }
  }
  }
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
