#pragma once
#include <parser.h>
#include <stdio.h>

int gen_llvm(parse_root_t ast, char * original_file, FILE * out, token * literals, int num_literals);