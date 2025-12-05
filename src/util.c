#include <util.h>

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
