#include "io.h"
#include "human_solver.h"
#include "helpers.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N 9

// ---------------------------------------------------------------------------------------------------- //
// --- HELPER FUNCTIONS --- //

// Global candidates array
static unsigned short candidates[N][N];


/**
 * Function: digitMask
 * -------------------
 * Creates a bitmask for a specific candidate digit.
 * The bitmask represents the digit as a single bit set in the corresponding position.
 *
 * Parameters:
 * - digit: The digit to create the bitmask for.
 *
 * Returns:
 * - A bitmask representing the candidate digit.
 */
static unsigned short digitMask(int d) {
    
    return 1U << (d - 1);
}


/**
 * Function: maskHasDigit
 * ----------------------
 * Checks whether a given candidate digit is present in the candidate mask of a cell.
 * The candidate mask is a bitmask representing the possible digits for a Sudoku cell.
 *
 * Parameters:
 * - mask: The bitmask representing candidates for a cell.
 * - digit: The digit to check for presence in the mask.
 *
 * Returns:
 * - true if the digit is present in the mask, false otherwise.
 */
static bool maskHasDigit(unsigned short mask, int d) {
    return (mask & digitMask(d)) != 0;
}


void formatCandidates(char *buffer, unsigned short mask) {
    int pos = 0;
    for (int d = 1; d <= 9; d++) {
        if (maskHasDigit(mask, d)) {
            pos += sprintf(buffer + pos, "%d, ", d);
        }
    }
    buffer[pos-2] = '\0';
    // buffer[pos] = '\0'; // Null-terminate the string
}


/**
 * Function: bitCount
 * -------------------
 * Counts the number of set bits in a bitmask, which represents
 * the number of candidates for a Sudoku cell.
 *
 * Parameters:
 * - mask: The bitmask representing candidates for a cell.
 *
 * Returns:
 * - The number of set bits (candidates) in the bitmask.
 */
static int bitCount(unsigned short mask) {
    unsigned short localMask = mask; // Create a copy of the mask
    int count = 0;
    while (localMask) {
        count += (localMask & 1);
        localMask >>= 1;
    }
    return count;
}


/**
 * Function: setCell
 * ------------------
 * Updates a specific cell in the Sudoku grid with a given digit.
 * Adjusts the candidates of all cells in the same row, column, and box
 * to remove the placed digit.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 * - r: The row index of the cell to update.
 * - c: The column index of the cell to update.
 * - d: The digit to place in the cell.
 *
 * Returns:
 * - Nothing. Modifies the `sudoku` grid and the `candidates` array in place.
 */
static void setCell(Sudoku *sudoku, int r, int c, int d) {
    // printf("Setting cell (%d, %d) to %d\n", r, c, d);

    // Update the Sudoku grid.
    sudoku->table[r][c] = d;

    // Clear candidates for the current cell.
    candidates[r][c] = 0;

    // Remove digit `d` from candidates in the same row and column.
    for (int i = 0; i < N; i++) {
        if (candidates[r][i] & digitMask(d)) {
            candidates[r][i] &= ~digitMask(d);
            // printf("Removed %d from row candidate (%d, %d)\n", d, r, i);
        }
        if (candidates[i][c] & digitMask(d)) {
            candidates[i][c] &= ~digitMask(d);
            // printf("Removed %d from column candidate (%d, %d)\n", d, i, c);
        }
    }

    // Remove digit `d` from candidates in the same box.
    int boxRowStart = (r / 3) * 3;
    int boxColStart = (c / 3) * 3;
    for (int rr = boxRowStart; rr < boxRowStart + 3; rr++) {
        for (int cc = boxColStart; cc < boxColStart + 3; cc++) {
            if (candidates[rr][cc] & digitMask(d)) {
                candidates[rr][cc] &= ~digitMask(d);
                // printf("Removed %d from box candidate (%d, %d)\n", d, rr, cc);
            }
        }
    }
}


/**
 * Function: initCandidates
 * -------------------------
 * Initializes the `candidates` array for the Sudoku grid. For each empty cell,
 * all digits (1-9) are initially considered possible. The candidates for filled cells
 * are cleared, and the candidates for related cells are updated accordingly.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 *
 * Returns:
 * - Nothing. Initializes the global `candidates` array.
 */
static void initCandidates(Sudoku *sudoku) {
    // Reset all candidates to "all digits possible."
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            candidates[r][c] = 0x1FF; // All digits (1-9) are possible
        }
    }

    // Process the filled cells to update candidates.
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (sudoku->table[r][c] != 0) {
                int givenDigit = sudoku->table[r][c];
                // printf("Initializing given cell (%d, %d) with digit %d\n", r, c, givenDigit);
                setCell(sudoku, r, c, givenDigit);
            }
        }
    }
}



// ---------------------------------------------------------------------------------------------------- //
// --- HUMAN TECHNIQUES IMPLEMENTATION --- //


// -------------------------------------- //
// --- SINGLES --- //


/**
 * Function: applyNakedSingle
 * --------------------------
 * Identifies cells where only one candidate is possible ("naked single") and
 * fills them with that value. This technique simplifies the grid by directly
 * solving straightforward cells.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 * - stats: Pointer to the SolverStats structure for tracking the use of techniques.
 * - solving_mode: boolean flag indicating whether to record the moves made in a log file.
 *
 * Returns:
 * - true if progress is made on the puzzle, false otherwise.
 */
bool applyNakedSingle(Sudoku *sudoku, SolverStats *stats, bool solving_mode) {

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (sudoku->table[r][c] == 0) { // Empty cell
                unsigned short mask = candidates[r][c];
                if (bitCount(mask) == 1) { // Only one candidate
                    for (int d = 1; d <= 9; d++) {
                        if (maskHasDigit(mask, d)) {
                            setCell(sudoku, r, c, d);
                            stats->naked_single++;
                            if (solving_mode) {
                                FILE *logFile = fopen("solver_actions.log", "a");
                                if (logFile == NULL) {
                                    printf("Error opening log file.\n");
                                    return false; // Exit if the log file cannot be opened
                                }
                                fprintf(logFile, "Naked Single: Placing %d in cell (%d, %d)\n", d, r, c);
                                fclose(logFile); // Close the file before returning
                            }
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}


/**
 * Function: applyHiddenSingle
 * ---------------------------
 * Identifies cells where a candidate digit can only be placed in one specific
 * cell in a unit (row, column, or box). Fills that cell with the digit.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 * - stats: Pointer to the SolverStats structure for tracking the use of techniques.
 * - solving_mode: boolean flag indicating whether to record the moves made in a log file.
 *
 * Returns:
 * - true if progress is made on the puzzle, false otherwise.
 */
bool applyHiddenSingle(Sudoku *sudoku, SolverStats *stats, bool solving_mode) {

    // Check rows for hidden singles
    for (int r = 0; r < N; r++) {
        for (int d = 1; d <= 9; d++) {
            int count = 0, col = -1;
            for (int c = 0; c < N; c++) {
                if (sudoku->table[r][c] == 0 && maskHasDigit(candidates[r][c], d)) {
                    count++;
                    col = c;
                }
            }
            if (count == 1) { // Only one cell in row can take this digit
                setCell(sudoku, r, col, d);
                stats->hidden_single++;
                if (solving_mode) {
                    FILE *logFile = fopen("solver_actions.log", "a");
                    if (logFile == NULL) {
                        printf("Error opening log file.\n");
                        return false; // Exit if the log file cannot be opened
                    }
                    fprintf(logFile, "Hidden Single (Row): Placing %d in cell (%d, %d)\n", d, r, col);
                    fclose(logFile); // Close the file before returning
                }
                return true;
            }
        }
    }

    // Check columns for hidden singles
    for (int c = 0; c < N; c++) {
        for (int d = 1; d <= 9; d++) {
            int count = 0, row = -1;
            for (int r = 0; r < N; r++) {
                if (sudoku->table[r][c] == 0 && maskHasDigit(candidates[r][c], d)) {
                    count++;
                    row = r;
                }
            }
            if (count == 1) { // Only one cell in column can take this digit
                setCell(sudoku, row, c, d);
                stats->hidden_single++;
                if (solving_mode) {
                    FILE *logFile = fopen("solver_actions.log", "a");
                    if (logFile == NULL) {
                        printf("Error opening log file.\n");
                        return false; // Exit if the log file cannot be opened
                    }
                    fprintf(logFile, "Hidden Single (Column): Placing %d in cell (%d, %d)\n", d, row, c);
                    fclose(logFile); // Close the file before returning
                }
                return true;
            }
        }
    }

    // Check boxes for hidden singles
    for (int boxRow = 0; boxRow < 3; boxRow++) {
        for (int boxCol = 0; boxCol < 3; boxCol++) {
            for (int d = 1; d <= 9; d++) {
                int count = 0, row = -1, col = -1;
                for (int r = 0; r < 3; r++) {
                    for (int c = 0; c < 3; c++) {
                        int globalRow = boxRow * 3 + r;
                        int globalCol = boxCol * 3 + c;
                        if (sudoku->table[globalRow][globalCol] == 0 &&
                            maskHasDigit(candidates[globalRow][globalCol], d)) {
                            count++;
                            row = globalRow;
                            col = globalCol;
                        }
                    }
                }
                if (count == 1) { // Only one cell in box can take this digit
                    setCell(sudoku, row, col, d);
                    stats->hidden_single++;
                    if (solving_mode) {
                        FILE *logFile = fopen("solver_actions.log", "a");
                        if (logFile == NULL) {
                            printf("Error opening log file.\n");
                            return false; // Exit if the log file cannot be opened
                        }
                        fprintf(logFile, "Hidden Single (Box): Placing %d in cell (%d, %d)\n", d, row, col);
                        fclose(logFile); // Close the file before returning
                    }
                    return true;
                }
            }
        }
    }

    return false;
}


// -------------------------------------- //
// --- PAIRS --- //


/**
 * Function: isNakedPair
 * ---------------------
 * Checks if two cells share the same two candidates, which qualifies them as a "naked pair."
 *
 * Parameters:
 * - mask1: The candidate bitmask for the first cell.
 * - mask2: The candidate bitmask for the second cell.
 *
 * Returns:
 * - true if the cells form a naked pair, false otherwise.
 */
bool isNakedPair(unsigned short mask1, unsigned short mask2) {
    return (mask1 == mask2) && (bitCount(mask1) == 2);
}


/**
 * Function: applyNakedPair
 * ------------------------
 * Identifies pairs of cells in a unit (row, column, or box) that share exactly
 * the same two candidates. Removes these candidates from all other cells in the unit.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 * - stats: Pointer to the SolverStats structure for tracking the use of techniques.
 * - solving_mode: boolean flag indicating whether to record the moves made in a log file.
 *
 * Returns:
 * - true if progress is made on the puzzle, false otherwise.
 */
bool applyNakedPair(Sudoku *sudoku, SolverStats *stats, bool solving_mode) {

    bool progress = false;

    // Check rows for Naked Pairs
    for (int r = 0; r < N; r++) {
        for (int c1 = 0; c1 < N - 1; c1++) {
            if (sudoku->table[r][c1] == 0 && bitCount(candidates[r][c1]) == 2) {
                for (int c2 = c1 + 1; c2 < N; c2++) {
                    if (isNakedPair(candidates[r][c1], candidates[r][c2])) {
                        unsigned short pairMask = candidates[r][c1];

                        for (int c3 = 0; c3 < N; c3++) {
                            if (c3 != c1 && c3 != c2 && sudoku->table[r][c3] == 0) {
                                if (candidates[r][c3] & pairMask) {
                                    if (solving_mode) {
                                        FILE *logFile = fopen("solver_actions.log", "a");
                                        if (logFile == NULL) {
                                            printf("Error opening log file.\n");
                                            return false; // Exit if the log file cannot be opened
                                        }
                                        char candidatesStr[10];
                                        formatCandidates(candidatesStr, candidates[r][c3] & pairMask);
                                        fprintf(logFile, "Naked pair at cells (%d, %d) and (%d, %d): removing candidates %s from cell (%d, %d)\n", r, c1, r, c2, candidatesStr, r, c3);
                                        fclose(logFile); // Close the file before returning
                                    }
                                    candidates[r][c3] &= ~pairMask;
                                    progress = true;
                                }
                            }
                        }
                        if (progress) {
                            if (solving_mode) {
                                FILE *logFile = fopen("solver_actions.log", "a");
                                fprintf(logFile,"\n");
                                fclose(logFile);
                            }
                            stats->naked_pair++;
                            return progress;
                        }                        
                    }
                }
            }
        }
    }

    // Check columns for Naked Pairs
    for (int c = 0; c < N; c++) {
        for (int r1 = 0; r1 < N - 1; r1++) {
            if (sudoku->table[r1][c] == 0 && bitCount(candidates[r1][c]) == 2) {
                for (int r2 = r1 + 1; r2 < N; r2++) {
                    if (isNakedPair(candidates[r1][c], candidates[r2][c])) {
                        unsigned short pairMask = candidates[r1][c];

                        for (int r3 = 0; r3 < N; r3++) {
                            if (r3 != r1 && r3 != r2 && sudoku->table[r3][c] == 0) {
                                if (candidates[r3][c] & pairMask) {
                                    if (solving_mode) {
                                        FILE *logFile = fopen("solver_actions.log", "a");
                                        if (logFile == NULL) {
                                            printf("Error opening log file.\n");
                                            return false; // Exit if the log file cannot be opened
                                        }
                                        char candidatesStr[10];
                                        formatCandidates(candidatesStr, candidates[r3][c] & pairMask);
                                        fprintf(logFile, "Naked pair at cells (%d, %d) and (%d, %d): removing candidates %s from cell (%d, %d)\n", r1, c, r2, c, candidatesStr, r3, c);
                                        fclose(logFile); // Close the file before returning
                                    }                                
                                    candidates[r3][c] &= ~pairMask;
                                    progress = true;
                                }
                            }
                        }
                        if (progress) {
                            if (solving_mode) {
                                FILE *logFile = fopen("solver_actions.log", "a");
                                fprintf(logFile,"\n");
                                fclose(logFile);
                            }
                            stats->naked_pair++;
                            return progress;
                        }
                    }
                }
            }
        }
    }

    // Check boxes for Naked Pairs
    for (int boxRow = 0; boxRow < 3; boxRow++) {
        for (int boxCol = 0; boxCol < 3; boxCol++) {
            int cells[9][2], count = 0;

            // Collect cells with exactly two candidates in the current box
            for (int r = 0; r < 3; r++) {
                for (int c = 0; c < 3; c++) {
                    int globalRow = boxRow * 3 + r;
                    int globalCol = boxCol * 3 + c;
                    if (sudoku->table[globalRow][globalCol] == 0 &&
                        bitCount(candidates[globalRow][globalCol]) == 2) {
                        cells[count][0] = globalRow;
                        cells[count][1] = globalCol;
                        count++;
                    }
                }
            }

            for (int i = 0; i < count - 1; i++) {
                for (int j = i + 1; j < count; j++) {
                    if (isNakedPair(candidates[cells[i][0]][cells[i][1]],
                                    candidates[cells[j][0]][cells[j][1]])) {
                        unsigned short pairMask = candidates[cells[i][0]][cells[i][1]];
                        

                        for (int k = 0; k < count; k++) {
                            if (k != i && k != j) {
                                int r = cells[k][0], c = cells[k][1];
                                if (candidates[r][c] & pairMask) {
                                    if (solving_mode) {
                                        FILE *logFile = fopen("solver_actions.log", "a");
                                        if (logFile == NULL) {
                                            printf("Error opening log file.\n");
                                            return false; // Exit if the log file cannot be opened
                                        }
                                        char candidatesStr[10];
                                        formatCandidates(candidatesStr, candidates[r][c] & pairMask);
                                        fprintf(logFile, "Naked pair at cells (%d, %d) and (%d, %d): removing candidates %s from cell (%d, %d)\n", cells[i][0], cells[i][1], cells[j][0], cells[j][1], candidatesStr, r, c);
                                        fclose(logFile); // Close the file before returning
                                    }                                  
                                    candidates[r][c] &= ~pairMask;
                                    progress = true;
                                }
                            }
                        }
                        if (progress) {
                            if (solving_mode) {
                                FILE *logFile = fopen("solver_actions.log", "a");
                                fprintf(logFile,"\n");
                                fclose(logFile);
                            }
                            stats->naked_pair++;
                            return progress;
                        }
                    }
                }
            }
        }
    }

    return progress;
}


/**
 * Function: isHiddenPair
 * -----------------------
 * Checks if two candidates appear exclusively in two cells within a unit (row, column, or box).
 *
 * Parameters:
 * - mask1: The candidate bitmask for the first cell.
 * - mask2: The candidate bitmask for the second cell.
 *
 * Returns:
 * - true if the candidates form a hidden pair, false otherwise.
 */
bool isHiddenPair(unsigned short mask1, unsigned short mask2) {
    unsigned short pairMask = mask1 & mask2;
    return bitCount(pairMask) == 2 && (bitCount(mask1) > 2 || bitCount(mask2) > 2);
}


/**
 * Function: removeOtherCandidates
 * -------------------------------
 * Removes all candidates from a cell's candidate mask except for the candidates
 * specified in the target mask.
 *
 * Parameters:
 * - cellMask: Pointer to the bitmask representing candidates for a cell.
 * - targetMask: The bitmask representing the candidates to retain.
 *
 * Returns:
 * - Nothing. Modifies cellMask in place.
 */
void removeOtherCandidates(unsigned short *cellMask, unsigned short pairMask) {
    *cellMask &= pairMask; // Remove all candidates except the pair
}


/**
 * Function: isUniqueToPair
 * -------------------------
 * Determines if a candidate digit is unique to two specific cells in a unit
 * (row, column, or box).
 *
 * Parameters:
 * - d: The candidate digit to check.
 * - unitCells: An array of all cells in the unit.
 * - pairCells: The two cells that are being checked as a pair.
 * - unitSize: The total number of cells in the unit.
 *
 * Returns:
 * - true if the digit is unique to the two cells, false otherwise.
 */
bool isUniqueToPair(int d, int unitCells[9][2], int pairCells[2][2], int unitSize) {
    for (int i = 0; i < unitSize; i++) {
        int r = unitCells[i][0], c = unitCells[i][1];
        if ((r != pairCells[0][0] || c != pairCells[0][1]) && 
            (r != pairCells[1][0] || c != pairCells[1][1]) && 
            maskHasDigit(candidates[r][c], d)) {
            return false; // Candidate `d` is present in another cell in the unit
        }
    }
    return true;
}


/**
 * Function: applyHiddenPair
 * -------------------------
 * Identifies pairs of candidates that appear only in two cells of a unit
 * (row, column, or box). Retains only these two candidates in the identified cells
 * and removes all others.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 * - stats: Pointer to the SolverStats structure for tracking the use of techniques.
 * - solving_mode: boolean flag indicating whether to record the moves made in a log file.
 *
 * Returns:
 * - true if progress is made on the puzzle, false otherwise.
 */
bool applyHiddenPair(Sudoku *sudoku, SolverStats *stats, bool solving_mode) {

    bool progress = false;

    // Iterate over rows, columns, and boxes
    for (int unitType = 0; unitType < 3; unitType++) { // 0 = row, 1 = column, 2 = box
        for (int i = 0; i < N; i++) {
            int unitCells[9][2], unitSize = 0;

            // Collect all cells in the current unit
            for (int j = 0; j < N; j++) {
                int r, c;
                if (unitType == 0) { // Row
                    r = i, c = j;
                } else if (unitType == 1) { // Column
                    r = j, c = i;
                } else { // Box
                    r = (i / 3) * 3 + j / 3;
                    c = (i % 3) * 3 + j % 3;
                }
                if (sudoku->table[r][c] == 0) {
                    unitCells[unitSize][0] = r;
                    unitCells[unitSize][1] = c;
                    unitSize++;
                }
            }

            // Iterate over all pairs of candidates
            for (int d1 = 1; d1 <= 9; d1++) {
                for (int d2 = d1 + 1; d2 <= 9; d2++) {
                    unsigned short pairMask = digitMask(d1) | digitMask(d2);
                    int pairCells[2][2], pairCount = 0;

                    // Find cells that match the pair mask
                    for (int j = 0; j < unitSize; j++) {
                        int r = unitCells[j][0], c = unitCells[j][1];
                        if ((candidates[r][c] & pairMask) == pairMask) {
                            if (pairCount < 2) {
                                pairCells[pairCount][0] = r;
                                pairCells[pairCount][1] = c;
                            }
                            pairCount++;
                        }
                    }

                    // Validate if it forms a hidden pair
                    if (pairCount == 2 &&
                        isUniqueToPair(d1, unitCells, pairCells, unitSize) &&
                        isUniqueToPair(d2, unitCells, pairCells, unitSize)) {
                        
                        // Check if the candidate masks are already reduced
                        int r1 = pairCells[0][0], c1 = pairCells[0][1];
                        int r2 = pairCells[1][0], c2 = pairCells[1][1];

                        if (candidates[r1][c1] == pairMask && candidates[r2][c2] == pairMask) {
                            continue; // Skip if no changes are needed
                        }

                        // Remove other candidates from the pair cells
                        removeOtherCandidates(&candidates[r1][c1], pairMask);
                        removeOtherCandidates(&candidates[r2][c2], pairMask);

                        if (solving_mode) {
                            FILE *logFile = fopen("solver_actions.log", "a");
                            if (logFile == NULL) {
                                printf("Error opening log file.\n");
                                return false; // Exit if the log file cannot be opened
                            }
                            fprintf(logFile, "Hidden Pair: [%d, %d] in cells (%d, %d) and (%d, %d), cleared other candidates in these cells.\n", d1, d2, r1, c1, r2, c2);
                            fclose(logFile); // Close the file before returning
                        }
                        progress = true;
                        stats->hidden_pair++;
                        return progress; // Exit after finding one hidden pair
                    }
                }
            }
        }
    }

    return progress;
}


/**
 * Function: applyPointingPair
 * ----------------------------
 * Identifies "pointing pairs," which divides in two cases: (1) a candidate appears 
 * only in two cells of a box that are aligned in a row or column. Removes this
 * candidate from other cells in the same row or column outside the box. (2) A candidate
 * appears only in two cells of a row or column, and they are in the same box. Remove
 * this candidate from other cells in the same box as the pair 
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 * - stats: Pointer to the SolverStats structure for tracking the use of techniques.
 * - solving_mode: boolean flag indicating whether to record the moves made in a log file.
 *
 * Returns:
 * - true if progress is made on the puzzle, false otherwise.
 */
bool applyPointingPair(Sudoku *sudoku, SolverStats *stats, bool solving_mode) {

    bool progress = false;

    // Iterate through all boxes
    for (int boxRow = 0; boxRow < 3; boxRow++) {
        for (int boxCol = 0; boxCol < 3; boxCol++) {
            int boxStartRow = boxRow * 3;
            int boxStartCol = boxCol * 3;

            // Iterate through all cells in the current box
            for (int r = boxStartRow; r < boxStartRow + 3; r++) {
                for (int c = boxStartCol; c < boxStartCol + 3; c++) {
                    if (sudoku->table[r][c] != 0) continue; // Skip filled cells

                    // Check each candidate for the current cell
                    for (int d = 1; d <= 9; d++) {
                        if (!maskHasDigit(candidates[r][c], d)) continue;

                        bool rowPairFound = false, colPairFound = false;
                        int otherRow = -1, otherCol = -1;

                        // Check for a pair in the row
                        for (int cc = boxStartCol; cc < boxStartCol + 3; cc++) {
                            if (cc != c && maskHasDigit(candidates[r][cc], d)) {
                                if (!rowPairFound) {
                                    rowPairFound = true;
                                    otherRow = r;
                                    otherCol = cc;
                                } else {
                                    rowPairFound = false; // More than two cells with the candidate in the row
                                    break;
                                }
                            }
                        }

                        if (rowPairFound) {
                            bool candidateInBox = false;
                            for (int rr = boxStartRow; rr < boxStartRow + 3; rr++) {
                                for (int cc = boxStartCol; cc < boxStartCol + 3; cc++) {
                                    if ((rr != r || cc != c) && (rr != otherRow || cc != otherCol) &&
                                        maskHasDigit(candidates[rr][cc], d)) {
                                        candidateInBox = true;
                                        break;
                                    }
                                }
                                if (candidateInBox) break;
                            }

                            if (!candidateInBox) { // Apply outside-the-box removal
                                for (int cc = 0; cc < N; cc++) {
                                    if (cc < boxStartCol || cc >= boxStartCol + 3) {
                                        if (maskHasDigit(candidates[r][cc], d)) {
                                            candidates[r][cc] &= ~digitMask(d);
                                            progress = true;
                                            if (solving_mode) {
                                                FILE *logFile = fopen("solver_actions.log", "a");
                                                if (logFile == NULL) {
                                                    printf("Error opening log file.\n");
                                                    return false; // Exit if the log file cannot be opened
                                                }
                                                fprintf(logFile, "Pointing Pair (Row Outside Box) at cells (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow, otherCol, d, r, cc);
                                                fclose(logFile); // Close the file before returning
                                            }
                                        }
                                    }
                                }
                                if (progress) {
                                    if (solving_mode) {
                                        FILE *logFile = fopen("solver_actions.log", "a");
                                        fprintf(logFile,"\n");
                                        fclose(logFile);
                                    }
                                    stats->pointing_pair++;
                                    return progress;
                                }
                            }
                            // Inside-the-box scenario
                            bool candidateOutsideBox = false;
                            for (int outsideCol = 0; outsideCol < N; outsideCol++) {
                                if (outsideCol < boxStartCol || outsideCol >= boxStartCol + 3) {
                                    if (maskHasDigit(candidates[r][outsideCol], d)) {
                                        candidateOutsideBox = true;
                                        break;
                                    }
                                }
                            }

                            if (!candidateOutsideBox) { // Apply inside-the-box removal
                                for (int innerR = boxStartRow; innerR < boxStartRow + 3; innerR++) {
                                    for (int innerC = boxStartCol; innerC < boxStartCol + 3; innerC++) {
                                        if (innerR != r && maskHasDigit(candidates[innerR][innerC], d)) {
                                            candidates[innerR][innerC] &= ~digitMask(d);
                                            progress = true;
                                            if (solving_mode) {
                                                FILE *logFile = fopen("solver_actions.log", "a");
                                                if (logFile == NULL) {
                                                    printf("Error opening log file.\n");
                                                    return false; // Exit if the log file cannot be opened
                                                }
                                                fprintf(logFile, "Pointing Pair (Row Inside Box) at cells (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow, otherCol, d, innerR, innerC);
                                                fclose(logFile); // Close the file before returning
                                            }
                                        }
                                    }
                                }
                                if (progress) {
                                    if (solving_mode) {
                                        FILE *logFile = fopen("solver_actions.log", "a");
                                        fprintf(logFile,"\n");
                                        fclose(logFile);
                                    }
                                    stats->pointing_pair++;
                                    return progress;
                                }                        
                            }
                        }

                        // Check for a pair in the column
                        for (int rr = boxStartRow; rr < boxStartRow + 3; rr++) {
                            if (rr != r && maskHasDigit(candidates[rr][c], d)) {
                                if (!colPairFound) {
                                    colPairFound = true;
                                    otherRow = rr;
                                    otherCol = c;
                                } else {
                                    colPairFound = false; // More than two cells with the candidate in the column
                                    break;
                                }
                            }
                        }

                        if (colPairFound) {

                            bool candidateInBox = false;
                            for (int rr = boxStartRow; rr < boxStartRow + 3; rr++) {
                                for (int cc = boxStartCol; cc < boxStartCol + 3; cc++) {
                                    if ((rr != r || cc != c) && (rr != otherRow || cc != otherCol) &&
                                        maskHasDigit(candidates[rr][cc], d)) {
                                        candidateInBox = true;
                                        break;
                                    }
                                }
                                if (candidateInBox) break;
                            }

                            if (!candidateInBox) { // Apply outside-the-box removal
                                for (int rr = 0; rr < N; rr++) {
                                    if (rr < boxStartRow || rr >= boxStartRow + 3) {
                                        if (maskHasDigit(candidates[rr][c], d)) {
                                            candidates[rr][c] &= ~digitMask(d);
                                            progress = true;
                                            if (solving_mode) {
                                                FILE *logFile = fopen("solver_actions.log", "a");
                                                if (logFile == NULL) {
                                                    printf("Error opening log file.\n");
                                                    return false; // Exit if the log file cannot be opened
                                                }
                                                fprintf(logFile, "Pointing Pair (Col Outside Box) at cells (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow, otherCol, d, rr, c);
                                                fclose(logFile); // Close the file before returning
                                            }
                                        }
                                    }
                                }
                                if (progress) {
                                    FILE *logFile = fopen("solver_actions.log", "a");
                                    fprintf(logFile,"\n");
                                    fclose(logFile);
                                    stats->pointing_pair++;
                                    return progress;
                                }
                            }
                            // Inside-the-box scenario
                            bool candidateOutsideBox = false;
                            for (int outsideRow = 0; outsideRow < N; outsideRow++) {
                                if (outsideRow < boxStartRow || outsideRow >= boxStartRow + 3) {
                                    if (maskHasDigit(candidates[outsideRow][c], d)) {
                                        candidateOutsideBox = true;
                                        break;
                                    }
                                }
                            }

                            if (!candidateOutsideBox) { // Apply inside-the-box removal
                                for (int innerR = boxStartRow; innerR < boxStartRow + 3; innerR++) {
                                    for (int innerC = boxStartCol; innerC < boxStartCol + 3; innerC++) {
                                        if (innerC != c && maskHasDigit(candidates[innerR][innerC], d)) {
                                            candidates[innerR][innerC] &= ~digitMask(d);
                                            progress = true;
                                            if (solving_mode) {
                                                FILE *logFile = fopen("solver_actions.log", "a");
                                                if (logFile == NULL) {
                                                    printf("Error opening log file.\n");
                                                    return false; // Exit if the log file cannot be opened
                                                }
                                                fprintf(logFile,"Pointing Pair (Col Inside Box) at cells (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow, otherCol, d, innerR, innerC);
                                                fclose(logFile); // Close the file before returning
                                            }
                                        }
                                    }
                                }
                                if (progress) {
                                    if (solving_mode) {
                                        FILE *logFile = fopen("solver_actions.log", "a");
                                        fprintf(logFile,"\n");
                                        fclose(logFile);
                                    }
                                    stats->pointing_pair++;
                                    return progress;
                                }                        
                            }
                        }
                    }
                }
            }
        }
    }

    return progress;
}


// -------------------------------------- //
// --- TRIPLES --- //


/**
 * Function: isNakedTriple
 * -----------------------
 * Checks if three cells together form a "naked triple," where the union
 * of their candidates consists of exactly three unique digits.
 *
 * Parameters:
 * - mask1: The candidate bitmask for the first cell.
 * - mask2: The candidate bitmask for the second cell.
 * - mask3: The candidate bitmask for the third cell.
 *
 * Returns:
 * - true if the cells form a naked triple, false otherwise.
 */
bool isNakedTriple(unsigned short mask1, unsigned short mask2, unsigned short mask3) {
    unsigned short combinedMask = mask1 | mask2 | mask3;
    return bitCount(combinedMask) == 3; // The combined mask must contain exactly 3 candidates
}


/**
 * Function: applyNakedTriple
 * --------------------------
 * Identifies three cells in a unit (row, column, or box) where the combined
 * candidates of these cells consist of exactly three unique digits. Removes
 * these digits from all other cells in the unit.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 * - stats: Pointer to the SolverStats structure for tracking the use of techniques.
 * - solving_mode: boolean flag indicating whether to record the moves made in a log file.
 *
 * Returns:
 * - true if progress is made on the puzzle, false otherwise.
 */
bool applyNakedTriple(Sudoku *sudoku, SolverStats *stats, bool solving_mode) {
    bool progress = false;

    // Check rows for Naked Triples
    for (int r = 0; r < N; r++) {
        for (int c1 = 0; c1 < N - 2; c1++) {
            if (sudoku->table[r][c1] == 0) {
                for (int c2 = c1 + 1; c2 < N - 1; c2++) {
                    if (sudoku->table[r][c2] == 0) {
                        for (int c3 = c2 + 1; c3 < N; c3++) {
                            if (sudoku->table[r][c3] == 0) {
                                if (isNakedTriple(candidates[r][c1], candidates[r][c2], candidates[r][c3])) {
                                    unsigned short tripleMask = candidates[r][c1] | candidates[r][c2] | candidates[r][c3];

                                    for (int c = 0; c < N; c++) {
                                        if (c != c1 && c != c2 && c != c3 && sudoku->table[r][c] == 0) {
                                            if (candidates[r][c] & tripleMask) {
                                                if (solving_mode) {
                                                    FILE *logFile = fopen("solver_actions.log", "a");
                                                    if (logFile == NULL) {
                                                        printf("Error opening log file.\n");
                                                        return false; // Exit if the log file cannot be opened
                                                    }
                                                    char candidatesStr[10];
                                                    formatCandidates(candidatesStr, candidates[r][c] & tripleMask);
                                                    fprintf(logFile, "Naked triple at cells (%d, %d), (%d, %d) and (%d, %d): removing candidates %s from cell (%d, %d)\n", r, c1, r, c2, r, c3, candidatesStr, r, c);
                                                    fclose(logFile); // Close the file before returning
                                                }   

                                                candidates[r][c] &= ~tripleMask;
                                                progress = true;
                                            }
                                        }
                                    }
                                    if (progress){
                                        if (solving_mode) {
                                            FILE *logFile = fopen("solver_actions.log", "a");
                                            fprintf(logFile,"\n");
                                            fclose(logFile);
                                        }
                                        stats->naked_triple++;
                                        return progress;
                                    }                                    
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Check columns for Naked Triples
    for (int c = 0; c < N; c++) {
        for (int r1 = 0; r1 < N - 2; r1++) {
            if (sudoku->table[r1][c] == 0) {
                for (int r2 = r1 + 1; r2 < N - 1; r2++) {
                    if (sudoku->table[r2][c] == 0) {
                        for (int r3 = r2 + 1; r3 < N; r3++) {
                            if (sudoku->table[r3][c] == 0) {
                                if (isNakedTriple(candidates[r1][c], candidates[r2][c], candidates[r3][c])) {
                                    unsigned short tripleMask = candidates[r1][c] | candidates[r2][c] | candidates[r3][c];

                                    for (int r = 0; r < N; r++) {
                                        if (r != r1 && r != r2 && r != r3 && sudoku->table[r][c] == 0) {
                                            if (candidates[r][c] & tripleMask) {
                                                if (solving_mode) {
                                                    FILE *logFile = fopen("solver_actions.log", "a");
                                                    if (logFile == NULL) {
                                                        printf("Error opening log file.\n");
                                                        return false; // Exit if the log file cannot be opened
                                                    }
                                                    char candidatesStr[10];
                                                    formatCandidates(candidatesStr, candidates[r][c] & tripleMask);
                                                    fprintf(logFile, "Naked triple at cells (%d, %d), (%d, %d) and (%d, %d): removing candidates %s from cell (%d, %d)\n", r1, c, r2, c, r3, c, candidatesStr, r, c);
                                                    fclose(logFile); // Close the file before returning
                                                }   

                                                candidates[r][c] &= ~tripleMask;
                                                progress = true;
                                            }
                                        }
                                    }
                                    if (progress){
                                        if (solving_mode) {
                                            FILE *logFile = fopen("solver_actions.log", "a");
                                            fprintf(logFile,"\n");
                                            fclose(logFile);
                                        }
                                        stats->naked_triple++;
                                        return progress;
                                    }                                    
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Check boxes for Naked Triples
    for (int boxRow = 0; boxRow < 3; boxRow++) {
        for (int boxCol = 0; boxCol < 3; boxCol++) {
            int cells[9][2], count = 0;

            // Collect all cells with candidates in the box
            for (int r = 0; r < 3; r++) {
                for (int c = 0; c < 3; c++) {
                    int globalRow = boxRow * 3 + r;
                    int globalCol = boxCol * 3 + c;
                    if (sudoku->table[globalRow][globalCol] == 0) {
                        cells[count][0] = globalRow;
                        cells[count][1] = globalCol;
                        count++;
                    }
                }
            }

            for (int i = 0; i < count - 2; i++) {
                for (int j = i + 1; j < count - 1; j++) {
                    for (int k = j + 1; k < count; k++) {
                        if (isNakedTriple(candidates[cells[i][0]][cells[i][1]],
                                          candidates[cells[j][0]][cells[j][1]],
                                          candidates[cells[k][0]][cells[k][1]])) {
                            unsigned short tripleMask = candidates[cells[i][0]][cells[i][1]] |
                                                        candidates[cells[j][0]][cells[j][1]] |
                                                        candidates[cells[k][0]][cells[k][1]];

                            for (int m = 0; m < count; m++) {
                                if (m != i && m != j && m != k) {
                                    int r = cells[m][0], c = cells[m][1];
                                    if (candidates[r][c] & tripleMask) {
                                        if (solving_mode) {
                                            FILE *logFile = fopen("solver_actions.log", "a");
                                            if (logFile == NULL) {
                                                printf("Error opening log file.\n");
                                                return false; // Exit if the log file cannot be opened
                                            }
                                            char candidatesStr[10];
                                            formatCandidates(candidatesStr, candidates[r][c] & tripleMask);
                                            fprintf(logFile, "Naked triple at cells (%d, %d), (%d, %d) and (%d, %d): removing candidates %s from cell (%d, %d)\n", 
                                                        cells[i][0], cells[i][1], cells[j][0], cells[j][1], cells[k][0], cells[k][1], candidatesStr, r, c);
                                            fclose(logFile); // Close the file before returning
                                        }

                                        candidates[r][c] &= ~tripleMask;
                                        progress = true;
                                    }
                                }
                            }
                            if (progress){
                                if (solving_mode) {
                                    FILE *logFile = fopen("solver_actions.log", "a");
                                    fprintf(logFile,"\n");
                                    fclose(logFile);
                                }
                                stats->naked_triple++;
                                return progress;
                            }
                        }
                    }
                }
            }
        }
    }

    return progress;
}


/**
 * Function: isHiddenTriple
 * ------------------------
 * Checks if three candidates are exclusive to three specific cells within a unit
 * (row, column, or box).
 *
 * Parameters:
 * - mask1: The candidate bitmask for the first cell.
 * - mask2: The candidate bitmask for the second cell.
 * - mask3: The candidate bitmask for the third cell.
 *
 * Returns:
 * - true if the candidates form a hidden triple, false otherwise.
 */
bool isHiddenTriple(unsigned short mask1, unsigned short mask2, unsigned short mask3) {
    unsigned short combinedMask = mask1 | mask2 | mask3;
    return bitCount(combinedMask) == 3; // The combined mask must contain exactly 3 candidates
}


/**
 * Function: removeOtherCandidatesForTriple
 * ----------------------------------------
 * Removes all candidates from a cell's candidate mask except for the candidates
 * specified in the triple mask.
 *
 * Parameters:
 * - cellMask: Pointer to the candidate bitmask for the cell.
 * - tripleMask: The bitmask representing the candidates to retain.
 *
 * Returns:
 * - Nothing. Modifies `cellMask` in place.
 */
void removeOtherCandidatesForTriple(unsigned short *cellMask, unsigned short tripleMask) {
    unsigned short originalMask = *cellMask;
    *cellMask &= tripleMask; // Keep only the candidates in the triple mask
}


/**
 * Function: applyHiddenTriple
 * ---------------------------
 * Identifies three candidates that appear only in three cells of a unit
 * (row, column, or box). Retains only these three candidates in the identified cells
 * and removes all others.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 * - stats: Pointer to the SolverStats structure for tracking the use of techniques.
 * - solving_mode: boolean flag indicating whether to record the moves made in a log file.
 *
 * Returns:
 * - true if progress is made on the puzzle, false otherwise.
 */
bool applyHiddenTriple(Sudoku *sudoku, SolverStats *stats, bool solving_mode) {

    bool progress = false;

    // Iterate over rows, columns, and boxes
    for (int unitType = 0; unitType < 3; unitType++) { // 0 = row, 1 = column, 2 = box
        for (int i = 0; i < N; i++) {
            int unitCells[9][2], unitSize = 0;

            // Collect all cells in the current unit
            for (int j = 0; j < N; j++) {
                int r, c;
                if (unitType == 0) { // Row
                    r = i, c = j;
                } else if (unitType == 1) { // Column
                    r = j, c = i;
                } else { // Box
                    r = (i / 3) * 3 + j / 3;
                    c = (i % 3) * 3 + j % 3;
                }
                if (sudoku->table[r][c] == 0) {
                    unitCells[unitSize][0] = r;
                    unitCells[unitSize][1] = c;
                    unitSize++;
                }
            }

            // Iterate over all triples of candidates
            for (int d1 = 1; d1 <= 9; d1++) {
                for (int d2 = d1 + 1; d2 <= 9; d2++) {
                    for (int d3 = d2 + 1; d3 <= 9; d3++) {
                        unsigned short tripleMask = digitMask(d1) | digitMask(d2) | digitMask(d3);
                        int tripleCells[3][2], tripleCount = 0;

                        // Find cells that contain any of the candidates in the triple mask
                        for (int j = 0; j < unitSize; j++) {
                            int r = unitCells[j][0], c = unitCells[j][1];
                            if ((candidates[r][c] & tripleMask) > 0) { // At least one of the candidates is present
                                if (tripleCount < 3) {
                                    tripleCells[tripleCount][0] = r;
                                    tripleCells[tripleCount][1] = c;
                                }
                                tripleCount++;
                            }
                        }

                        // Check if the candidates are exclusive to exactly three cells
                        if (tripleCount == 3) {

                            int r1 = tripleCells[0][0], c1 = tripleCells[0][1];
                            int r2 = tripleCells[1][0], c2 = tripleCells[1][1];
                            int r3 = tripleCells[2][0], c3 = tripleCells[2][1];

                            // Ensure the tripleMask is valid (only includes the hidden triple candidates)
                            unsigned short actualTripleMask =
                                (candidates[r1][c1] | candidates[r2][c2] | candidates[r3][c3]) &
                                tripleMask;

                            if (bitCount(actualTripleMask) != 3) {
                                continue;
                            }

                            // Remove other candidates from the triple cells
                            for (int k = 0; k < 3; k++) {
                                int r = tripleCells[k][0], c = tripleCells[k][1];
                                unsigned short oldMask = candidates[r][c];
                                removeOtherCandidatesForTriple(&candidates[r][c], actualTripleMask);

                                if (oldMask != candidates[r][c]) { // Only count if there's a change
                                    progress = true;
                                }
                            }

                            if (progress) {
                                if (solving_mode) {
                                    FILE *logFile = fopen("solver_actions.log", "a");
                                    if (logFile == NULL) {
                                        printf("Error opening log file.\n");
                                        return false; // Exit if the log file cannot be opened
                                    }
                                    fprintf(logFile,"Hidden Triple: [%d, %d, %d] in cells (%d, %d), (%d, %d) and (%d, %d), Cleared other candidates\n", d1, d2, d3, r1, c1, r2, c2, r3, c3);
                                    fprintf(logFile,"\n");
                                    fclose(logFile); // Close the file before returning
                                }
                                stats->hidden_triple++;
                                return progress;
                            }
                        }
                    }
                }
            }
        }
    }

    return progress;
}


/**
 * Function: applyPointingTriple
 * -----------------------------
 * Identifies "pointing triples," where a candidate appears only in three cells
 * of a box that are aligned in a row or column. Removes this candidate from other
 * cells in the same row or column outside the box.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 * - stats: Pointer to the SolverStats structure for tracking the use of techniques.
 * - solving_mode: boolean flag indicating whether to record the moves made in a log file.
 *
 * Returns:
 * - true if progress is made on the puzzle, false otherwise.
 */
bool applyPointingTriples(Sudoku *sudoku, SolverStats *stats, bool solving_mode) {

    bool progress = false;

    // Iterate through all boxes
    for (int boxRow = 0; boxRow < 3; boxRow++) {
        for (int boxCol = 0; boxCol < 3; boxCol++) {
            int boxStartRow = boxRow * 3;
            int boxStartCol = boxCol * 3;

            // Iterate through all cells in the current box
            for (int r = boxStartRow; r < boxStartRow + 3; r++) {
                for (int c = boxStartCol; c < boxStartCol + 3; c++) {
                    if (sudoku->table[r][c] != 0) continue; // Skip filled cells

                    // Check each candidate for the current cell
                    for (int d = 1; d <= 9; d++) {
                        if (!maskHasDigit(candidates[r][c], d)) continue;

                        // Check if there is another cell in the same row/column within the box having this candidate
                        bool tripleFound = false;
                        int otherRow1 = -1, otherCol1 = -1;
                        int otherRow2 = -1, otherCol2 = -1;

                        for (int rr = boxStartRow; rr < boxStartRow + 3; rr++) {
                            int match_found_row = 0;
                            for (int cc = boxStartCol; cc < boxStartCol + 3; cc++) {
                                if ((rr != r || cc != c) && maskHasDigit(candidates[rr][cc], d)) {
                                    if (rr == r) {
                                        if (match_found_row == 0) {
                                            otherRow1 = rr;
                                            otherCol1 = cc;
                                        } else if (match_found_row == 1) {
                                            otherRow2 = rr;
                                            otherCol2 = cc;
                                        }
                                        match_found_row++;
                                    }
                                }
                            }
                            if (match_found_row == 2) {
                                tripleFound = true;
                                break;
                            }
                        }
                        if (!tripleFound) {
                            for (int ccc = boxStartCol; ccc < boxStartCol + 3; ccc++) {
                                int match_found_col = 0;
                                for (int rrr = boxStartRow; rrr < boxStartRow + 3; rrr++) {
                                    if ((rrr != r) && maskHasDigit(candidates[rrr][ccc], d)) {
                                        if (ccc == c) {
                                            if (match_found_col == 0) {
                                                otherRow1 = rrr;
                                                otherCol1 = ccc;
                                            } else if (match_found_col == 1) {
                                                otherRow2 = rrr;
                                                otherCol2 = ccc;
                                            }
                                            match_found_col++;
                                        }
                                    }
                                }
                                if (match_found_col == 2) {
                                    tripleFound = true;
                                    break;
                                }
                            }
                        }

                        if (!tripleFound) continue;

                        // Outside-the-box scenario
                        bool candidateInBox = false;
                        for (int rr = boxStartRow; rr < boxStartRow + 3; rr++) {
                            for (int cc = boxStartCol; cc < boxStartCol + 3; cc++) {
                                if ((rr != r || cc != c) && (rr != otherRow1 || cc != otherCol1) && (rr != otherRow2 || cc != otherCol2) && maskHasDigit(candidates[rr][cc], d)) {
                                    candidateInBox = true;
                                    break;
                                }
                            }
                            if (candidateInBox) break;
                        }

                        if (!candidateInBox) { // Apply outside-the-box removal
                            if (r == otherRow1) { // Row-aligned
                                for (int cc = 0; cc < N; cc++) {
                                    if (cc < boxStartCol || cc >= boxStartCol + 3) {
                                        if (maskHasDigit(candidates[r][cc], d)) {
                                            candidates[r][cc] &= ~digitMask(d);
                                            progress = true;
                                            if (solving_mode) {
                                                FILE *logFile = fopen("solver_actions.log", "a");
                                                if (logFile == NULL) {
                                                    printf("Error opening log file.\n");
                                                    return false; // Exit if the log file cannot be opened
                                                }
                                                fprintf(logFile,"Pointing Triple (Outside Box) at cells (%d, %d), (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow1, otherCol1, otherRow2, otherCol2, d, r, cc);
                                                fclose(logFile); // Close the file before returning
                                            }
                                        }
                                    }
                                } 
                                if (progress) {
                                    FILE *logFile = fopen("solver_actions.log", "a");
                                    fprintf(logFile,"\n");
                                    fclose(logFile);
                                    stats->pointing_triple++;
                                    return progress;
                                }
                            } else if (c == otherCol1) { // Column-aligned
                                for (int rr = 0; rr < N; rr++) {
                                    if (rr < boxStartRow || rr >= boxStartRow + 3) {
                                        if (maskHasDigit(candidates[rr][c], d)) {
                                            candidates[rr][c] &= ~digitMask(d);
                                            progress = true;
                                            if (solving_mode) {
                                                FILE *logFile = fopen("solver_actions.log", "a");
                                                if (logFile == NULL) {
                                                    printf("Error opening log file.\n");
                                                    return false; // Exit if the log file cannot be opened
                                                }
                                                fprintf(logFile,"Pointing Triple (Outside Box) at cells (%d, %d), (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow1, otherCol1, otherRow2, otherCol2, d, rr, c);
                                                fclose(logFile); // Close the file before returning
                                            }
                                        }
                                    }
                                }
                                if (progress) {
                                    FILE *logFile = fopen("solver_actions.log", "a");
                                    fprintf(logFile,"\n");
                                    fclose(logFile);
                                    stats->pointing_triple++;
                                    return progress;
                                }    
                            }
                            continue; // Skip inside-the-box logic
                        }

                        // Inside-the-box scenario
                        bool candidateOutsideBox = false;
                        if (r == otherRow1) { // Row-aligned
                            for (int outsideCol = 0; outsideCol < N; outsideCol++) {
                                if (outsideCol < boxStartCol || outsideCol >= boxStartCol + 3) {
                                    if (maskHasDigit(candidates[r][outsideCol], d)) {
                                        candidateOutsideBox = true;
                                        break;
                                    }
                                }
                            }
                        } else if (c == otherCol1) { // Column-aligned
                            for (int outsideRow = 0; outsideRow < N; outsideRow++) {
                                if (outsideRow < boxStartRow || outsideRow >= boxStartRow + 3) {
                                    if (maskHasDigit(candidates[outsideRow][c], d)) {
                                        candidateOutsideBox = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if (!candidateOutsideBox) { // Apply inside-the-box removal
                            for (int innerR = boxStartRow; innerR < boxStartRow + 3; innerR++) {
                                for (int innerC = boxStartCol; innerC < boxStartCol + 3; innerC++) {
                                    if (r == otherRow1) { // Row-aligned
                                        if (innerR != r && maskHasDigit(candidates[innerR][innerC], d)) {
                                            candidates[innerR][innerC] &= ~digitMask(d);
                                            progress = true;
                                            if (solving_mode) {
                                                FILE *logFile = fopen("solver_actions.log", "a");
                                                if (logFile == NULL) {
                                                    printf("Error opening log file.\n");
                                                    return false; // Exit if the log file cannot be opened
                                                }
                                                fprintf(logFile,"Pointing Triple (Inside Box) at cells (%d, %d), (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow1, otherCol1, otherRow2, otherCol2, d, innerR, innerC);
                                                fclose(logFile); // Close the file before returning
                                            }
                                        }
                                    } else if (c == otherCol1) { // Column-aligned
                                        if (innerC != c && maskHasDigit(candidates[innerR][innerC], d)) {
                                            candidates[innerR][innerC] &= ~digitMask(d);
                                            progress = true;
                                            if (solving_mode) {
                                                FILE *logFile = fopen("solver_actions.log", "a");
                                                if (logFile == NULL) {
                                                    printf("Error opening log file.\n");
                                                    return false; // Exit if the log file cannot be opened
                                                }
                                                fprintf(logFile,"Pointing Triple (Inside Box) at cells (%d, %d), (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow1, otherCol1, otherRow2, otherCol2, d, innerR, innerC);
                                                fclose(logFile); // Close the file before returning
                                            }
                                        }
                                    }
                                }
                            }
                            if (progress) {
                                FILE *logFile = fopen("solver_actions.log", "a");
                                fprintf(logFile,"\n");
                                fclose(logFile);
                                stats->pointing_triple++;
                                return progress;
                            }
                        }
                    }
                }
            }
        }
    }

    return progress;
}


// -------------------------------------- //
// --- ADVANCED TECHNIQUE --- //


/**
 * Function: applyXWing
 * --------------------
 * Identifies "X-Wing" patterns, where a candidate digit appears in exactly
 * two cells in two different rows (or columns), and these cells align in the
 * same columns (or rows). Removes this candidate from all other cells in the
 * affected columns (or rows).
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 * - stats: Pointer to the SolverStats structure for tracking the use of techniques.
 *
 * Returns:
 * - true if progress is made on the puzzle, false otherwise.
 */
bool applyXWing(Sudoku *sudoku, SolverStats *stats) {
    bool progress = false;

    // Row-based X-Wing
    for (int d = 1; d <= 9; d++) { // Iterate through all digits
        for (int r1 = 0; r1 < N - 1; r1++) { // First row
            int columns1[2], count1 = 0;
            for (int c = 0; c < N; c++) {
                if (maskHasDigit(candidates[r1][c], d)) {
                    if (count1 < 2) {
                        columns1[count1++] = c;
                    } else {
                        count1 = 0; // More than 2 columns in the row -> Not an X-Wing
                        break;
                    }
                }
            }
            if (count1 != 2) continue; // Only continue if exactly 2 candidates in the row

            for (int r2 = r1 + 1; r2 < N; r2++) { // Second row
                int columns2[2], count2 = 0;
                for (int c = 0; c < N; c++) {
                    if (maskHasDigit(candidates[r2][c], d)) {
                        if (count2 < 2) {
                            columns2[count2++] = c;
                        } else {
                            count2 = 0; // More than 2 columns in the row -> Not an X-Wing
                            break;
                        }
                    }
                }
                if (count2 != 2 || columns1[0] != columns2[0] || columns1[1] != columns2[1]) {
                    continue; // Ensure columns match for a valid X-Wing
                }

                // Found a row-based X-Wing
                for (int r = 0; r < N; r++) {
                    if (r == r1 || r == r2) continue;
                    for (int i = 0; i < 2; i++) {
                        int c = columns1[i];
                        if (maskHasDigit(candidates[r][c], d)) {
                            candidates[r][c] &= ~digitMask(d);
                            progress = true;
                        }
                    }
                }
                if (progress) {
                    stats->x_wing++;
                    return progress;
                }
            }
        }
    }

    // Column-based X-Wing
    for (int d = 1; d <= 9; d++) { // Iterate through all digits
        for (int c1 = 0; c1 < N - 1; c1++) { // First column
            int rows1[2], count1 = 0;
            for (int r = 0; r < N; r++) {
                if (maskHasDigit(candidates[r][c1], d)) {
                    if (count1 < 2) {
                        rows1[count1++] = r;
                    } else {
                        count1 = 0; // More than 2 rows in the column -> Not an X-Wing
                        break;
                    }
                }
            }
            if (count1 != 2) continue; // Only continue if exactly 2 candidates in the column

            for (int c2 = c1 + 1; c2 < N; c2++) { // Second column
                int rows2[2], count2 = 0;
                for (int r = 0; r < N; r++) {
                    if (maskHasDigit(candidates[r][c2], d)) {
                        if (count2 < 2) {
                            rows2[count2++] = r;
                        } else {
                            count2 = 0; // More than 2 rows in the column -> Not an X-Wing
                            break;
                        }
                    }
                }
                if (count2 != 2 || rows1[0] != rows2[0] || rows1[1] != rows2[1]) {
                    continue; // Ensure rows match for a valid X-Wing
                }

                // Found a column-based X-Wing
                for (int c = 0; c < N; c++) {
                    if (c == c1 || c == c2) continue;
                    for (int i = 0; i < 2; i++) {
                        int r = rows1[i];
                        if (maskHasDigit(candidates[r][c], d)) {
                            candidates[r][c] &= ~digitMask(d);
                            progress = true;
                        }
                    }
                }
                if (progress) {
                    stats->x_wing++;
                    return progress;
                }
            }
        }
    }

    return progress;
}


// ---------------------------------------------------------------------------------------------------- //
// --- SOLVER --- //


/**
 * Function: validateSudoku
 * -------------------------
 * Helper function validating a Sudoku grid to ensure it adheres to Sudoku rules. 
 * Checks for duplicates in rows, columns, and boxes.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 *
 * Returns:
 * - true if the Sudoku grid is valid, false otherwise.
 */
bool validateSudoku(Sudoku *sudoku) {
    for (int r = 0; r < N; r++) {
        int rowMask = 0, colMask = 0;
        for (int c = 0; c < N; c++) {
            if (sudoku->table[r][c] != 0) {
                if (rowMask & digitMask(sudoku->table[r][c])) return false;
                rowMask |= digitMask(sudoku->table[r][c]);
            }
            if (sudoku->table[c][r] != 0) {
                if (colMask & digitMask(sudoku->table[c][r])) return false;
                colMask |= digitMask(sudoku->table[c][r]);
            }
        }
    }

    for (int boxRow = 0; boxRow < 3; boxRow++) {
        for (int boxCol = 0; boxCol < 3; boxCol++) {
            int boxMask = 0;
            for (int r = 0; r < 3; r++) {
                for (int c = 0; c < 3; c++) {
                    int globalRow = boxRow * 3 + r;
                    int globalCol = boxCol * 3 + c;
                    if (sudoku->table[globalRow][globalCol] != 0) {
                        if (boxMask & digitMask(sudoku->table[globalRow][globalCol])) return false;
                        boxMask |= digitMask(sudoku->table[globalRow][globalCol]);
                    }
                }
            }
        }
    }

    return true;
}


/**
 * Function: solve_human
 * ----------------------
 * Solves a Sudoku puzzle using human-like strategies. Applies techniques
 * iteratively until no more progress can be made.
 *
 * Parameters:
 * - sudoku: Pointer to the Sudoku puzzle structure.
 * - stats: Pointer to the SolverStats structure for tracking the use of techniques.
 * - solving_mode: boolean flag indicating whether to record the moves made in a log file.
 *
 * Returns:
 * - true if the puzzle is solved, false otherwise.
 */
bool solve_human(Sudoku *sudoku, SolverStats *stats, bool solving_mode) {

    // Clear the log file at the beginning of the function
    FILE *logFile = fopen("solver_actions.log", "w");
    if (logFile == NULL) {
        // printf("Error opening log file for clearing.\n");
        return false; // Exit if the log file cannot be opened
    }
    fclose(logFile); // Close immediately after clearing

    initCandidates(sudoku);
    bool progress;
    do {
        progress = false;
        if (applyNakedSingle(sudoku, stats, solving_mode)) {
            if (!validateSudoku(sudoku)) {
                // printf("Error: Invalid state after applying Naked Single.\n");
                return false;
            }
            progress = true;
        } else if (applyHiddenSingle(sudoku, stats, solving_mode)) {
            if (!validateSudoku(sudoku)) {
                // printf("Error: Invalid state after applying Hidden Single.\n");
                return false;
            }
            progress = true;
        } else if (applyPointingPair(sudoku,stats, solving_mode)) {
            if (!validateSudoku(sudoku)) {
                // printf("Error: Invalid state after applying Pointing Pair.\n");
                return false;
            }
            progress = true;
        } else if (applyNakedPair(sudoku, stats, solving_mode)) {
            if (!validateSudoku(sudoku)) {
                // printf("Error: Invalid state after applying Naked Pair.\n");
                return false;
            }
            progress = true;
        } else if (applyHiddenPair(sudoku, stats, solving_mode)) {
            if (!validateSudoku(sudoku)) {
                // printf("Error: Invalid state after applying Hidden Pair.\n");
                return false;
            }
            progress = true;
        } else if (applyPointingTriples(sudoku,stats, solving_mode)) {
            if (!validateSudoku(sudoku)) {
                // printf("Error: Invalid state after applying Pointing Triple.\n");
                return false;
            }
            progress = true;
        } else if (applyNakedTriple(sudoku, stats, solving_mode)) {
            if (!validateSudoku(sudoku)) {
                // printf("Error: Invalid state after applying Naked Triple.\n");
                return false;
            }
            progress = true;
        } else if (applyHiddenTriple(sudoku, stats, solving_mode)){
            if (!validateSudoku(sudoku)) {
                // printf("Error: Invalid state after applying Hidden Triple.\n");
                return false;
            }
            progress = true;
        }  else if (applyXWing(sudoku, stats)) {
            if (!validateSudoku(sudoku)) {
                // printf("Error: Invalid state after applying X-Wing.\n");
                return false;
            }
            progress = true;
        }
    } while (progress);

    return !find_empty(sudoku, &(int){0}, &(int){0});
}


/**
 * Function: print_stats
 * ----------------------
 * Prints the statistics of the human-solving process, including the number
 * of times each technique was used.
 *
 * Parameters:
 * - stats: Pointer to the SolverStats structure containing the statistics.
 *
 * Returns:
 * - Nothing. Outputs the statistics to the standard output.
 */
void print_stats(SolverStats *stats) {
    printf("naked sing: %d\n", stats->naked_single);
    printf("hidden sing: %d\n", stats->hidden_single);
    printf("naked pair: %d\n", stats->naked_pair);
    printf("hidden pair: %d\n", stats->hidden_pair);
    printf("pointing pair: %d\n", stats->pointing_pair);
    printf("naked triple: %d\n", stats->naked_triple);
    printf("hidden triple: %d\n", stats->hidden_triple);
    printf("pointing triple: %d\n", stats->pointing_triple);
    printf("X wing: %d\n", stats->x_wing);
}


// ---------------------------------------------------------------------------------------------------- //
// --- MAIN FUNCTION --- //


/**
 * Function: main
 * --------------
 * Entry point for the human solver program. Reads a Sudoku puzzle from a file,
 * attempts to solve it using human-like strategies, and outputs the solution
 * along with statistics of the solving process.
 *
 * Parameters:
 * - argc: The number of command-line arguments.
 * - argv: An array of command-line arguments. The second argument should be the file path
 *         of the input Sudoku puzzle.
 *
 * Returns:
 * - 0 on successful execution, or an error code for invalid inputs or unsolvable puzzles.
 */
// int main(int argc, char *argv[]) {
//     if (argc != 2) {
//         printf("Usage: %s <input_file>\n", argv[0]);
//         return 1;
//     }

//     Sudoku sudoku;
//     SolverStats stats = {0};
//     bool solving_mode = false;
//     parse_file(&sudoku, argv[1]);
//     printf("Initial Sudoku:\n");
//     print_table(&sudoku);

//     if (solve_human(&sudoku, &stats, solving_mode)) {
//         if (validateSudoku(&sudoku)) {
//             printf("Solved Sudoku:\n");
//             print_table(&sudoku);
//             print_stats(&stats);
//         } else {
//             printf("Error: Invalid Sudoku state detected after solving.\n");
//         }
//     } else {
//         printf("Cannot solve Sudoku with current strategies.\n");
//         print_table(&sudoku);
//         print_stats(&stats);
//     }
//     write_to_file(&sudoku, "sudoku_solution.txt");

//     return 0;
// }
