#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


#define N 9


// define the struct to hold the sudoku table
typedef struct {
    int table[N][N];
} Sudoku;


// ---------------------------------------------------------------------------------------------------- //
// --- FILE I/O FUNCTIONS --- //


/**
 * Function: parse_file
 * ---------------------
 * Parses an input file to populate the Sudoku grid.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure to populate.
 * - filename: The name of the file containing the Sudoku grid.
 */
void parse_file(Sudoku *sudoku, char *filename) {
    char file_str[256];
    char *buffer = file_str;

    // open the file as a file and read its content into a buffer
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file '%s'\n", filename);
        exit(1);    
    }
    fread(file_str, sizeof(char), 256, file);

    char *end;
    int i = 0, j = 0;

    while (*buffer) {
        // skip white spaces
        while (isspace(*buffer)) buffer++;

        // convert the string to a number and put it in the table
        int num = strtod(buffer, &end);
        sudoku->table[i][j] = num;

        // update indexes
        j++;
        if (*buffer == '\n') {
            i++;
            j = 0;
        }

        // align the buffer to the next number
        while (buffer != end) buffer++;
        // break
        if (*buffer == '\0' || (i == 8 && j == 8)) break;   
    }
    fclose(file);
}


/**
 * Function: write_to_file
 * ------------------------
 * Writes the current state of the Sudoku grid to a file.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure containing the grid.
 * - filename: The name of the file to write the grid to.
 */
void write_to_file(Sudoku *sudoku, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error creating file '%s'\n", filename);
        exit(1);
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(file, "%d ", sudoku->table[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}


// ---------------------------------------------------------------------------------------------------- //
// --- HELPER FUNCTIONS --- //


/**
 * Function: find_empty
 * --------------------
 * Finds an empty cell in the Sudoku grid.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure containing the grid.
 * - row: Pointer to store the row index of the empty cell.
 * - col: Pointer to store the column index of the empty cell.
 * 
 * Returns:
 * - true if an empty cell is found, otherwise false.
 */
bool find_empty(Sudoku *sudoku, int *row, int *col) {
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
 * Checks if a given number can be placed in the specified cell.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure containing the grid.
 * - guess: The number to be placed.
 * - row: Row index of the cell.
 * - col: Column index of the cell.
 * 
 * Returns:
 * - true if the placement is valid, otherwise false.
 */
bool is_valid(Sudoku *sudoku, int guess, int row, int col) {

    // check the row
    for (int c = 0; c < N; c++) {
        if (sudoku->table[row][c] == guess) {
            return false;
        }
    }

    // check the column
    for (int r = 0; r < N; r++) {
        if (sudoku->table[r][col] == guess) {
            return false;
        }
    }

    // check the 3x3 box
    int box_row = (row / 3) * 3;
    int box_col = (col / 3) * 3;
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (sudoku->table[box_row + r][box_col + c] == guess) {
                return false;
            }
        }
    }

    return true;
}

/**
 * Function: display
 * -----------------
 * Displays the current state of the Sudoku grid.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure containing the grid.
 */
void display(Sudoku *sudoku) {
    for (int i = 0; i < N; i++) {
        //print the horizonal line at the beginning
        if (i == 0) {
            printf("--------+-------+--------");
            printf("\n");
        }

        for (int j = 0; j < N; j++) {
            // print vertical lines to separate the 3x3 blocks
            if (j == 0) {
                printf("|");
            }
            // print the number
            printf(" %d", sudoku->table[i][j]);
            // print the vertical line at the right-end
            if ((j+1) % 3 == 0) {
                printf(" |");
            }
        }

        // print horizontal lines to separate the 3x3 blocks
        if ((i+1) % 3 == 0) {
            printf("\n");
            printf("--------+-------+--------");
        }
        printf("\n");
    }
}


// ---------------------------------------------------------------------------------------------------- //
// --- SOLVER FUNCTION --- //


/**
 * Function: solve_sudoku
 * ----------------------
 * Solves the Sudoku puzzle using backtracking.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure containing the grid.
 * 
 * Returns:
 * - true if the puzzle is solved, otherwise false.
 */
bool solve_sudoku(Sudoku *sudoku) {
    int row, col;
    // if there is no empty space left, then puzzle is solved
    if (!find_empty(sudoku, &row, &col)) {
        return true;
    }

    for (int guess = 1; guess <= 9; guess++) {
        if (is_valid(sudoku, guess, row, col)) {
            sudoku->table[row][col] = guess;
            if (solve_sudoku(sudoku)) {
                return true;
            }

            // if the guess didn't lead to solution, backtrack
            sudoku->table[row][col] = 0;
        }
    }

    // trigger backtracking again
    return false;
}


// ---------------------------------------------------------------------------------------------------- //
// --- MAIN FUNCTION --- //


/**
 * Function: main
 * --------------
 * Main function to run the Sudoku solver.
 * 
 * - Reads the Sudoku puzzle from a file.
 * - Displays the initial grid.
 * - Validates the grid.
 * - Attempts to solve the puzzle.
 * - Displays the solution if one exists, otherwise prints "No solution exists".
 * - Writes the output to a file.
 */
int main() {
    Sudoku sudoku;
    parse_file(&sudoku, "sudoku_tmp.txt");
    // display(&sudoku);

    // Validate the input grid
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            int num = sudoku.table[row][col];
            if (num != 0) {
                sudoku.table[row][col] = 0;
                if (!is_valid(&sudoku, num, row, col)) {
                    sudoku.table[row][col] = num;
                    // printf("No solution exists\n");
                    write_to_file(&sudoku, "sudoku_solution.txt");
                    return 0;
                }
                sudoku.table[row][col] = num;
            }
        }
    }

    if (solve_sudoku(&sudoku)) {
        // printf("\nSolved Sudoku:\n");
        // display(&sudoku);
    } else {
        // printf("No solution exists\n");
    }
    write_to_file(&sudoku, "sudoku_solution.txt");

    return 0;
}