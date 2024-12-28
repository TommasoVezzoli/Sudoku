#include "helpers.h"
#include "io.h"
#include "human_solver.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> /*for better randomization*/
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
 * Function: random_transformations
 * --------------------------------
 * Applies a series of random transformations to the Sudoku grid to ensure variability
 * while preserving the Sudoku's validity and solution properties. The transformations include:
 * 
 * 1. Permuting digits 1 through 9.
 * 2. Swapping row bands.
 * 3. Swapping column bands.
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

    // (Optional) Add row-within-band or col-within-band swaps
    // for even more variety.
}

// ---------------------------------------------------------------------------------------------------- //
// --- VALID GRID GENERATOR --- //

/**
 * Function: count_solutions_recursive
 * -----------------------------------
 * Recursive helper function to count the number of solutions for a Sudoku puzzle
 * using a backtracking approach. Tracks the number of trials (search steps) performed.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure to solve.
 * - n_solutions: Pointer to a variable to count the number of solutions found.
 * - trials: Pointer to a variable to count the number of search steps performed.
 *
 * Returns:
 * - The number of solutions found.
 */
int count_solutions_recursive(Sudoku *sudoku, int *n_solutions, int *trials) {
    int row, col;

    if (!find_empty(sudoku, &row, &col)) {
        (*n_solutions)++;
        return *n_solutions;
    }

    for (int guess = 1; guess <= 9; guess++) {
        (*trials)++;  // Increment the trial counter for each attempt
        if (is_valid(sudoku, guess, row, col)) {
            sudoku->table[row][col] = guess;
            if (count_solutions_recursive(sudoku, n_solutions, trials) == N_SOL) {
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
 * Counts the number of solutions for a given Sudoku puzzle using a backtracking algorithm.
 * It also computes the total number of trials (or search steps) performed during the enumeration process,
 * which can be used to evaluate the computational complexity of solving the puzzle.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure representing the puzzle to solve.
 *
 * Returns:
 * - The number of solutions found for the Sudoku puzzle.
 */
int count_solutions(Sudoku *sudoku) {
    int n_solutions = 0;
    int trials = 0;  // Initialize the trial counter
    return count_solutions_recursive(sudoku, &n_solutions, &trials);
}

// ---------------------------------------------------------------------------------------------------- //
// --- UNIQUE SOLUTION CHECKER --- //

/**
 * Function: check_uniqueness_reduction_absurdity
 * ----------------------------------------------
 * Verifies if removing a cell value preserves a unique solution using a "reduction to absurdity" approach.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure.
 * - row: Row index of the cell.
 * - col: Column index of the cell.
 *
 * Returns:
 * - 1 if the cell can be safely removed while preserving uniqueness.
 * - 0 otherwise.
 */
int check_uniqueness_reduction_absurdity(Sudoku *sudoku, int row, int col)
{
    // Make a local copy so we don't mess up the original puzzle
    Sudoku localCopy;
    memcpy(&localCopy, sudoku, sizeof(Sudoku));
    
    // The digit we're 'digging out'
    int removedDigit = localCopy.table[row][col];

    // Remove the digit from the local copy
    localCopy.table[row][col] = 0;
    
    // Try every possible digit except the removed one
    for (int guess = 1; guess <= 9; guess++) {
        if (guess == removedDigit) 
            continue;
        
        if (is_valid(&localCopy, guess, row, col)) {
            localCopy.table[row][col] = guess;
            
            // If a valid solution exists with this 'guess',
            // that means there's *at least* two solutions overall.
            if (count_solutions(&localCopy) > 0) {
                return 0;  // NOT unique
            }
            
            // Revert to empty and continue checking
            localCopy.table[row][col] = 0;
        }
    }

    // If no alternate digit yields a solution,
    // the puzzle is unique with the originally removed digit.
    return 1; // Unique
}


bool solve_sudoku(
    Sudoku *sudoku,
    int *n_solutions
) {
    int row, col;

    // Update the number of solutions and save the current one if found
    if (!find_empty(sudoku, &row, &col)) {
        (*n_solutions)++;
        char filename[256];
        snprintf(filename, sizeof(filename), "Solutions/solution%d.txt", *n_solutions);
        write_to_file(sudoku, filename);
        
        if (*n_solutions == N_SOL) {
            return true;
        }
        return false;
    }

    for (int guess = 1; guess <= 9; guess++) {
        if (is_valid(sudoku, guess, row, col)) {
            sudoku->table[row][col] = guess;
            if (solve_sudoku(sudoku, n_solutions)) {
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

/*I USED THE BACKTRACK SOLVER INSTEAD OF THE COUNT_SOLUTION AS IT IS MORE RELIABLE; COUNT SOLUTIONS MAY ENTER INFINITE LOOP*/
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

    clock_t start_time = clock(); // Record the start time

    while (true) {

        // Check timeout
        clock_t current_time = clock();
        double elapsed_time = ((double)(current_time - start_time)) / CLOCKS_PER_SEC;
        if (elapsed_time > TIMEOUT_SECONDS) {
            printf("\nTimeout exceeded during grid generation!\n");
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
        // Terminate if the puzzle has a unique solution
        // if (count_solutions(sudoku) >= 1) {
        //     return true;
        // }
        int n_solutions = 0;
        if (solve_sudoku(sudoku, &n_solutions)) {
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
 * -----------------------
 * Get the next cell to be dug in the generation of the Sudoku puzzle.
 * 
 * Parameters:
 * - row: Pointer to the row of the cell.
 * - col: Pointer to the col of the cell.
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
 * Sample the number of cells to be given in the generated puzzle.
 * Note: The bounds for the number of given cells in the generated puzzle are listed below.
 *      Level     Total cells
 *      ------------------------
 *        1         39 - 45
 *        2         34 - 38
 *        3         28 - 33
 *        4         23 - 27
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
        return bound % 7 + 39;
    } else if (level == 2) {
        return bound % 5 + 34;
    } else if (level == 3) {
        return bound % 6 + 28;
    } else if (level == 4) {
        return bound % 5 + 21;
    }
}


/**
 * Function: sample_row_col_bound
 * ------------------------------
 * Samples the lower bound for the number of given cells in each row and column 
 * for a Sudoku puzzle based on the difficulty level.
 *
 * The row/column lower bound values are determined as follows:
 * 
 *      Level     Minimum Given Cells in Rows/Columns
 *      ---------------------------------------------
 *        1                  4
 *        2                  3
 *        3                  2
 *        4                  0
 *
 * Parameters:
 * - level: The difficulty level of the Sudoku (1–4).
 *
 * Returns:
 * - The minimum number of given cells required in rows and columns for the specified level.
 */
int sample_row_col_bound(int level) {
    if (level == 1) {
        return 4;
    } else if (level == 2) {
        return 3;
    } else if (level == 3) {
        return 2;
    } else if (level == 4) {
        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------- //

// --- DIGGING FOR level 1-3-4 --- //
/**
 * Function: timed_dig
 * -------------------
 * Implements the digging procedure for Sudoku level 4. Cells are removed iteratively 
 * until the desired number of givens is reached or a timeout occurs.
 *
 * Key features:
 * - A timeout mechanism is in place to prevent excessive computation time.
 * - Ensures the puzzle remains uniquely solvable after each cell removal.
 * - Applies level-specific constraints on the number of givens per row and column.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku grid to be modified.
 * - level: The difficulty level for which the grid is being generated.
 */
void sudoku_dig(Sudoku *sudoku, int level, int cell_bound) {
    int row_col_bound;

    row_col_bound = sample_row_col_bound(level);

    int givens_row_arr[N] = {0};
    int givens_col_arr[N] = {0};
    int total_givens = N * N;

    clock_t start_time = clock(); // Record the start time

    // Initialize counts of givens per row and column
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (sudoku->table[i][j] != 0) {
                givens_col_arr[j]++;
                givens_row_arr[i]++;
            }
        }
    }

    int row = rand() % N;
    int col = rand() % N;

    while (total_givens > cell_bound) {
        // Check timeout
        clock_t current_time = clock();
        double elapsed_time = ((double)(current_time - start_time)) / CLOCKS_PER_SEC;
        if (elapsed_time > TIMEOUT_SECONDS) {
            printf("\nTimeout exceeded during digging!\n");
            break;
        }

        if (sudoku->table[row][col] == 0) {
            get_next_cell(&row, &col);
            continue; // Skip already empty cells
        }

        if (givens_row_arr[row] <= row_col_bound || givens_col_arr[col] <= row_col_bound) {
            get_next_cell(&row, &col);
            continue;
        }

        int tmp = sudoku->table[row][col];

        if (check_uniqueness_reduction_absurdity(sudoku, row, col) != 1){
            sudoku->table[row][col] = tmp; // Revert if not unique
        } else {
            // Successfully removed the cell
            sudoku->table[row][col] = 0;
            givens_col_arr[col]--;
            givens_row_arr[row]--;
            total_givens--;
        }

        get_next_cell(&row, &col); // Move to the next cell
    }
}


// ---------------------------------------------------------------------------------------------------- //
// --- LEVEL ASSESSMENT --- //

/**
 * Function: assess_level
 * ----------------------
 * Evaluates the difficulty of the generated Sudoku puzzle using the specified rating formula.
 *
 * The function integrates the `solve_human` method to simulate solving the puzzle and
 * collects the frequency of each technique used. It then calculates the difficulty score `D`
 * based on the following formula:
 *
 *      D = 0.2 * (50 - G) + 0.15 * (5 - R) + 0.4 * T + 0.25 * S / 10
 *
 * Where:
 * - G: Number of given cells in the puzzle.
 * - R: Minimum number of givens in rows/columns.
 * - T: Weighted sum of techniques used during solving:
 *       T = sum(w_i * f_i), where w_i is the weight and f_i is the frequency of technique i.
 * - S: Search depth during solving, scaled to a range of 0–10.
 *
 * The function exits the puzzle generation loop if the assessed difficulty level
 * matches the desired difficulty level. Otherwise, it retries with a new puzzle.
 *
 * Parameters:
 * - sudoku: Pointer to the digged Sudoku grid.
 * - possible: 3D boolean array representing possible values for each cell.
 * - level: Desired difficulty level (1–4).
 *
 * Returns:
 * - assessed difficulty.
 */
int assess_level(Sudoku *sudoku, SolverStats *stats, int input_level) {
    
    // 1. Calculate the number of givens (G)
    int givens = 0;
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            if (sudoku->table[row][col] != 0) {
                givens++;
            }
        }
    }
    // 2. Initialize variables for counting techniques (done outside)
    // SolverStats stats = {0};

    // 3. Solve the puzzle with the human solver and track technique stats
    Sudoku sudoku_copy;
    memcpy(&sudoku_copy, sudoku, sizeof(Sudoku)); // Create a copy of the unsolved puzzle

    // if (!solve_human(&sudoku_copy, stats) && !sudokuSolver(&sudoku_copy, stats)) { // If the puzzle is unsolvable by the human solver
    if (!solve_human2(&sudoku_copy, stats)) { 
        // printf("Puzzle unsolvable by human techniques. Retrying...\n");
        return -1; // Signal to retry in the main loop
    }

    // 4. Determine R (minimum row/column constraint met)
    int row_col_bound = sample_row_col_bound(input_level);

    // 5. Compute T (weighted human techniques)
    double weighted_techniques = 
        0.6 * stats->naked_single + 
        1 * stats->hidden_single + 
        3 * stats->naked_pair + 
        4 * stats->hidden_pair + 
        4 * stats->pointing_pair + 
        5 * stats->naked_triple + 
        5 * stats->pointing_triple + 
        6 * stats->hidden_triple + 
        7 * stats->x_wing;

    // 6. Compute S (scaled solving steps, normalized by 10)
    // int max_depth_search = 0;
    int trials = 0;
    int n_solutions = 0; // Dummy variable since we don't need the number of solutions here
    count_solutions_recursive(sudoku, &n_solutions, &trials);
    printf("Trials: %d\n", trials);
    double S = (double)trials / 100000.0 * 10.0;
    if (S > 10.0) S = 10.0; // Cap S at 10

    // 7. Compute the difficulty rating (D)
    printf("Givens: %d\n", givens);
    printf("Row/col bound: %d\n", row_col_bound);
    printf("Techniques score: %lf\n",weighted_techniques);
    printf("Solving steps: %lf\n", S);

    if (input_level == 4 && weighted_techniques < 50){
        return -1;
    }

    double difficulty = 
        0.1 * (50 - givens) + 
        0.15 * (5 - row_col_bound) + 
        0.5 * weighted_techniques + 
        0.25 * (S / 10.0);
    
    printf("Difficulty found: %lf\n", difficulty);

    // 8. Map difficulty rating to level
    int assessed_level;
    if (difficulty >= 5 && difficulty <= 15) {
        assessed_level = 1;
    } else if (difficulty > 15 && difficulty <= 20) {
        assessed_level = 2;
    } else if (difficulty > 20 && difficulty <= 24) {
        assessed_level = 3;
    } else if (difficulty > 24) {
        assessed_level = 4;
    } else {
        return -1;
    }

    // 9. Check if assessed level matches the input level
    return assessed_level;
}


// ---------------------------------------------------------------------------------------------------- //
// --- MAIN FUNCTION --- //

/*POSSIBLE IDEA: apply random transformation AFTER digging, and then the same transformation also to the original puzzle
in order to avoid having always the same digging pattern*/


/**
 * Function: main
 * --------------
 * Entry point for the Sudoku generator program. The program generates Sudoku puzzles
 * with a specific difficulty level (1–4) based on the defined rating formula.
 *
 * The main function performs the following steps:
 * 1. Accepts user input for the desired difficulty level.
 * 2. Generates a valid Sudoku grid using randomized backtracking.
 * 3. Applies the digging procedure to remove cells according to the difficulty bounds.
 * 4. Assesses the difficulty of the resulting puzzle using the `assess_level` function.
 * 5. Outputs the generated puzzle and writes it to a file if the difficulty matches.
 *
 * Returns:
 * - 0 on successful execution, otherwise an error code.
 */
int main() {
    srand(time(NULL)); // Initialize random seed

    Sudoku sudoku;
    SolverStats stats = {0};
    int level;
    printf("\nEnter the level desired (1-4): ");
    if (scanf("%d", &level) != 1) { // Validate input
        printf("Invalid input! Please enter an integer between 1 and 4.\n");
        return 1; // Exit if input is invalid
    }

    // Validate the level input
    if (level < 1 || level > 4) {
        printf("Invalid level! Please enter a value between 1 and 4.\n");
        return 1; // Exit if the level is out of range
    }

    while (1) {
        // Step 1: Generate a valid grid
        printf("\nGenerating a new valid grid...\n");
        if (!generate_valid_grid(&sudoku)) {
            printf("Failed to generate a valid grid within the time limit. Retrying...\n");
            continue; // Retry the generation process
        }
        random_transformations(&sudoku);
        print_table(&sudoku);

        // Step 2: Attempt the digging procedure
        int cell_bound = sample_cells_bound(level);
        printf("\nStarting the digging procedure...\n");
        sudoku_dig(&sudoku, level, cell_bound);

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
            printf("Generated puzzle matches desired difficulty level %d.\nStats:\n", level);
            print_stats(&stats);
            break;
        } else if (assessed_level == -1){
            printf("Retrying due to unsolvable puzzle...\n");
            continue;
        } else {
            printf("Level mismatch, next generation...\n");
            continue;
        }

    }
    print_table(&sudoku);
    write_to_file(&sudoku, "test.txt");

    return 0;
}