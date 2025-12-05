#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stage1.h>
#include <stage2.h>

/**
  Main function, reads arguments and runs the program
*/
int main(int argc, char * argv[]) {

  // Input file
  char * file_name = malloc(256);
  strcpy(file_name, "./input.zr");

  // Output file
  char * output_name = malloc(256);
  strcpy(output_name, "./a.out");
  
  // include paths
  char ** include_dirs = malloc(sizeof(char*) * 32);
  int num_includes = 0;

  // read arguments
  int current_arg = 1;
  int stage = 5;
  for (int i = 1; i < argc; i++) {

    // Input file
    if (strcmp(argv[i], "-f") == 0) {
      i++;
      strcpy(file_name, argv[i]);
    }
    else if (strcmp(argv[i], "-o") == 0) {
      i++;
      strcpy(output_name, argv[i]);
    }
    // Include paths
    else if (argv[i][0] == '-' && argv[i][1] == 'I') {
      include_dirs[num_includes] = malloc(64);

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
    else if (strcmp(argv[i], "-s2") == 0) {
      stage = 2;
    }
  }

  int ret = 0;
  if (stage > 3 || stage == 1) {
    ret = stage1(file_name, output_name, include_dirs, num_includes);
    if (ret) goto exit;
    strcpy(file_name, output_name);
  }
  if (stage > 3 || stage == 2) {
    ret = stage2(file_name, output_name);
    if (ret) goto exit;
  }
 exit:
  for (int i = 0; i < num_includes; i++) {
    free(include_dirs[i]);
  }

  free(include_dirs);
  free(file_name);
  free(output_name);
  return ret;
} 
