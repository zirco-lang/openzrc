#pragma once

/**
  Skips white space
  buffer: buffer
  start: current index
  return: amount to move head by
*/
int skip_white(char * buffer, int start);

/**
  Returns the index of the end of the next token
  buffer: input buffer
  start: current index
  return: amount
*/
int next_tok(char * buffer, int start);

/**
 * Returns 1 if character is in range
 * first - first character to check
 * max - last character to checl
 * c - character to check against
 * return 1 if in range, 0 otherwise
 */
int in_range(char first, char max, char c);

/**
 * Returns 1 if characte is a decimal digit
 * c - character
 * return - 1 if in range, 0 otherwise
 */
int is_digit(char c);

/**
 * Returns 1 if character is a letter
 * c - character
 * return - 1 if in range, 0 otherwise
 */
int is_alpha(char c);

/**
 * returns 1 if char is alphanumeric
 * c - character to check
 * return - 1 if in range, 0 otherwise
 */
int is_alpha_num(char c);

/**
 * Allocates a substring from a buffer
 * data_len - length of substring
 * idx - start index
 * buffer - buffer to copy from
 * return - pointer to substring
 */
void * alloc_substring(int data_len, char * buffer, int idx);

/**
 * Allocates some space on the heap to use
 * size - size of usage
 */
void* add_alloc(int size);

/**
 * quits the program.
 * code - exit code.
 */
void quit(int code);