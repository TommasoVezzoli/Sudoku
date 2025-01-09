#include "helpers.h"
#include <stdbool.h>
#include <stdlib.h>


// ---------------------------------------------------------------------------------------------------- //
// --- HELPER FUNCTIONS --- //


/**
 * Function: find_empty
 * --------------------
 * Find an empty cell in the Sudoku grid.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure.
 * - row: Pointer for the row index.
 * - col: Pointer for the col index.
 * 
 * Returns:
 * - true if an empty cell is found,
 *   false otherwise.
 */
bool find_empty(
    Sudoku *sudoku,
    int *row,
    int *col
) {
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (sudoku->table[r][c] == 0) {
                *row = r;
                *col = c;
                return true;
            }
        }
    }
    return false;
}


/**
 * Function: is_valid
 * ------------------
 * Check if a given number can be placed in a given cell.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure.
 * - guess: Number to be placed.
 * - row: Row index.
 * - col: Col index.
 * 
 * Returns:
 * - true if the placement is valid,
 *   false otherwise.
 */
bool is_valid(
    Sudoku *sudoku,
    int guess,
    int row,
    int col
) {

    // Check row and col
    for (int i = 0; i < N; i++) {
        if ((sudoku->table[row][i] == guess) || (sudoku->table[i][col] == guess)) {
            return false;
        }
    }

    // Check the 3x3 box
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (sudoku->table[(row/3)*3 + r][(col/3)*3 + c] == guess) {
                return false;
            }
        }
    }

    return true;
}


// ---------------------------------------------------------------------------------------------------- //
// --- HUMAN LOGIC FUNCTIONS --- //


