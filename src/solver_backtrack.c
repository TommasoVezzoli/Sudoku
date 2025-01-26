#include "helpers.h"
#include "io.h"
#include "solver_human.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N_SOL 5

#ifdef _WIN32
    #define PATH_SEPARATOR "\\"
#else
    #define PATH_SEPARATOR "/"
#endif

// ---------------------------------------------------------------------------------------------------- //
// --- SOLVER --- //


/**
 * Function: solve_sudoku
 * ----------------------
 * Solve the Sudoku puzzle using the backtracking algorithm.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure.
 * - n_solutions: Pointer to the number of solutions.
 * 
 * Returns:
 * - true if the puzzle is solved,
 *   false otherwise.
 */
bool solve_sudoku(
    Sudoku *sudoku,
    int *n_solutions,
    char *output_path
) {
    int row, col;

    // Update the number of solutions and save the current one if found
    if (!find_empty(sudoku, &row, &col)) {
        (*n_solutions)++;
        char filename[256];

        snprintf(filename, sizeof(filename), "%s" PATH_SEPARATOR "solution%d.txt", output_path, *n_solutions);
        write_to_file(sudoku, filename);
        
        if (*n_solutions == N_SOL) {
            return true;
        }
        return false;
    }
    for (int guess = 1; guess <= 9; guess++) {
        if (is_valid(sudoku, guess, row, col)) {
            sudoku->table[row][col] = guess;
            if (solve_sudoku(sudoku, n_solutions, output_path)) {
                if (*n_solutions == N_SOL) {
                    return true;
                }
            }
            // Backtrack if the guess was incorrect
            sudoku->table[row][col] = 0;
        }
    }
    return false;
}


// ---------------------------------------------------------------------------------------------------- //
// --- MAIN FUNCTION --- //


/**
 * Function: main
 * --------------
 * Main function to run the Sudoku solver.
 * 
 * - Read the Sudoku from a file.
 * - Print the initial grid.
 * - Validate it.
 * - Attempts to solve it.
 * - Print the number of solutions found (it can be at most N_SOL).
 */
int main(
    int argc,
    char *argv[]
) {

    if (argc != 4) {
        printf("Usage: %s <input_file> <output_path> <log_path>\n", argv[0]);
        return 1;
    }
    Sudoku sudoku;
    parse_file(&sudoku, argv[1]);

    Sudoku sudoku_copy;
    memcpy(&sudoku_copy, &sudoku, sizeof(Sudoku));
    SolverStats stats_copy = {0};
    solve_human(&sudoku_copy, &stats_copy, true, argv[3]);

    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            int num = sudoku.table[row][col];
            if (num != 0) {
                sudoku.table[row][col] = 0;
                if (!is_valid(&sudoku, num, row, col)) {
                    sudoku.table[row][col] = num;
                    printf("Invalid Sudoku\n");
                    // write_to_file(&sudoku, "sudoku_solution.txt");
                    return 0;
                }
                sudoku.table[row][col] = num;
            }
        }
    }

    int n_solutions = 0;
    solve_sudoku(&sudoku, &n_solutions, argv[2]);
    // printf("Found %d solutions out of %d\n", n_solutions, N_SOL);
    return 0;
}