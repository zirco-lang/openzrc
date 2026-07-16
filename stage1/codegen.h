#ifndef __CODEGEN_H
#define __CODEGEN_H

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

#include "parser.h"
/*
 * Value list, stores a reference value for variables in a linked list
 */
typedef struct value_list {
  char name[64];
  LLVMValueRef val;
  LLVMTypeRef typ;
  struct value_list * next;
} VALUE_LIST;

void free_list(VALUE_LIST** vl);

int gen_code(char * input_name, char * output_name, GRAMMAR_T * parse_tree);


#endif
