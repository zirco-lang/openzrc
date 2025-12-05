#pragma once

/**
 * Stage one compiler - Preprocessor.
 *
 * file_name - input file
 * out_file_name - Output file
 * path - Include paths
 * path_size - Length of include paths
 * return success
 */
int stage1(char * file_name, char * out_file_name, char** path, int path_size);
