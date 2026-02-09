#ifndef _GRID_H
#define _GRID_H

#include <stddef.h>

size_t *grid_calculate(char **array,size_t array_count,size_t column);
void grid_print(char **array,size_t array_count,size_t column);
void grid_print_auto(char **array,size_t array_count);

#endif
