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
