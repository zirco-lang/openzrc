#include "parser.h"
#include "util.h"

#include <stdlib.h>

/*
 * Parse type definitions
 */
int parse_type(TOKEN ** tokens, int alloc_tokens, int idx) {
  switch (tokens[0][idx].tok) {
  case TOK_I32:
    return TOK_I32;
    break;
    //TODO: more types
  default:
    return -1;
  }
}

int is_binop(int tok) {
  switch (tok) {
  case TOK_PLUS:
  case TOK_MINUS:
    return 1;
    break;
  default:
    return 0;
    break;
  }
}
/*
 * Parse an expression
 */
int parse_expr(TOKEN ** tokens, int alloc_tokens, GRAMMAR_T * out, int idx) {
  if (idx+1 < alloc_tokens && is_binop(tokens[0][idx+1].tok)) {
    BINARY_OP * binary = malloc(sizeof(BINARY_OP));
    GRAMMAR_T * lhs = malloc(sizeof(GRAMMAR_T));
    GRAMMAR_T * rhs = malloc(sizeof(GRAMMAR_T));
    
    lhs->typ = PARSER_TOKEN;
    lhs->val = (void*)(*tokens+idx);

    rhs->typ = PARSER_TOKEN;
    rhs->val = (void*)(*tokens+idx+2);

    binary->lhs = lhs;
    binary->rhs = rhs;
    binary->typ = (*tokens+idx+1);

    out->typ = PARSER_BINARY_OP;
    out->val = (void*)binary;
    return 3;
  }
  
  out->typ = PARSER_TOKEN;
  out->val = (void*)(*tokens+idx);
  
  return 1;
}

/*
  Parse a `let` declaration
 */
int parse_let_decl(TOKEN ** tokens, int alloc_tokens, PARSE_DECL * out, int idx) {

  // child value is unknown until needed
  GRAMMAR_T * val = malloc(sizeof(GRAMMAR_T));
  val->typ = PARSER_UNKNOWN;

  // check for valid definition
  if (tokens[0][idx+1].tok != TOK_IDENTIFIER ||
      tokens[0][idx+2].tok != TOK_COLON ||
      parse_type(tokens, alloc_tokens, idx+3) == -1
      ) {
    return -1;
  }

  // initialize output
  out->identifier = (*tokens)+idx+1;
  out->typ = (*tokens)+idx+3;
  int amt = 4;

  // if there is an equal sign, we are also defining. check for an expression afterwards.
  if (tokens[0][idx+4].tok == TOK_EQ) {
    amt += parse_expr(tokens, alloc_tokens, val, idx+5);
  }
  
  out->val = val;
  return amt;
}

/*
 * Parse a statement
 */
int parse_stmt(TOKEN ** tokens, int alloc_tokens, GRAMMAR_T * out, int idx) {
  TOKEN current = tokens[0][idx];
  // an empty semicolon is a valid statement
  if (current.tok == TOK_SEMICOLON) {
    out->typ = PARSER_TOKEN;
    out->val = (void*)((*tokens)+idx);
    return 1;
  } if (current.tok == TOK_OPEN_CURLY) {
    out->typ = PARSER_LIST;
    GRAMMAR_LIST * list = 0;
    GRAMMAR_LIST * current_item;
    int count = 1;
    while ((tokens[0][idx + count]).tok != TOK_CLOSE_CURLY && idx + count < alloc_tokens) {
      if (list == 0) {
	list = malloc(sizeof(GRAMMAR_LIST));
	current_item = list;
      } else {
	current_item->next = malloc(sizeof(GRAMMAR_LIST));
	current_item = current_item->next;
      }
      current_item->val = malloc(sizeof(GRAMMAR_T));
      current_item->next = 0;
      count += parse_stmt(tokens, alloc_tokens, current_item->val, idx+count);
    }
    count++;
    out->val = (void*)list;
    return count;
  }
  // check for a let identifier
  if (current.tok == TOK_LET) {
    out->typ = PARSER_LET_DECL;
    PARSE_DECL * val = malloc(sizeof(PARSE_DECL));
    out->val = (void *) val;
    return parse_let_decl(tokens, alloc_tokens, val, idx) + 1;
  }
  if (current.tok == TOK_RETURN) {
    out->typ = PARSER_RETURN;
    out->val = 0;
    GRAMMAR_T val;
    val.typ = PARSER_TOKEN;
    val.val = 0;
    int len = 1;
    if (tokens[0][idx+1].tok != TOK_SEMICOLON) {
      out->val = malloc(sizeof(GRAMMAR_T));
      len += parse_expr(tokens, alloc_tokens, (GRAMMAR_T*)(out->val), idx+1);
      if (tokens[0][idx+len].tok != TOK_SEMICOLON) {
	quit(1, "missing semicolon at end of return");
      }
      len++;
    }
    return len;
    
    
  }
  // in this case we don't know what we just hit, so check it.
  out->typ = PARSER_UNKNOWN;
  return 1;
  
}

int free_parser(GRAMMAR_T * out);

void free_parser_list(GRAMMAR_LIST * list) {
  if (list == 0) return;
  GRAMMAR_LIST * prev = 0;
  GRAMMAR_LIST * current = list;

  while (list->next != 0) {
    current = list;
    while (current->next != 0) {
      prev = current;
      current = current->next;
    }
    free_parser(current->val);
    free(current);
    prev->next = 0;
  }
  free_parser(list->val);
  free(list);
}
/*
 * This frees the memory allocated by the parser
 */
int free_parser(GRAMMAR_T * out) {
  if (out == 0) return 0;
  switch (out->typ) {
    // statements always have a value
  case PARSER_STMT:
  case PARSER_EXPR:
  case PARSER_RETURN:
    {
      if (out->val != 0) free_parser((GRAMMAR_T*)(out->val));
    }
    break;
    // decls have their own objects
  case PARSER_LET_DECL:
  case PARSER_FN:
    {
      PARSE_DECL * decl = (PARSE_DECL*)(out->val);
      free_parser((GRAMMAR_T*)(decl->val));
      free(decl);
    }
    break;
    // same with unary operators
  case PARSER_BINARY_OP:
    {
      BINARY_OP * decl = (BINARY_OP*)(out->val);
      free_parser((GRAMMAR_T*)(decl->rhs));
      free_parser((GRAMMAR_T*)(decl->lhs));
      free(decl);
    }
    break;
  case PARSER_LIST:
    {
      free_parser_list((GRAMMAR_LIST*)(out->val));      
    }
    break;
  }

  // this actually frees the current object
  free(out);
  return 0;
}


/**
 * Parses a function
 */
int parse_fn(TOKEN ** tokens, int alloc_tokens, GRAMMAR_T * out, int idx) {
  // check for fn decl
  if (tokens[0][idx].tok != TOK_FN) {
    return -1;
  }
  // parse identifier
  PARSE_DECL * outd = malloc(sizeof(PARSE_DECL));
  outd->identifier = &tokens[0][idx+1];

  // parse open paren
  if (tokens[0][idx+2].tok != TOK_OPEN_PAREN) {
    return -1;
  }

  // parse args
  int idx2 = 3;
  // todo: fn args

  // check for type def
  if (tokens[0][idx+idx2].tok != TOK_CLOSE_PAREN ||
      tokens[0][idx+idx2+1].tok != TOK_MINUS ||
      tokens[0][idx+idx2+2].tok != TOK_GT) {
    return -1;
  }

  // check for valid type
  if (parse_type(tokens, alloc_tokens, idx+idx2+3) == -1) {
    return -1;
  }

  // set up output
  outd->typ = &tokens[0][idx+idx2+3];
  outd->val = malloc(sizeof(GRAMMAR_T));
  outd->val->typ = PARSER_UNKNOWN;

  // parse inside of function
  int tmp = parse_stmt(tokens, alloc_tokens, outd->val, idx+idx2+4);

  // output data and return length
  out->typ = PARSER_FN;
  out->val = (void*) outd;
  return idx2+4+tmp;
}

/*
 * This is for early development, and prints a parse tree
 */
void print_tree(GRAMMAR_T * out) {
  switch (out->typ) {
  case PARSER_STMT:
    printf("Begin Statement\n");
    print_tree((GRAMMAR_T*)(out->val));
    printf("End Statement\n");
    break;
  case PARSER_EXPR:
    printf("Begin EXPR\n");
    print_tree((GRAMMAR_T*)(out->val));
    printf("End EXPR\n");
    break;
  case PARSER_LET_DECL:
    {
      printf("Begin Let Decl:\n");
      PARSE_DECL * val = (PARSE_DECL*)(out->val);
      printf("  identifier: %s\n", val->identifier->val);
      printf("  typ: %s\n", val->typ->val);
      print_tree(val->val);
      printf("End let decl\n");
    }
    break;
  case PARSER_BINARY_OP:
    {
      BINARY_OP * val = (BINARY_OP*)(out->val);
      printf("Begin Binary OP\n");
      printf("typ: \n  ");
      print_token(val->typ->tok, &val->typ->val);
      printf("begin lhs\n");
      print_tree(val->lhs);
      printf("end lhs\n");
      printf("begin rhs\n");
      print_tree(val->rhs);
      printf("end rhs\n");
      printf("End Binary op\n");
    }
    break;
  case PARSER_TOKEN:
    {
      TOKEN * tok = (TOKEN*)(out->val);
      printf("Token: \n  ");
      print_token(tok->tok, &tok->val);
    }
    break;
  case PARSER_LIST:
    {
      printf("Begin parser list\n");
      GRAMMAR_LIST * l = (GRAMMAR_LIST *)out->val;
      do {
	if (l->val != 0) {
	  print_tree(l->val);
	}
      } while ((l = l->next) != 0);
      printf("end parser list\n");
    }
    break;
  case PARSER_RETURN:
    {
    printf("begin parser return\n");
    if (out->val != 0) {
      print_tree(((GRAMMAR_T *)(out->val)));
    }
    printf("end parser return\n");
    }
    break;
  case PARSER_FN:
    {
      printf("begin parser fn\n");
      PARSE_DECL * val = (PARSE_DECL*)(out->val);
      printf("  identifier: %s\n", val->identifier->val);
      printf("  typ: %s\n", val->typ->val);
      print_tree(val->val);
      printf("end parser fn\n");
    }
    break;
  }
}

/*
 * This is the main parser
 */
int parser(TOKEN ** tokens, int alloc_tokens, GRAMMAR_T * out) {
  // since we aren't ready for a full implementation, we are just parsing a statement.
  out->typ = PARSER_STMT;
  int idx = 0;
  GRAMMAR_T * val = malloc(sizeof(GRAMMAR_T));
  out->val = (void*) val;
  int tmp = parse_stmt(tokens, alloc_tokens, val, idx);
  if (tmp < 0) {
    return tmp;
  }
  idx += tmp;
  return idx;
}
