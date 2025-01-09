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
void parse_file(Sudoku *sudoku, const char *filename) {
    char file_str[1024]; // Increased buffer size to handle larger files

    // Open the file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    // Read the entire file
    size_t bytes_read = fread(file_str, sizeof(char), sizeof(file_str) - 1, file);
    file_str[bytes_read] = '\0'; // Null-terminate the buffer
    fclose(file);

    // Parse the file
    char *buffer = file_str;
    int i = 0, j = 0;

    while (*buffer) {
        // Skip white spaces
        while (isspace(*buffer)) buffer++;

        // Convert the string to an integer
        char *end;
        int num = strtol(buffer, &end, 10); // Use base 10 for integers

        // Validate and assign the number
        if (num < 0 || num > 9) {
            printf("Invalid number in file: %d\n", num);
            exit(1);
        }
        sudoku->table[i][j] = num;

        // Update indices
        j++;
        if (j == 9) { // Move to the next row after 9 numbers
            j = 0;
            i++;
        }
        if (i == 9) break; // Stop after filling 9 rows

        // Move to the next number
        buffer = end;
    }

    // Validate that the table is completely filled
    if (i != 9 || j != 0) {
        printf("Error: File does not contain a complete 9x9 grid.\n");
        exit(1);
    }
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