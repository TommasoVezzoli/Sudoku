#include "io.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>


// ---------------------------------------------------------------------------------------------------- //
// --- INPUT/OUTPUT FUNCTIONS --- //


/**
 * Function: parse_file
 * --------------------
 * Parse an input file to populate the Sudoku grid.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure.
 * - filename: Name of the file.
 */
void parse_file(Sudoku *sudoku, char *filename) {
    char file_str[256];
    char *buffer = file_str;

    // Open the file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file '%s'\n", filename);
        exit(1);    
    }
    // Read the file into a buffer
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
        // break if the buffer is empty or the table is full
        if (*buffer == '\0' || (i == 8 && j == 8)){
            break;
        }
    }
    fclose(file);
}


/**
 * Function: print_table
 * ---------------------
 * Print the current state of the Sudoku grid to the terminal.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure.
 */
void print_table(Sudoku *sudoku) {
    for (int i = 0; i < N; i++) {
        
        if (i == 0) {
            printf("--------+-------+--------");
            printf("\n");
        }

        for (int j = 0; j < N; j++) {
            if (j == 0) {
                printf("|");
            }
            printf(" %d", sudoku->table[i][j]);
            if ((j+1) % 3 == 0) {
                printf(" |");
            }
        }

        if ((i+1) % 3 == 0) {
            printf("\n");
            printf("--------+-------+--------");
        }
        printf("\n");
    }
}


/**
 * Function: write_to_file
 * -----------------------
 * Write the current state of the Sudoku grid to a file.
 * 
 * Parameters:
 * - sudoku: Pointer to the Sudoku structure.
 * - filename: Name of the file.
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