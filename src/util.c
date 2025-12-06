#include <util.h>

#include <stdlib.h>

int skip_white(char * buffer, int start) {
  int out = 0;
  while (
	 buffer[start + out] == ' ' ||
	 buffer[start + out] == '\t' ||
	 buffer[start + out] == '\n' ||
	 buffer[start + out] == '\r') {
    out++;
  }
  return out;
}

int next_tok(char * buffer, int start) {
  int out = 0;
  while (buffer[start + out] != ' ' &&
	 buffer[start + out] != '\t' &&
	 buffer[start + out] != '\n' &&
	 buffer[start + out] != 0 &&
	 buffer[start + out] != '\r') {
    out++;
  }
  return out;
}

int in_range(char first, char max, char c) {
  for (char i = first; i <= max; i++) {
    if ((i) == c) return 1;
  }
  return 0;
}

int is_digit(char c) {
  return in_range('0', '9', c);
}

int is_alpha(char c) {
  return in_range('a', 'z', c) || in_range('A', 'Z', c);
}

int is_alpha_num(char c) {
  return is_alpha(c) || is_digit(c);
}

void * allocs[1024*16];
int num_allocs = 0;

void* add_alloc(int size) {
  void * ret = malloc(size);
  allocs[num_allocs] = ret;
  num_allocs++;
  return ret;
}

void quit(int code) {
  for (int i = 0; i < num_allocs; i++) {
    free(allocs[i]);
  }
  exit(code);
}

void * alloc_substring(int data_len, char * buffer, int idx) {
  void * data = add_alloc(data_len+1);
  for (int i = 0; i < data_len; i++) {
    ((char*) data)[i] = buffer[idx + i];
  }
  ((char*)data)[data_len] = 0;
  return data;
}