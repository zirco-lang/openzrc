#include <lexer.h>
#include <util.h>
#include <string.h>
#include <stdlib.h>

/**
 * Returns length of integer literal, or 0.
 * *c - integer literal string
 * return - length or 0
 */
int is_int_lit(char *c) {
  int len = strlen(c);
  if (is_digit(c[0])) {
    int current = 1;
    while (current < len && is_digit(c[current])) {
      current++;
    }
    return current;
  }
  return 0;
}

/**
 * checks for a string
 * *c - buffer to check for string
 * return - length of string, or 0
 */
int is_string(char *c) {
  int len = strlen(c);
  if (len > 0 && c[0] == '"') {
    int current = 1;
    while (current < len && (c[current] != '"' || c[current-1] == '\\')) {
      current++;
    }
    if (c[current] == '"') {
      current++;
    } else {return 0;}
    return current;
  }
  return 0;
}

/**
 * Checks for a label
 * *c - label string to check
 * return - length of label or 0
 */
int is_label(char *c) {
  int len = strlen(c);
  
  if (len > 0 && (is_alpha(c[0]) || c[0] == '_')) {
    int current = 1;

    while (current < len && (is_alpha_num(c[current]) || c[current] == '_')) {
      current++;
    }
    return current;
  }
  return 0;
}

void print_token(FILE * out, token token) {
  fprintf(out, "token[type=%s", lexer_type_names[token.type]);
  if (token.data_len > 0) {
    if (token.type == LT_LABEL || token.type == LT_INT_LITERAL || token.type == LT_STRING_LITERAL) {
      fprintf(out, " data={%s}", (char*) token.data);
    }
    fprintf(out, ", data_len=%d", token.data_len);
  }
  fprintf(out, "]");
}

void print_tokens(FILE* out, token * tokens, int * num_tokens) {
  for (int i = 0; i < *num_tokens; i++) {
    print_token(out, tokens[i]);
    fprintf(out, "\n");
  }
}

void tokenize(token * tokens, int * num_tokens, char * buffer) {
  int idx = 0;
  while (buffer[idx] != EOF && idx < strlen(buffer)) {
    
    lexer_type t = LT_UNKNOWN;
    void* data = 0;
    int data_len = 0;
    if ((data_len = is_label(buffer + idx))) {
      data = alloc_substring(data_len,buffer, idx);
      t = LT_LABEL;
    } else if ((data_len = is_string(buffer + idx))) {
      data = alloc_substring(data_len, buffer, idx);
      t = LT_STRING_LITERAL;
    } else if ((data_len = is_int_lit(buffer + idx))) {
      data = alloc_substring(data_len, buffer, idx);
      t = LT_INT_LITERAL;
    }
    else {
      data_len = 1;
      switch (buffer[idx]) {
        case ';':
          t = LT_SEMICOLON;
          break;
        case ':':
          t = LT_COLON;
          break;
        case '(':
          t = LT_OPEN_PAREN;
          break;
        case ')':
          t = LT_CLOSE_PAREN;
          break;
        case '[':
          t = LT_OPEN_BRACKET;
          break;
        case ']':
          t = LT_CLOSE_BRACKET;
          break; 
        case '{':
          t = LT_OPEN_CURLY;
          break;
        case '}':
          t = LT_CLOSE_CURLY;
          break;
        case '*':
          t = LT_ASTERISK;
          break;
        case '=':
          t = LT_EQUALS;
          break;
        case ',':
          t = LT_COMMA;
          break;
        case '-':
            t = LT_HYP;
            break;
        case '>':
            t = LT_GREATER;
            break;
        case '<':
            t = LT_LESSER;
            break;
        case '.':
            t = LT_DOT;
            break;
        case '\n':
        case ' ':
        case '\t':
        case '\r':
          break;
      }
    }
    if (t != LT_UNKNOWN) {
      tokens[*num_tokens].type = t;
      tokens[*num_tokens].data = data;
      tokens[*num_tokens].data_len = data_len;
      (*num_tokens)++;
    }
    idx += data_len;
  }
}
