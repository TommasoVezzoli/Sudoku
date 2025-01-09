#include "io.h"
#include <stdbool.h>
#ifndef HELPERS_H
#define HELPERS_H

bool find_empty(
    Sudoku *sudoku,
    int *row,
    int *col
);

bool is_valid(
    Sudoku *sudoku,
    int guess,
    int row,
    int col
);

#endif