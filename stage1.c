#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


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

  fclose(fptr);
  return 0;
}
