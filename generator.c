#include "helpers.h"
#include "io.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


#define N_STARTING_PIVOTS 11
#define N_SOL 5


// ---------------------------------------------------------------------------------------------------- //
// --- VALID GRID GENERATOR --- //


/**
 * Function: count_solutions
 * -------------------------
 * Count the number of solutions for a given Sudoku puzzle using the backtracking algorithm.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure.
 * 
 * Returns:
 * - The number of solutions found.
 */
int count_solutions(Sudoku *sudoku) {
    int n_solutions = 0;
    int row, col;

    // Helper recursive function
    int count_solutions_recursive(Sudoku *sudoku, int *n_solutions) {
        if (!find_empty(sudoku, &row, &col)) {
            (*n_solutions)++;
            return *n_solutions;
        }

        for (int guess = 1; guess <= 9; guess++) {
            if (is_valid(sudoku, guess, row, col)) {
                sudoku->table[row][col] = guess;
                if (count_solutions_recursive(sudoku, n_solutions) == N_SOL) {
                    return N_SOL;
                }
                // Backtrack if the guess was incorrect
                sudoku->table[row][col] = 0;
            }
        }
        return *n_solutions;
    }
    return count_solutions_recursive(
        sudoku,
        &n_solutions
    );
}


/**
 * Function: generate_valid_grid
 * -----------------------------
 * Generate a valid Sudoku grid starting a given number of random pivots.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure.
 */
void generate_valid_grid(Sudoku *sudoku) {
    while (true) {

        // Place random pivots in the grid to ensure uniqueness
        for (int i = 0; i < N_STARTING_PIVOTS; i++) {
            int row = rand() % 9;
            int col = rand() % 9;
            int guess = rand() % 9 + 1;
            if (is_valid(sudoku, guess, row, col)) {
                sudoku->table[row][col] = guess;
            }
        }

        // Terminate if the puzzle has a unique solution
        if (count_solutions(sudoku) == 1) {
            return;
        }
        // Reset the grid otherwise
        else {
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    sudoku->table[i][j] = 0;
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------------------------------- //
// --- DIGGING SEQUENCES AND BOUNDS --- //


/**
 * Function: get_next_cell
 * -----------------------
 * Get the next cell to be dug in the generation of the Sudoku puzzle.
 * 
 * Parameters:
 * - row: Pointer to the row of the cell.
 * - col: Pointer to the col of the cell.
 * - dig_mode: The mode of digging.
 * Note: The mapping for the dig_mode parameter is below.
 *      Value              Description
 *      -------------------------------------
 *        1       left to right top to bottom
 *        2           wandering along "S"
 *        3              skip one cell
 *        4                randomize
 */
void get_next_cell(
    int *row,
    int *col,
    int dig_mode
) {
    if (dig_mode == 1) {
        if (*row == N && *col == N) {
            break;
        }
        *col = (*col+1) % N;
        if (*col == 0) {
            *row = (*row+1);
        }
    }
    
    if (dig_mode == 2) {
        if (*row == N && *col == N) {
            break;
        }
        if (*row % 2 == 0) {
            *col = (*col+1);
            if (*col >= N) {
                *row = (*row+1);
                *col = N-1;
            }
        } else {
            *col = (*col-1);
            if (*col <= -1) {
                *row = (*row+1);
                *col = 0;
            }
        }
    }

    if (dig_mode == 3) {
        if (*row == N && *col == N) {
            break;
        }
        if (*row % 2 == 0) {
            *col = *col+2;
            if (*col >= N) {
                *row = (*row+1);
                *col = N-2;
            }
        } else {
            *col = *col-2;
            if (*col <= -1) {
                *row = (*row+1);
                *col = 0;
            }
        }
    }

    if (dig_mode == 4) {
        *row = rand() % N;
        *col = rand() % N;
    }
}


/**
 * Function: sample_cells_bound
 * ----------------------------
 * Sample the number of cells to be given in the generated puzzle.
 * Note: The bounds for the number of given cells in the generated puzzle are listed below.
 *      Level     Total cells     Cells in rows\cols
 *      --------------------------------------------
 *        1          >= 50                5
 *        2         36 - 49               4
 *        3         32 - 35               3
 *        4         28 - 31               2
 *        5         22 - 27               0
 * 
 * Parameters:
 * - level: The level of difficulty of the Sudoku.
 * 
 * Returns:
 * - The bound on the cells to be given.
 */
int sample_cells_bound(int level) {
    int bound = rand();
    if (level == 1) {
        return bound % 10 + 50;
    } else if (level == 2) {
        return bound % 14 + 36;
    } else if (level == 3) {
        return bound % 04 + 32;
    } else if (level == 4) {
        return bound % 04 + 28;
    } else if (level == 5) {
        return bound % 66 + 22;
    }
    
}

// ---------------------------------------------------------------------------------------------------- //
// --- PRUNING --- //


// ---------------------------------------------------------------------------------------------------- //
// --- EQUIVALENT PROPAGATION --- //


// ---------------------------------------------------------------------------------------------------- //
// --- MAIN FUNCTION --- //


/**
 * Function: eval_sudoku
 * ---------------------
 * Evaluate the difficulty of a Sudoku puzzle.
 * Note: the evaluation is computed as the weighted average of 4 factors.
 *             Factor            Weights
 *      --------------------------------
 *            n. Cells             0.4
 *       Cells in rows\cols        0.2
 *      Applicable technique       0.2
 *          Search times           0.2
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure.
 * 
 * Returns:
 * - The difficulty level of the Sudoku.
 */
int eval_sudoku(
    Sudoku *sudoku
) {
    int n_solutions = 0;
    int guess = sudoku->table[row][col];
    sudoku->table[row][col] = 0;
    count_solutions(sudoku);
    sudoku->table[row][col] = guess;
    return n_solutions;
}


int main() {
    Sudoku sudoku;
    generate_valid_grid(&sudoku);
    print_table(&sudoku);
    return 0;
}