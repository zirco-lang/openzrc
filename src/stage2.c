#include <stage2.h>
#include <lexer.h>
#include <parser.h>
#include <util.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int stage2(char * in_file, char * out_file, stage2_output_type output_type) {
  
  FILE* in = fopen(in_file, "r");
  char* buffer = add_alloc(1024 * 16);
  int len = fread((void*)buffer, sizeof(char), 1024*16, in);
  fclose(in);

  if (len == 0) {
    printf("File is empty! Skipping!\n");
    return 1;
  }
  
  token tokens[2048];
  int num_tokens = 0;
  
  tokenize(tokens, &num_tokens, buffer);
  parse_root_t * parsed = parse(tokens, num_tokens);

  FILE* out = fopen(out_file, "w");
  
  if (output_type == OT_TOKENS) {
    print_tokens(out, tokens, &num_tokens);
  } else if (output_type == OT_TREE) {
    print_tree(out, *parsed);
    fprintf(out, "\n");
  } else printf("output type unimplemented!\n");
  fclose(out);
  return 0;
}
