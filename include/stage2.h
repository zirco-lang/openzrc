#pragma once

typedef enum output_type {
    OT_TOKENS = 0,
    OT_TREE
} stage2_output_type;

int stage2(char * in_file, char * out_file, stage2_output_type output_type);
