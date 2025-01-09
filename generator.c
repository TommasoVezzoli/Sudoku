#include "helpers.h"
#include "io.h"
#include "solver_human.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>


#define N_STARTING_PIVOTS 11
#define N_SOL 5
#define TIMEOUT_SECONDS 1


/******************************************************************************
 * Random Transformations
 * These preserve Sudoku validity if applied to a fully solved Sudoku.
 * If applied to a partially filled puzzle, they still produce a puzzle with
 * the same solutions (not violating constraints), but some transformations
 * may be less impactful if many cells are still empty.
 ******************************************************************************/

/**
 * Function: permute_digits
 * ------------------------
 * Randomly permutes the digits 1 through 9 in a solved or partially solved Sudoku grid.
 * This transformation replaces each digit with another unique digit, preserving 
 * the Sudoku's validity and solution properties.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku grid to be transformed.
 */
void permute_digits(Sudoku *sudoku) {
    // map[d] = the new digit for old digit d
    int map[10];
    for (int d = 1; d <= 9; d++) {
        map[d] = d;
    }
    // Shuffle the map array
    for (int d = 9; d >= 2; d--) {
        int randIndex = rand() % d + 1; // random in [1..d]
        // swap map[d], map[randIndex]
        int temp = map[d];
        map[d] = map[randIndex];
        map[randIndex] = temp;
    }

    // Apply it to sudoku->table
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            int oldVal = sudoku->table[r][c];
            if (oldVal != 0) {
                sudoku->table[r][c] = map[oldVal];
            }
        }
    }
}


/**
 * Function: swap_row_bands
 * ------------------------
 * Swaps two entire row bands in the Sudoku grid. A row band consists of three 
 * consecutive rows (e.g., rows 0–2, 3–5, or 6–8).
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku grid to be transformed.
 * - bandA: Index of the first row band to swap (0–2).
 * - bandB: Index of the second row band to swap (0–2).
 */
void swap_row_bands(Sudoku *sudoku, int bandA, int bandB) {
    if (bandA == bandB) return; // no swap needed
    for (int offset = 0; offset < 3; offset++) {
        int row1 = bandA*3 + offset;
        int row2 = bandB*3 + offset;
        for (int col = 0; col < 9; col++) {
            int temp = sudoku->table[row1][col];
            sudoku->table[row1][col] = sudoku->table[row2][col];
            sudoku->table[row2][col] = temp;
        }
    }
}


/**
 * Function: swap_col_bands
 * ------------------------
 * Swaps two entire column bands in the Sudoku grid. A column band consists of three 
 * consecutive columns (e.g., columns 0–2, 3–5, or 6–8).
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku grid to be transformed.
 * - bandA: Index of the first column band to swap (0–2).
 * - bandB: Index of the second column band to swap (0–2).
 */
void swap_col_bands(Sudoku *sudoku, int bandA, int bandB) {
    if (bandA == bandB) return;
    for (int offset = 0; offset < 3; offset++) {
        int col1 = bandA*3 + offset;
        int col2 = bandB*3 + offset;
        for (int row = 0; row < 9; row++) {
            int temp = sudoku->table[row][col1];
            sudoku->table[row][col1] = sudoku->table[row][col2];
            sudoku->table[row][col2] = temp;
        }
    }
}


/**
 * Function: rotate_sudoku
 * ------------------------
 * Rotates the Sudoku grid by the specified angle.
 * Valid angles are 90, 180, and 270 degrees.
 * The rotation changes the positions of the digits in the grid while preserving the Sudoku's solution properties.

 * Parameters:
 * - sudoku: Pointer to the Sudoku grid to be rotated.
 * - angle: The angle by which the Sudoku grid should be rotated (90, 180, or 270 degrees).
 */
void rotate_sudoku(Sudoku *sudoku, int angle) {
    Sudoku temp = *sudoku;
    if (angle == 90) {
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                sudoku->table[c][8 - r] = temp.table[r][c];
            }
        }
    } else if (angle == 180) {
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                sudoku->table[8 - r][8 - c] = temp.table[r][c];
            }
        }
    } else if (angle == 270) {
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                sudoku->table[8 - c][r] = temp.table[r][c];
            }
        }
    }
}


/**
 * Function: reflect_sudoku
 * -------------------------
 * Reflects the Sudoku grid along the specified axis.
 * The reflection can be horizontal (H) or vertical (V).
 * This transformation adjusts the positions of the digits while preserving the solution properties.

 * Parameters:
 * - sudoku: Pointer to the Sudoku grid to be reflected.
 * - axis: The axis of reflection ('H' for horizontal, 'V' for vertical).
 */
void reflect_sudoku(Sudoku *sudoku, char axis) {
    Sudoku temp = *sudoku;
    if (axis == 'H') { // Horizontal reflection
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                sudoku->table[r][8 - c] = temp.table[r][c];
            }
        }
    } else if (axis == 'V') { // Vertical reflection
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                sudoku->table[8 - r][c] = temp.table[r][c];
            }
        }
    }
}


/**
 * Function: random_transformations
 * --------------------------------
 * Applies a series of random transformations to the Sudoku grid to ensure variability
 * while preserving the Sudoku's validity and solution properties. The transformations include:
 * 
 * 1. Permuting digits 1 through 9.
 * 2. Swapping row bands.
 * 3. Swapping column bands.
 * 4. Rotation or reflection of the grid.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku grid to be transformed.
 */
void random_transformations(Sudoku *sudoku) {
    // 1) Permute digits
    permute_digits(sudoku);

    // 2) Random row band swaps
    int bandA = rand() % 3;
    int bandB = rand() % 3;
    swap_row_bands(sudoku, bandA, bandB);

    // do one more random swap
    bandA = rand() % 3;
    bandB = rand() % 3;
    swap_row_bands(sudoku, bandA, bandB);

    // 3) Random column band swaps
    bandA = rand() % 3;
    bandB = rand() % 3;
    swap_col_bands(sudoku, bandA, bandB);

    bandA = rand() % 3;
    bandB = rand() % 3;
    swap_col_bands(sudoku, bandA, bandB);

    // 4) Rotations and reflections
    int transform = rand() % 4;
    switch (transform) {
        case 0:
            rotate_sudoku(sudoku, 90); // Rotate 90 degrees
            break;
        case 1:
            rotate_sudoku(sudoku, 180); // Rotate 180 degrees
            break;
        case 2:
            rotate_sudoku(sudoku, 270); // Rotate 270 degrees
            break;
        case 3:
            reflect_sudoku(sudoku, 'H'); // Reflect horizontally
            break;
        case 4:
            reflect_sudoku(sudoku, 'V'); // Reflect vertically
            break;
    }

}


// ---------------------------------------------------------------------------------------------------- //
// --- UNIQUE SOLUTION CHECKER --- //

/**
 * Function: count_solutions_recursive
 * -----------------------------------
 * Helper function that recursively counts the number of solutions for a given Sudoku puzzle using 
 * backtracking. Tracks the number of trials (search steps) and ensures computation stays within a 
 * given timeout.

 * Parameters:
 * - sudoku: Pointer to the Sudoku grid.
 * - n_solutions: Pointer to a variable that tracks the number of solutions found.
 * - trials: Pointer to a variable that tracks the number of search steps performed.
 * - start_time: The clock time when the function was called in dynamic_dig, used for timeout checks.

 * Returns:
 * - The total number of solutions found so far. If the timeout is exceeded, returns the current count.
 */
int count_solutions_recursive(Sudoku *sudoku, int *n_solutions, int *trials, time_t start_time) {
    int row, col;

    time_t current_time = time(NULL);
    double elapsed_time = ((double)(current_time - start_time)) / CLOCKS_PER_SEC;
    if (elapsed_time > TIMEOUT_SECONDS) {
        // printf("\nTimeout exceeded during grid generation!\n");
        return false; // Signal failure to generate a count solutions within the time limit
    }

    if (!find_empty(sudoku, &row, &col)) {
        (*n_solutions)++;
        return *n_solutions;
    }

    for (int guess = 1; guess <= 9; guess++) {
        (*trials)++;  // Increment the trial counter for each attempt
        if (is_valid(sudoku, guess, row, col)) {
            sudoku->table[row][col] = guess;
            if (count_solutions_recursive(sudoku, n_solutions, trials, start_time) == N_SOL) {
                return N_SOL;
            }
            sudoku->table[row][col] = 0;  // Backtrack
        }
    }
    return *n_solutions;
}

/**
 * Function: count_solutions
 * -------------------------
 * Counts the number of solutions for a given Sudoku puzzle using the recursive helper function 
 * `count_solutions_recursive`. Includes timeout management for the computation.

 * Parameters:
 * - sudoku: Pointer to the Sudoku grid.
 * - start_time: The clock time when the function was called in dynamic_dig, used for timeout checks.

 * Returns:
 * - The total number of solutions for the Sudoku grid. If the timeout is exceeded, the count may be incomplete.
 */
int count_solutions(Sudoku *sudoku, time_t start_time) {
    int n_solutions = 0;
    int trials = 0;  // Initialize the trial counter
    return count_solutions_recursive(sudoku, &n_solutions, &trials, start_time);
}


// ---------------------------------------------------------------------------------------------------- //
// --- VALID GRID GENERATOR --- //

/**
 * Function: solve_sudoku
 * -----------------------
 * Attempts to solve the given Sudoku puzzle using a backtracking algorithm. 
 * Tracks the number of solutions found and ensures computation stays within a timeout limit.

 * Parameters:
 * - sudoku: Pointer to the Sudoku grid to be solved.
 * - n_solutions: Pointer to a variable that tracks the number of solutions found.
 * - start_time: The clock time when the function generate_valid_grid was called, used for timeout checks.

 * Returns:
 * - true if the grid has at least one solution (indicating a valid puzzle), false otherwise.
 */
bool solve_sudoku(
    Sudoku *sudoku,
    int *n_solutions,
    time_t start_time
) {
    int row, col;

    time_t current_time = time(NULL);
    double elapsed_time = ((double)(current_time - start_time)) / CLOCKS_PER_SEC;
    if (elapsed_time > TIMEOUT_SECONDS) {
        // printf("\nTimeout exceeded during grid generation!\n");
        return false; // Signal failure to find a solution within the time limit
    }

    // Update the number of solutions
    if (!find_empty(sudoku, &row, &col)) {
        (*n_solutions)++;
        
        if (*n_solutions == N_SOL) {
            return true;
        }
        return false;
    }

    for (int guess = 1; guess <= 9; guess++) {
        if (is_valid(sudoku, guess, row, col)) {
            sudoku->table[row][col] = guess;
            if (solve_sudoku(sudoku, n_solutions, start_time)) {
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

/**
 * Function: generate_valid_grid
 * -----------------------------
 * Generates a valid Sudoku grid by placing random pivots and ensuring the puzzle 
 * has at least one solution. Includes a timing mechanism to prevent excessive computation.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure to be modified.
 *
 * Returns:
 * - true if the grid is successfully generated within the allowed time, false otherwise.
 */
bool generate_valid_grid(Sudoku *sudoku) {

    time_t start_time = time(NULL); // Record the start time

    while (true) {

        // Check timeout
        time_t current_time = time(NULL);
        double elapsed_time = ((double)(current_time - start_time)) / CLOCKS_PER_SEC;
        if (elapsed_time > TIMEOUT_SECONDS) {
            // printf("\nTimeout exceeded during grid generation!\n");
            return false; // Signal failure to generate a grid within the time limit
        }

        // Place random pivots in the grid to ensure uniqueness
        for (int i = 0; i < N_STARTING_PIVOTS; i++) {
            int row = rand() % 9;
            int col = rand() % 9;
            int guess = rand() % 9 + 1;
            if (is_valid(sudoku, guess, row, col)) {
                sudoku->table[row][col] = guess;
            }
        }
        // Terminate if the puzzle has a solution
        int n_solutions = 0;

        if (solve_sudoku(sudoku, &n_solutions, start_time)) {
            return true;
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
 * ------------------------
 * Randomly selects the next cell in the Sudoku grid to attempt removal or modification during the digging phase.

 * Parameters:
 * - row: Pointer to an integer that will hold the row index of the selected cell.
 * - col: Pointer to an integer that will hold the column index of the selected cell.
 */
void get_next_cell(
    int *row,
    int *col
    ) {
    *row = rand() % N;
    *col = rand() % N;
}


/**
 * Function: sample_cells_bound
 * ----------------------------
 * Generates the bound for the number of filled cells to leave in a Sudoku puzzle,
 * based on the desired difficulty level. Adjusts the range of filled cells to reflect
 * different levels of difficulty.

 * Parameters:
 * - level: Desired difficulty level (1–4).

 * Returns:
 * - A random value within the specified range for the number of filled cells:
 *   Level 1: 33–40 cells.
 *   Level 2: 28–33 cells.
 *   Level 3: 24–28 cells.
 *   Level 4: 19–24 cells.
 */
int sample_cells_bound(int level) {
    int bound = rand();
    if (level == 1) {
        // Easy puzzles generally have more givens (e.g., 33-40 cells)
        return bound % 7 + 33; // Range: 33-40
    } else if (level == 2) {
        // Medium puzzles have slightly fewer givens (e.g., 28-33 cells)
        return bound % 5 + 28; // Range: 28-33
    } else if (level == 3) {
        // Hard puzzles have even fewer givens (e.g., 24-28 cells)
        return bound % 4 + 24; // Range: 24-28
    } else if (level == 4) {
        // Very hard puzzles have the fewest givens (e.g., 19-24 cells)
        return bound % 5 + 19; // Range: 19-24
    }
    return 30; // Default fallback
}


// ---------------------------------------------------------------------------------------------------- //


// --- DIGGING --- //
/**
 * Function: dynamic_dig
 * ----------------------
 * Implements a flexible cell removal strategy for creating Sudoku puzzles.
 * Cells are removed iteratively while ensuring the puzzle remains uniquely solvable and adheres to the required difficulty level.
 * This method checks if the puzzle requires techniques of a certain level after removing each cell.

 * Parameters:
 * - sudoku: Pointer to the Sudoku grid to be modified.
 * - level: Desired difficulty level (1–4).
 * - cell_bound: Minimum number of cells that must remain filled in the puzzle.
 */
void dynamic_dig(Sudoku *sudoku, int level, int cell_bound) {
    int total_givens = 81;
    time_t start_time = time(NULL);
    bool solving_mode = false;

    while (total_givens > cell_bound) {
        time_t current_time = time(NULL);
        double elapsed_time = ((double)(current_time - start_time)) / CLOCKS_PER_SEC;
        if (elapsed_time > TIMEOUT_SECONDS) {
            // printf("Timeout during digging!\n");
            break;
        }

        int row = rand() % 9;
        int col = rand() % 9;
        if (sudoku->table[row][col] == 0) continue; // Skip empty cells

        int backup = sudoku->table[row][col];
        sudoku->table[row][col] = 0;

        SolverStats temp_stats = {0};
        Sudoku temp_sudoku;
        memcpy(&temp_sudoku, sudoku, sizeof(Sudoku));

        if (!solve_human(&temp_sudoku, &temp_stats, solving_mode) || count_solutions(sudoku, time(NULL)) != 1) {
            sudoku->table[row][col] = backup; // Restore if unsolvable or not unique
            continue;
        }

        total_givens--;

        // Check if the desired technique level is achieved
        bool requires_level_3 = temp_stats.naked_triple > 0 || temp_stats.pointing_triple > 0 || temp_stats.hidden_triple > 0;
        bool requires_level_4 = temp_stats.hidden_triple > 0;
        if ((level == 3 && requires_level_3) || (level == 4 && requires_level_4)) {
            break;
        }
    }
}

// ---------------------------------------------------------------------------------------------------- //
// --- LEVEL ASSESSMENT --- //

/**
 * Function: assess_level
 * -----------------------
 * Determines the difficulty level of a Sudoku puzzle based on the techniques
 * required to solve it using a human-like solver. Flags techniques into four categories
 * and returns the level that matches the puzzle's requirements.

 * Parameters:
 * - sudoku: Pointer to the Sudoku grid to be assessed.
 * - stats: Pointer to a `SolverStats` structure that tracks the solving techniques used.
 * - input_level: The desired difficulty level (1–4).

 * Returns:
 * - The assessed difficulty level if it matches `input_level`.
 * - -1 if the puzzle is unsolvable, doesn't match the desired level, or lacks the required techniques.
 */
int assess_level(Sudoku *sudoku, SolverStats *stats, int input_level) {
    // Copy the Sudoku puzzle
    Sudoku sudoku_copy;
    memcpy(&sudoku_copy, sudoku, sizeof(Sudoku));
    bool solving_mode = true;

    // Solve the puzzle with the human solver and track techniques
    if (!solve_human(&sudoku_copy, stats, solving_mode)) {
        return -1; // Signal to retry if unsolvable
    }

    // Flags for technique levels
    bool requires_level_1 = stats->naked_single > 0 || stats->hidden_single > 0;
    bool requires_level_2 = ((stats->naked_pair + stats->hidden_pair + stats->pointing_pair) > 0);
    bool requires_level_3 = ((stats->naked_triple + stats->pointing_triple + stats->hidden_triple) > 0);
    bool requires_level_4 = stats->x_wing > 0;

    // Determine assessed level
    int assessed_level = 1; // Default to level 1
    if (requires_level_4) {
        assessed_level = 4;
    } else if (requires_level_3) {
        assessed_level = 3;
    } else if (requires_level_2) {
        assessed_level = 2;
    }

    // Check if the assessed level matches the input level
    if (assessed_level != input_level) {
        return -1; // Signal to retry
    }

    return assessed_level;
}



// ---------------------------------------------------------------------------------------------------- //
// --- MAIN FUNCTION --- //

// Array of known seed filenames for level 3 and level 4
const char *level_3_seeds[] = {
    "Seeds/level3/puzzle1.txt",
    "Seeds/level3/puzzle2.txt",
    "Seeds/level3/puzzle3.txt",
    "Seeds/level3/puzzle4.txt",
    "Seeds/level3/puzzle5.txt",
    "Seeds/level3/puzzle6.txt",
    "Seeds/level3/puzzle7.txt",
    "Seeds/level3/puzzle8.txt",
    "Seeds/level3/puzzle9.txt",
    "Seeds/level3/puzzle10.txt"
};

const char *level_4_seeds[] = {
    "Seeds/level4/puzzle1.txt",
    "Seeds/level4/puzzle2.txt",
    "Seeds/level4/puzzle3.txt",
    "Seeds/level4/puzzle4.txt",
    "Seeds/level4/puzzle5.txt"
};

/**
 * Function: main
 * --------------
 * Entry point for the Sudoku generator program. Handles puzzle generation
 * for difficulty levels 1–4. Levels 1–2 are generated dynamically using a
 * structured digging process, while levels 3–4 are based on pre-existing
 * seed puzzles with random transformations.

 * Key Steps:
 * 1. Accepts user input for the desired difficulty level.
 * 2. For levels 1–2, generates a valid grid dynamically and removes cells while ensuring uniqueness.
 * 3. For levels 3–4, selects a random pre-generated seed puzzle, applies random transformations,
 *    and ensures it adheres to the desired level. This because generating hard puzzles on the fly 
 *    could be computationally intensive, making user experience slow.
 * 4. Outputs the generated puzzle and writes it to a file if it matches the input level.

 * Returns:
 * - 0 on successful execution, or an error code for invalid inputs.
 */
int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    srand(time(NULL)); // Initialize random seed

    Sudoku sudoku;
    SolverStats stats = {0};

    memset(sudoku.table, 0, sizeof(sudoku.table));  // Set all elements to 0

    int level = atoi(argv[1]);
    // printf("\nEnter the level desired (1-4): ");
    // if (scanf("%d", &level) != 1) { // Validate input
    //     printf("Invalid input! Please enter an integer between 1 and 4.\n");
    //     return 1; // Exit if input is invalid
    // }

    // Validate the level input
    // if (level < 1 || level > 4) {
    //     printf("Invalid level! Please enter a value between 1 and 4.\n");
    //     return 1; // Exit if the level is out of range
    // }

    if (level <= 2) {
        while (1) {
            // Step 1: Generate a valid grid
            // printf("\nGenerating a new valid grid...\n");
            if (!generate_valid_grid(&sudoku)) {
                // printf("Failed to generate a valid grid within the time limit. Retrying...\n");
                continue; // Retry the generation process
            }
            // printf("\nGrid generated!\n");
            random_transformations(&sudoku);

            // Step 2: Attempt the digging procedure
            int cell_bound = sample_cells_bound(level);
            // printf("\nStarting the digging procedure...\n");
            dynamic_dig(&sudoku, level, cell_bound);

            // Check if the puzzle meets the desired criteria
            int total_givens = 0;
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    if (sudoku.table[i][j] != 0) {
                        total_givens++;
                    }
                }
            }
            if (total_givens > cell_bound) {
                continue;
            }

            SolverStats stats = {0};
            int assessed_level = assess_level(&sudoku, &stats, level);
            if (assessed_level == level) {
                // printf("Generated puzzle matches desired difficulty level %d.\nStats:\n", level);
                break;
            } else if (assessed_level == -1) {
                // printf("Retrying due to unsolvable puzzle...\n");
                continue;
            } else {
                // printf("Level mismatch, next generation...\n");
                continue;
            }
        }
    } else {
        while (1) {
            // Step 1: Select a random seed puzzle for levels 3 and 4
            const char** seed_files = (level == 3) ? level_3_seeds : level_4_seeds;
            int seed_count = (level == 3) ? sizeof(level_3_seeds) / sizeof(level_3_seeds[0]) :
                                            sizeof(level_4_seeds) / sizeof(level_4_seeds[0]);
            int random_index = rand() % seed_count;

            parse_file(&sudoku, seed_files[random_index]);

            // Step 2: Apply random transformations
            random_transformations(&sudoku); //NEED TO ADJUST, IT DOES NOT WORK
            // printf("\nTransformed puzzle:\n");
            SolverStats stats = {0};
            int assessed_level = assess_level(&sudoku, &stats, level);
            if (assessed_level == level) {
                // printf("Generated puzzle matches desired difficulty level %d.\nStats:\n", level);
                // print_stats(&stats);
                break;
            }else if (assessed_level == -1) {
                // printf("Retrying due to unsolvable puzzle...\n");
                break;
            } else {
                // printf("Level mismatch, next generation...\n");
                continue;
            }
        }
    }

    // Save the resulting puzzle to a file
    // print_table(&sudoku);
    write_to_file(&sudoku, "Tmp/sudoku_gen.txt");

    return 0;
}