#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>


/**
 * Creates a substring from a root buffer into an output buffer from start to end
 *
 * in_buf - input buffer
 * out_buf - Output buffer
 * start - start index
 * end - end index
 */
void substring(char * in_buf, char * out_buf, int start, int end) {
  for (int i = start; i < end; i++) {
    out_buf[i-start] = in_buf[i];
  }
  out_buf[end-start] = 0;
}

// forward decl
void handle_include(char * name, char ** path, int path_size, FILE * out_file);

/**
 * Main Stage 1 handler
 *
 * out_file - Output file
 * buffer - input buffer
 * buffer_size - size of input buffer
 * path - include paths
 * path_size - number of include paths
 * return - status
 */
int stage1_inner(FILE* out_file, char * buffer, int buffer_size, char ** path, int path_size) {

  // current read index
  int counter = 0;
  while (counter < buffer_size) {

    // read the next token
    counter += skip_white(buffer, counter);
    int end = counter + next_tok(buffer, counter);
    char * token = malloc(64);
    substring(buffer, token, counter, end);
    counter = end;

    // handle includes
    if (strcmp(token, "#include") == 0) {
      // read file path
      counter += skip_white(buffer, counter);
      int end2 = counter + next_tok(buffer, counter);
      char * include = malloc(64);
      substring(buffer, include, counter+1, end2-1);
      
      handle_include(include, path, path_size, out_file);

      counter = end2;
      free(include);

    }
    // write any other tokens
    else {
      fprintf(out_file, "%s ", token);
      if (token[strlen(token)-1] == ';') {
	fprintf(out_file, "\n");
      }
    }
    
    free(token);
  }
  return 0;
}


/**
 * Handles a main include path
 *
 * name - name of file
 * path - include path
 * path_size - size of include paths
 * out_file - output file
 */
void handle_include(char * name, char ** path, int path_size, FILE * out_file) {
  // iterate through include paths
  for (int i = 0; i < path_size; i++) {
    // load current path
    char * current_path = malloc(512);
    strcpy(current_path, path[i]);
    strcat(current_path, "/");
    strcat(current_path, name);

    // try to read a possible include file
    FILE * input = fopen(current_path, "r");
    if (input == 0) {
      free(current_path);
      continue;
    }

    // if it's valid, include it into the output file.
    char * in_buf = malloc(1024*16);
    int amt = fread((void*) in_buf, sizeof(char), 1024*16, input);
    fclose(input);
    
    int out = stage1_inner(out_file, in_buf, amt, path, path_size);

    // Clean up
    free(in_buf);
    free(current_path);
    break;
  }
}

int stage1(char * file_name, char * out_file_name, char** path, int path_size) {

  // load files
  FILE * in_file = fopen(file_name, "r");
  FILE * out_file = fopen(out_file_name, "w");

  // read the input file
  char * in_buf = malloc(1024*16);
  int amt = fread((void*) in_buf, sizeof(char), 1024*16, in_file);
  fclose(in_file);

  // handle stage 1
  int out = stage1_inner(out_file, in_buf, amt, path, path_size);

  // clean up
  free(in_buf);
  fclose(out_file);
  return out;
}
