#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stage1.h>
#include <stage2.h>
#include <util.h>

/**
  Main function, reads arguments and runs the program
*/
int main(int argc, char * argv[]) {

  // Input file
  char * file_name = add_alloc(256);
  strcpy(file_name, "./input.zr");

  // Output file
  char * output_name = add_alloc(256);
  strcpy(output_name, "./a.out");
  
  // include paths
  char ** include_dirs = (char**)add_alloc(sizeof(char*) * 32);
  int num_includes = 0;

  // read arguments
  int current_arg = 1;
  int stage = 5;
  stage2_output_type output_type_2 = OT_TREE;
  for (int i = 1; i < argc; i++) {

    // Input file
    if (strcmp(argv[i], "-f") == 0) {
      i++;
      strcpy(file_name, argv[i]);
    }
    else if (strcmp(argv[i], "--emit") == 0) {
      i++;
      if (strcmp(argv[i], "tree") == 0) {
        output_type_2 = OT_TREE;
      } else if (strcmp(argv[i], "tokens") == 0) {
        output_type_2 = OT_TOKENS;
      }
    }
    else if (strcmp(argv[i], "-o") == 0) {
      i++;
      strcpy(output_name, argv[i]);
    }
    // Include paths
    else if (argv[i][0] == '-' && argv[i][1] == 'I') {
      include_dirs[num_includes] = add_alloc(64);

      for (int j = 2; j < strlen(argv[i]); j++) {
	      include_dirs[num_includes][j-2] = argv[i][j];
      }

      include_dirs[num_includes][strlen(argv[i])-2] = 0;
      num_includes++;
    }
    // Stage 1 only
    else if (strcmp(argv[i], "-s1") == 0) {
      stage = 1;
    }
    // Stage 2 only
    else if (strcmp(argv[i], "-s2") == 0) {
      stage = 2;
    }
  }

  int ret = 0;
  if (stage > 3 || stage == 1) {
    ret = stage1(file_name, output_name, include_dirs, num_includes);
    if (ret) quit(ret);
    strcpy(file_name, output_name);
  }
  if (stage > 3 || stage == 2) {
    ret = stage2(file_name, output_name, output_type_2);
    if (ret) quit(ret);
  }
  quit(ret);
  return ret;
}


