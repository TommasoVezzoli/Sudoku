#include <stdio.h>
#include <stdlib.h>

#define DIM 9
#define SIZE_ROWS 9
#define SIZE_COLUMNS 9

int UNSOLVED = 81;

/* Function Prototypes */
void parse_file(int puzzle[DIM][DIM], const char *filename);
void printPuzzle(int puzzle[DIM][DIM]);
void setUpPuzzle(int puzzle[DIM][DIM], int possible[DIM][DIM][DIM]);
int solvePuzzle(int puzzle[DIM][DIM], int possible[DIM][DIM][DIM]);
void updatePossible(int puzzle[DIM][DIM], int possible[DIM][DIM][DIM], int row, int column);
void write_to_file(int puzzle[DIM][DIM], const char *filename);
int boxSingles(int puzzle[DIM][DIM], int possible[DIM][DIM][DIM]);
int checkRows(int puzzle[DIM][DIM], int possible[DIM][DIM][DIM]);

/* Function Definitions */

/**
 * Function: parse_file
 * ---------------------
 * Parses an input file to populate the Sudoku puzzle array.
 *
 * Parameters:
 * - puzzle: 2D array representing the Sudoku grid.
 * - filename: The name of the file containing the Sudoku grid.
 */
void parse_file(int puzzle[DIM][DIM], const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file '%s'\n", filename);
        exit(1);
    }

    for (int i = 0; i < SIZE_ROWS; i++) {
        for (int j = 0; j < SIZE_COLUMNS; j++) {
            if (fscanf(file, "%d", &puzzle[i][j]) != 1) {
                printf("Error: Invalid format in file '%s'\n", filename);
                fclose(file);
                exit(1);
            }
        }
    }
    fclose(file);
}

/**
 * Function: printPuzzle
 * ----------------------
 * Prints the current state of the Sudoku grid.
 *
 * Parameters:
 * - puzzle: 2D array representing the Sudoku grid.
 */
void printPuzzle(int puzzle[DIM][DIM]) {
    printf("-------------------------\n");
    for (int i = 0; i < SIZE_ROWS; i++) {
        printf("| ");
        for (int j = 0; j < SIZE_COLUMNS; j++) {
            printf("%d ", puzzle[i][j]);
            if (((j + 1) % 3) == 0) {
                printf("| ");
            }
        }
        printf("\n");
        if (((i + 1) % 3) == 0) {
            printf("-------------------------\n");
        }
    }
}

/**
 * Function: setUpPuzzle
 * ----------------------
 * Initializes the possible values for each cell in the Sudoku grid.
 *
 * Parameters:
 * - puzzle: 2D array representing the initial Sudoku grid.
 * - possible: 3D array representing possible values for each cell.
 */
void setUpPuzzle(int puzzle[DIM][DIM], int possible[DIM][DIM][DIM]) {
    for (int i = 0; i < SIZE_ROWS; i++) {
        for (int j = 0; j < SIZE_COLUMNS; j++) {
            if (puzzle[i][j] == 0) {
                for (int k = 0; k < DIM; k++) {
                    possible[i][j][k] = 0;
                }
            } else {
                for (int k = 0; k < DIM; k++) {
                    possible[i][j][k] = 1;
                }
            }
        }
    }
    
    for (int i = 0; i < SIZE_ROWS; i++) {
        for (int j = 0; j < SIZE_COLUMNS; j++) {
            if (puzzle[i][j] != 0) {
                updatePossible(puzzle, possible, i, j);
                UNSOLVED--;
            }
        }
    }
}

/**
 * Function: updatePossible
 * -------------------------
 * Updates the possible values for each cell in the specified row, column, and box after placing a number.
 *
 * Parameters:
 * - puzzle: 2D array representing the Sudoku grid.
 * - possible: 3D array representing possible values for each cell.
 * - row: The row index of the placed number.
 * - column: The column index of the placed number.
 */
void updatePossible(int puzzle[DIM][DIM], int possible[DIM][DIM][DIM], int row, int column) {
    int number = puzzle[row][column] - 1;

    // Update the row and column
    for (int x = 0; x < SIZE_ROWS; x++) {
        possible[row][x][number] = 1;
        possible[x][column][number] = 1;
    }

    // Update the box
    int startRow = (row / 3) * 3;
    int startCol = (column / 3) * 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            possible[startRow + i][startCol + j][number] = 1;
        }
    }
}

/**
 * Function: boxSingles
 * ---------------------
 * Checks for cells within a box that can only hold a single possible value and solves them.
 *
 * Parameters:
 * - puzzle: 2D array representing the Sudoku grid.
 * - possible: 3D array representing possible values for each cell.
 *
 * Returns:
 * - 1 if a cell is solved, otherwise 0.
 */
int boxSingles(int puzzle[DIM][DIM], int possible[DIM][DIM][DIM]) {
    int count, tempRow, tempCol;
    for (int box = 0; box < DIM; box++) {
        for (int num = 0; num < DIM; num++) {
            count = 0;
            tempRow = -1;
            tempCol = -1;
            for (int i = (box / 3) * 3; i < (box / 3) * 3 + 3; i++) {
                for (int j = (box % 3) * 3; j < (box % 3) * 3 + 3; j++) {
                    if (puzzle[i][j] == 0 && possible[i][j][num] == 0) {
                        count++;
                        tempRow = i;
                        tempCol = j;
                    }
                }
            }
            if (count == 1) {
                puzzle[tempRow][tempCol] = num + 1;
                updatePossible(puzzle, possible, tempRow, tempCol);
                UNSOLVED--;
                return 1;
            }
        }
    }
    return 0;
}

/**
 * Function: checkRows
 * --------------------
 * Checks each row for cells that can only hold a single possible value and solves them.
 *
 * Parameters:
 * - puzzle: 2D array representing the Sudoku grid.
 * - possible: 3D array representing possible values for each cell.
 *
 * Returns:
 * - 1 if a cell is solved, otherwise 0.
 */
int checkRows(int puzzle[DIM][DIM], int possible[DIM][DIM][DIM]) {
    int sum[DIM], temp[DIM];
    for (int i = 0; i < SIZE_ROWS; i++) {
        for (int j = 0; j < DIM; j++) {
            sum[j] = 0;
            temp[j] = -1;
        }
        for (int j = 0; j < SIZE_COLUMNS; j++) {
            if (puzzle[i][j] == 0) {
                for (int k = 0; k < DIM; k++) {
                    if (possible[i][j][k] == 0) {
                        sum[k]++;
                        temp[k] = j;
                    }
                }
            }
        }
        for (int k = 0; k < DIM; k++) {
            if (sum[k] == 1) {
                puzzle[i][temp[k]] = k + 1;
                updatePossible(puzzle, possible, i, temp[k]);
                UNSOLVED--;
                return 1;
            }
        }
    }
    return 0;
}

/**
 * Function: solvePuzzle
 * ----------------------
 * Attempts to solve the Sudoku puzzle by filling in cells that have only one possible value,
 * as well as checking for box singles and row/column exclusions.
 *
 * Parameters:
 * - puzzle: 2D array representing the Sudoku grid.
 * - possible: 3D array representing possible values for each cell.
 *
 * Returns:
 * - 1 if a cell is successfully solved, otherwise 0.
 */
int solvePuzzle(int puzzle[DIM][DIM], int possible[DIM][DIM][DIM]) {
    int progress = 0;
    
    // Pass 1: Solve cells with only one possible value
    for (int i = 0; i < SIZE_ROWS; i++) {
        for (int j = 0; j < SIZE_COLUMNS; j++) {
            if (puzzle[i][j] == 0) {
                int count = 0;
                int value = -1;
                for (int k = 0; k < DIM; k++) {
                    if (possible[i][j][k] == 0) {
                        count++;
                        value = k;
                    }
                }
                if (count == 1) {
                    puzzle[i][j] = value + 1;
                    updatePossible(puzzle, possible, i, j);
                    UNSOLVED--;
                    progress = 1;
                }
            }
        }
    }
    
    //Pass 2: Box singles
    if (boxSingles(puzzle, possible)) {
        progress = 1;
    }

    //Pass 3: Row checks
    if (checkRows(puzzle, possible)) {
        progress = 1;
    }

    return progress;
}

/**
 * Function: write_to_file
 * ------------------------
 * Writes the current state of the Sudoku grid to a file.
 *
 * Parameters:
 * - puzzle: 2D array representing the Sudoku grid.
 * - filename: The name of the file to write the grid to.
 */
void write_to_file(int puzzle[DIM][DIM], const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error creating file '%s'\n", filename);
        exit(1);
    }

    for (int i = 0; i < SIZE_ROWS; i++) {
        for (int j = 0; j < SIZE_COLUMNS; j++) {
            fprintf(file, "%d ", puzzle[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

/**
 * Function: main
 * --------------
 * Main function to run the Sudoku solver.
 *
 * - Creates and sets up the Sudoku puzzle.
 * - Attempts to solve the puzzle using a series of checking and updating functions.
 * - Prints the final solved or unsolved grid.
 * - Writes the solution to a file.
 */
int main() {
    int puzzle[DIM][DIM];
    int possible[DIM][DIM][DIM];
    int progress;
    int initialPuzzle[DIM][DIM];

    parse_file(puzzle, "sudoku_tmp.txt");
    setUpPuzzle(puzzle, possible);

    // printPuzzle(puzzle);

    while (UNSOLVED > 0) {
        progress = solvePuzzle(puzzle, possible);
        if (progress == 0) {
            // printf("\nFailed to solve the puzzle!\n\n");
            parse_file(initialPuzzle, "sudoku_tmp.txt"); /*TODO: find a better way to return the initial sudoku without parsing again*/
            write_to_file(initialPuzzle, "sudoku_solution.txt"); /*Write function to return input file*/
            // printPuzzle(initialPuzzle);
            break;
        }
    }
    if (UNSOLVED == 0) {
        // printf("Solution found!\n");
        write_to_file(puzzle, "sudoku_solution.txt");
        // printPuzzle(puzzle);
    }
   
    return 0;
}
