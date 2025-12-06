#pragma once

typedef enum output_type {
    OT_TOKENS = 0,
    OT_TREE,
    OT_LLVM,
    OT_OBJ
} stage2_output_type;

int stage2(char * original_file, char * in_file, char * out_file, stage2_output_type output_type);
