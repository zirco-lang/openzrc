#include <stage2.h>
#include <lexer.h>
#include <parser.h>
#include <util.h>
#include <emitter_llvm.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int stage2(char * original_file, char * in_file, char * out_file, stage2_output_type output_type) {
  
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

  token literals[256];
  int num_literals = 0;
  for (int i = 0; i < num_tokens; i++) {
    if (tokens[i].type == LT_INT_LITERAL || tokens[i].type == LT_STRING_LITERAL) {
      literals[num_literals].type = tokens[i].type;
      literals[num_literals].data = tokens[i].data;
      literals[num_literals].data_len = tokens[i].data_len;

      tokens[i].data = 0;
      tokens[i].data_len = num_literals;
      tokens[i].type = LT_LITERAL;
      num_literals++;
    }
  }
  parse_root_t * parsed = parse(tokens, num_tokens, literals);

  FILE* out = fopen(out_file, "w");
  
  if (output_type == OT_TOKENS) {
    print_tokens(out, tokens, &num_tokens);
    fclose(out);
  } else if (output_type == OT_TREE) {
    print_tree(out, *parsed);
    fprintf(out, "\n");
    fclose(out);
  } else {
    gen_llvm(*parsed, original_file, out, literals, num_literals);
    fclose(out);
    char * base_out = alloc_substring(strlen(original_file) - 2, original_file, 0);
    if (output_type != OT_LLVM) {
      char cmd1[256];
      sprintf(cmd1, "opt -O3 -S %s -o %sll && rm %s", out_file, base_out, out_file);
      int ret = system(cmd1);
      if (ret) return ret;
      char cmd2[256];
      sprintf(cmd2, "clang -c -o %so %sll && rm %sll", base_out, base_out, base_out);
      ret = system(cmd2);
      if (ret) return ret;
      char cmd3[256];
      sprintf(cmd3, "clang -lc -o %s %so && rm %so", out_file, base_out, base_out);
      ret = system(cmd3);
      if (ret) return ret;
    }
  }
  return 0;
}
