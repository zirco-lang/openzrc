
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// forward declare quit
void quit(int code, char * reason);

//===================================
// Begin Lexer
//
//==================================

/**
 List of supported tokens
*/
enum ZR_TOK {
  TOK_EOF = 0,
  TOK_LET,
  TOK_COLON,
  TOK_SEMICOLON,
  TOK_I32,
  TOK_IDENTIFIER,
  TOK_LITERAL,
  TOK_EQ,
  TOK_PLUS,
  TOK_OPEN_CURLY,
  TOK_CLOSE_CURLY,
  TOK_RETURN,
  TOK_UNKNOWN = -1,
};

/*
 * Checks single-character tokens
 */
int check_char_toks(int ch) {
  unsigned char ch2 = (unsigned char) ch;
  if (ch == EOF) return TOK_EOF;
  switch (ch2) {
  case ':':
    return TOK_COLON;
    break;
  case ';':
    return TOK_SEMICOLON;
    break;
  case '=':
    return TOK_EQ;
    break;
  case '+':
    return TOK_PLUS;
    break;
  case '{':
    return TOK_OPEN_CURLY;
    break;
  case '}':
    return TOK_CLOSE_CURLY;
  default:
    return TOK_UNKNOWN;
  }
  return TOK_UNKNOWN;
}

/*
 * Gathers the next token
 */
  
int next_tok(FILE * fptr, char ** out) {
  // Create buffer for temporary storage
  char buf[255];
  int idx = 0;
  int ch = fgetc(fptr);
  char ch2 = (char) ch;

  // Remove any whitespace before next token
  while (ch2 == ' ' || ch2 == '\t' || ch2 == '\n') {
    ch = fgetc(fptr);
    ch2 = (char) ch;
  }

  // Check for single-character values
  int val = check_char_toks(ch);
  if (val != TOK_UNKNOWN) return val;

  // Check for alphabetical
  while ((ch2 >= 'A' && ch2 <= 'Z') || (ch2 >= 'a' && ch2 <= 'z')) {
    buf[idx] = ch2;
    idx++;
    ch = fgetc(fptr);
    ch2 = (char) ch;
  } 

  // check for keywords (only let is found here for now...)
  buf[idx] = 0;
  if (strcmp(buf, "let") == 0) return TOK_LET;
  if (strcmp(buf, "return") == 0) return TOK_RETURN;
  // this is a case to find things that start with numbers
  int start = 0;
  if (idx > 0) start = 1;

  // check for numbers
  while (ch2 >= '0' && ch2 <= '9') {
    buf[idx] = ch2;
    idx++;
    ch = fgetc(fptr);
    ch2 = (char) ch;
  }

  // go back a step as we sometimes go past where we need to when checking
  buf[idx] = 0;
  if (idx > 0) fseek(fptr, -1, SEEK_CUR);

  // set output buffer
  strcpy(*out, buf);
  // check for type definitions (only i32 for now...)
  if (strcmp(buf, "i32") == 0) return TOK_I32;

  // this is either an identifier or a literal of some sort...
  if (idx > 0) {
    // if it starts with a number, it's always a literal.
    if (start == 0) {
      return TOK_LITERAL;
    }
    return TOK_IDENTIFIER;
  }

  // write our single-character unknowns here...
  snprintf(*out, 2, "%c", ch2);
  return TOK_UNKNOWN;
}

/*
 * Prints a token nicely...
 */
void print_token(int tok, char** buf) {
  switch (tok) {
  case TOK_EOF:
    printf("EOF\n");
    break;
  case TOK_COLON:
    printf(":\n");
    break;
  case TOK_LET:
    printf("let\n");
    break;
  case TOK_SEMICOLON:
    printf(";\n");
    break;
  case TOK_I32:
    printf("i32\n");
    break;
  case TOK_IDENTIFIER:
    printf("identifier: %s\n", *buf);
    break;
  case TOK_LITERAL:
    printf("literal: %s\n", *buf);
    break;
  case TOK_EQ:
    printf("=\n");
    break;
  case TOK_PLUS:
    printf("+\n");
    break;
  case TOK_OPEN_CURLY:
    printf("{\n");
    break;
  case TOK_CLOSE_CURLY:
    printf("}\n");
    break;
  case TOK_RETURN:
    printf("return\n");
    break;
  case TOK_UNKNOWN:
    printf("Unknown value: %s\n", *buf);
    break;
  default:
    {
    char error[128];
    snprintf(error, 128, "Unimplemented token: %s\n", *buf);
    quit(1, error);
    }
    break;
  }
}

/*
 * Token Structure for parsing
 */
typedef struct {
  int tok;
  char * val;
} TOKEN;


int lexer(FILE * fptr, TOKEN ** tokens, int tok_len) {
  int val;
  //allocate first token
  int i = 0;
  (tokens[0][i].val) = malloc(255);

  // allocate rest of tokens that are found
  while ((val = next_tok(fptr, &(tokens[0][i].val))) != 0 && i < tok_len) {
    tokens[0][i].tok = val;
    i++;
    (tokens[0][i].val) = malloc(255);

  }

  // output last token value
  tokens[0][i].tok = val;

  // this is the final size...
  i++;
  return i;
}

//====================
// PARSER
//
//====================

/*
 * Types of values in a parser
 */
enum parser_type {
  PARSER_STMT = 0,
  PARSER_EXPR,
  PARSER_LET_DECL,
  PARSER_RETURN,
  PARSER_UNARY_OP,
  PARSER_TOKEN,
  PARSER_LIST,
  PARSER_UNKNOWN = -1,
};

/*
 * Grammar items
 */
typedef struct {
  int typ;
  void * val;
} GRAMMAR_T;

/*
 * Linked List of Grammar_ts
 */
typedef struct grammar_list {
  GRAMMAR_T * val;
  struct grammar_list * next;  
} GRAMMAR_LIST;

/*
 * Special case for `let` statements
 */
typedef struct {
  TOKEN * identifier;
  TOKEN * typ;
  GRAMMAR_T * val;
} LET_DECL;

/*
 * Case for unary operators
 */
typedef struct {
  GRAMMAR_T * lhs;
  GRAMMAR_T * rhs;
  TOKEN * typ;
} UNARY_OP;

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

/*
 * Parse an expression
 */
int parse_expr(TOKEN ** tokens, int alloc_tokens, GRAMMAR_T * out, int idx) {
  if (idx+1 < alloc_tokens && tokens[0][idx+1].tok == TOK_PLUS) {
    UNARY_OP * unary = malloc(sizeof(UNARY_OP));
    GRAMMAR_T * lhs = malloc(sizeof(GRAMMAR_T));
    GRAMMAR_T * rhs = malloc(sizeof(GRAMMAR_T));
    
    lhs->typ = PARSER_TOKEN;
    lhs->val = (void*)(*tokens+idx);

    rhs->typ = PARSER_TOKEN;
    rhs->val = (void*)(*tokens+idx+2);

    unary->lhs = lhs;
    unary->rhs = rhs;
    unary->typ = (*tokens+idx+1);

    out->typ = PARSER_UNARY_OP;
    out->val = (void*)unary;
    return 3;
  }
  
  out->typ = PARSER_TOKEN;
  out->val = (void*)(*tokens+idx);
  
  return 1;
}

/*
  Parse a `let` declaration
 */
int parse_let_decl(TOKEN ** tokens, int alloc_tokens, LET_DECL * out, int idx) {

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
    LET_DECL * val = malloc(sizeof(LET_DECL));
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
    {
      free_parser((GRAMMAR_T*)(out->val));
    }
    break;
    // let decls have their own objects
  case PARSER_LET_DECL:
    {
      LET_DECL * decl = (LET_DECL*)(out->val);
      free_parser((GRAMMAR_T*)(decl->val));
      free(decl);
    }
    break;
    // same with unary operators
  case PARSER_UNARY_OP:
    {
      UNARY_OP * decl = (UNARY_OP*)(out->val);
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
      LET_DECL * val = (LET_DECL*)(out->val);
      printf("  identifier: %s\n", val->identifier->val);
      printf("  typ: %s\n", val->typ->val);
      print_tree(val->val);
      printf("End let decl\n");
    }
    break;
  case PARSER_UNARY_OP:
    {
      UNARY_OP * val = (UNARY_OP*)(out->val);
      printf("Begin Unary OP\n");
      printf("typ: \n  ");
      print_token(val->typ->tok, &val->typ->val);
      printf("begin lhs\n");
      print_tree(val->lhs);
      printf("end lhs\n");
      printf("begin rhs\n");
      print_tree(val->rhs);
      printf("end rhs\n");
      printf("End unary op\n");
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

//===================
// Code Generation
//===================


/*
 * Value list, stores a reference value for variables in a linked list
 */
typedef struct value_list {
  char name[64];
  LLVMValueRef val;
  LLVMTypeRef typ;
  struct value_list * next;
} VALUE_LIST;

/*
 * Adds a reference value to the linked list.
 */
void add_value(char * name, LLVMValueRef val, LLVMTypeRef typ, VALUE_LIST ** vl) {
  VALUE_LIST * current;
  if (*vl == 0) {
    *vl = malloc(sizeof(VALUE_LIST));
    current = *vl;
  } else {
    current = *vl;
    while (current->next != 0) {
      current = current->next;
    }

    current->next = malloc(sizeof(VALUE_LIST));
    current = current->next;
  }
  current->next = 0;
  strcpy(current->name, name);
  memcpy(&current->typ, &typ, sizeof(LLVMTypeRef));
  memcpy(&current->val, &val, sizeof(LLVMValueRef));
}

/*
 * Frees a value linked list
 */
void free_list(VALUE_LIST** vl) {
  if (*vl == 0) return;
  VALUE_LIST * current = *vl;
  VALUE_LIST * prev = 0;
  while ((*vl)->next != 0) {
    current = *vl;
    prev = 0;
    while (current->next != 0) {
      prev = current;
      current = current->next;
    }
    free(current);
    if (prev != 0) prev->next = 0;
  }
  free(*vl);
  
}

/*
 * Gets a value from the value linked list.
 */
int get_value(char * name, VALUE_LIST ** vl, LLVMValueRef * out, LLVMTypeRef * typ) {
  VALUE_LIST * current = *vl;
  while (current != 0 && (strcmp(current->name, name) != 0)) {
    current = current->next;
  }

  if (current == 0) {
    return 0;
  }
  if (strcmp(current->name, name) == 0) {
     *out = current->val;
     *typ = current->typ;
  }
  return 0;
}


/*
 * Gets a LLVMTypeRef from a token
 */
void get_type_ref(LLVMTypeRef * ref, int tok) {
  switch (tok) {
    
  case TOK_I32:
  default:
    *ref = LLVMInt32Type();
    break;
  }
}


/*
 * Generates an expression.
 */
int gen_expr(GRAMMAR_T * parse_tree, LLVMModuleRef* mod, LLVMBuilderRef * builder, VALUE_LIST** vl, LLVMValueRef * out, int i) {
  switch (parse_tree->typ) {
  case PARSER_TOKEN:
    {
      TOKEN * tok = (TOKEN*)parse_tree->val;
      switch (tok->tok) {
      case TOK_LITERAL:
	{
	  *out = LLVMConstInt(LLVMInt32Type(), atoi(tok->val), 0);
	}
	break;
      case TOK_IDENTIFIER:
	{
	  LLVMTypeRef typ;
	  LLVMValueRef ptr;
	  get_value(tok->val, vl, &ptr, &typ);
	  char name[64];
	  snprintf(name, 64, "__%s_ld", tok->val);
	  *out = LLVMBuildLoad2(*builder, typ, ptr, name);
	}
	break;

      }
      
    }
    break;
  case PARSER_UNARY_OP:
    {
      UNARY_OP * uop = (UNARY_OP*) parse_tree->val;
      LLVMValueRef lhs;
      LLVMValueRef rhs;
      gen_expr(uop->lhs, mod, builder, vl, &lhs, i+1);
      gen_expr(uop->rhs, mod, builder, vl, &rhs, i+1);

      TOKEN * typ = uop->typ;

      switch (typ->tok) {
      case TOK_PLUS:
	{
	  char name[32];
	  snprintf(name, 32, "_add_%d", i);
	  *out = LLVMBuildAdd(*builder, lhs, rhs, name);
	}
	break;
      }
      
    }
    break;
  }
  return 0;
}

/*
 * Generates a statement
 */
int gen_stmt(GRAMMAR_T * parse_tree, LLVMModuleRef* mod, LLVMBuilderRef * builder, LLVMValueRef * fn, VALUE_LIST** vl, int idx) {
  
  switch (parse_tree->typ) {
  case PARSER_LET_DECL:
    {
      LET_DECL * decl = (LET_DECL*)parse_tree->val;
      char * name = decl->identifier->val;
      int typ = decl->typ->tok;
      LLVMTypeRef var_typ;
      get_type_ref(&var_typ, typ);

      add_value(name, LLVMBuildAlloca(*builder, var_typ, name), var_typ, vl);
      if (decl->val != 0) {
	LLVMValueRef ref;
	LLVMValueRef get;
	LLVMTypeRef typ;
	gen_expr(decl->val, mod, builder, vl, &ref, 0);
	get_value(name, vl, &get, &typ);
	LLVMBuildStore(*builder, ref, get);
      }   
    }
    break;
  case PARSER_LIST: {
    GRAMMAR_LIST * l = (GRAMMAR_LIST *) parse_tree->val;
    GRAMMAR_LIST * current = l;
    while (current != 0) {
      gen_stmt(current->val, mod, builder, fn, vl, ++idx);
      current = current->next;
    }
  }
    break;
  case PARSER_RETURN:
    {
      LLVMValueRef ret_val;
      if (parse_tree->val == 0) {
	ret_val = LLVMConstInt(LLVMInt32Type(), 0, 0);
      } else {
	GRAMMAR_T * val = (GRAMMAR_T *) parse_tree->val;
	gen_expr(val, mod, builder, vl, &ret_val, 0);
	
      }
      LLVMBuildRet(*builder, ret_val);
    }
    break;
  }
  return 0;
}

/*
 * This is for early development, where no function definitions exist. This generates a test function.
 */
int test_gen_fn(GRAMMAR_T * parse_tree, LLVMModuleRef* mod, LLVMBuilderRef * builder, VALUE_LIST ** vl) {

  // No parameters, returns an integer
  LLVMTypeRef param_types[] = {};
  LLVMTypeRef ret_type = LLVMFunctionType(LLVMInt32Type(), param_types, 0, 0);

  // Main function name
  LLVMValueRef main = LLVMAddFunction(*mod, "main", ret_type);
  // Entry point
  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main, "entry");
  LLVMPositionBuilderAtEnd(*builder, entry);
  
  // Generates a single statement from the parse tree
  gen_stmt((GRAMMAR_T*)parse_tree->val, mod, builder, &main, vl, 0);
  
  return 0;
}


/*
 * Generates the final file.
 */
int gen_code(char * input_name, char * output_name, GRAMMAR_T * parse_tree) {

  // Create module and builder
  LLVMModuleRef mod = LLVMModuleCreateWithName(input_name);
  LLVMBuilderRef builder = LLVMCreateBuilder();

  // Value linked list
  VALUE_LIST * vl = 0;


  // TODO: do real function generation, for now test function
  test_gen_fn(parse_tree, &mod, &builder, &vl);
  

  // Compile
  char *error = NULL;
  LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
  LLVMDisposeMessage(error);
  
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();

  if (LLVMWriteBitcodeToFile(mod, output_name) != 0) {
    fprintf(stderr, "error writing bitcode to file, skipping\n");
  }

  // Clean up
  LLVMDisposeBuilder(builder);
  free_list(&vl);
  
  return 0;
}

//====================
// Main Program
//====================

 GRAMMAR_T* parse_tree_main = 0;
 TOKEN * tokens_main = 0;
 int alloc_tokens_main;
 int num_tokens_main = 0;

/*
 * exit the program
 */
void quit(int code, char * reason) {
   if (code != 0) {
     printf("Program crashed! Reason: %s\n", reason);
   }
   
   if (parse_tree_main != 0) free_parser(parse_tree_main);
   if (tokens_main != 0) {
     for (int i = 0; i < num_tokens_main; i++) {
       free(tokens_main[i].val);
     }
     free(tokens_main);
   }
   
   exit(code);
 }
 
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

  // allocate a bunch of tokens to read. this number is low for testing.
  alloc_tokens_main = 32;
  tokens_main = malloc(sizeof(TOKEN) * alloc_tokens_main);

  // run the lexer and stop reading from the file.
  num_tokens_main = lexer(fptr, &tokens_main, alloc_tokens_main);
  fclose(fptr);

  // set up the parser
  parse_tree_main = malloc(sizeof(GRAMMAR_T));
  (*parse_tree_main).typ = PARSER_UNKNOWN;

  // run the parser
  int parsed = parser(&tokens_main, alloc_tokens_main, parse_tree_main);
  if (parsed < 0) {
    printf("unable to parse information properly\n");
  }
  
  // Print the parse tree
  printf("Parse tree:\n");
  print_tree(parse_tree_main);

  // start to generate some basic code
  gen_code(file, "output.bc", parse_tree_main);

  // free the parser and lexer since we are good.
  quit(0, "success");
  return 0;
}
