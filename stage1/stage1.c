#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "util.h"


GRAMMAR_T* parse_tree_main = 0;
TOKEN * tokens_main = 0;
int alloc_tokens_main = 0;

int num_tokens_main = 0;

/*
 * exit the program
 */
void quit(int code, char * reason) {
   if (code != 0) {
     printf("Program crashed! Reason: %s\n", reason);
   }
   
   if (parse_tree_main != 0) free_parser(parse_tree_main);
   if (tokens_main != 0) {
     for (int i = 0; i < num_tokens_main; i++) {
       free(tokens_main[i].val);
     }
     free(tokens_main);
   }
   
   exit(code);
 }
 
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
  alloc_tokens_main = 32;
  tokens_main = malloc(sizeof(TOKEN) * alloc_tokens_main);

  // run the lexer and stop reading from the file.
  num_tokens_main = lexer(fptr, &tokens_main, alloc_tokens_main);
  fclose(fptr);

  // set up the parser
  parse_tree_main = malloc(sizeof(GRAMMAR_T));
  (*parse_tree_main).typ = PARSER_UNKNOWN;

  // run the parser
  int parsed = parser(&tokens_main, alloc_tokens_main, parse_tree_main);
  if (parsed < 0) {
    printf("unable to parse information properly\n");
  }
  
  // Print the parse tree
  printf("Parse tree:\n");
  print_tree(parse_tree_main);

  // start to generate some basic code
  gen_code(file, "output.bc", parse_tree_main);

  // free the parser and lexer since we are good.
  quit(0, "success");
  return 0;
}
