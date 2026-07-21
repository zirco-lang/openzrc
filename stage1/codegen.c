#include "codegen.h"
#include "lexer.h"
#include "parser.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>
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
  case PARSER_BINARY_OP:
    {
      BINARY_OP * uop = (BINARY_OP*) parse_tree->val;
      LLVMValueRef lhs;
      LLVMValueRef rhs;
      gen_expr(uop->lhs, mod, builder, vl, &lhs, i+1);
      gen_expr(uop->rhs, mod, builder, vl, &rhs, i+1);

      TOKEN * typ = uop->typ;

      switch (typ->tok) {
      case TOK_PLUS:
	{
	  char name[32];
	  snprintf(name, 32, "__add_%d", i);
	  *out = LLVMBuildAdd(*builder, lhs, rhs, name);
	}
	break;
      case TOK_MINUS:
	{
	  char name[32];
	  snprintf(name, 32, "__sub_%d", i);
	  *out = LLVMBuildSub(*builder, lhs, rhs, name);
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
      PARSE_DECL * decl = (PARSE_DECL*)parse_tree->val;
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
