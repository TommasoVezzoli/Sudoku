#include "io.h"
#include "human_solver.h"
#include "helpers.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N 9

// Global candidates array
static unsigned short candidates[N][N];
// void printCandidates() {
//     for (int r = 0; r < N; r++) {
//         for (int c = 0; c < N; c++) {
//             printf("Cell (%d, %d): Candidates = %03X\n", r, c, candidates[r][c]);
//         }
//     }
// }



void printCandidatesFromMask(unsigned short mask) {
    printf("Mask: %03X (binary: ", mask);
    for (int i = 8; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf(") -> Candidates: ");
    for (int d = 1; d <= 9; d++) {
        if (mask & (1U << (d - 1))) {
            printf("%d ", d);
        }
    }
    printf("\n");
}


// Helper Functions
static unsigned short digitMask(int d) {
    
    return 1U << (d - 1);
}

static bool maskHasDigit(unsigned short mask, int d) {
    return (mask & digitMask(d)) != 0;
}

void printCandidates() {
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            printf("Cell (%d, %d): Candidates = [", r, c);
            for (int d = 1; d <= 9; d++) {
                if (maskHasDigit(candidates[r][c], d)) {
                    printf("%d", d);
                    // Add a comma and space if not the last candidate
                    if (d < 9) {
                        printf(", ");
                    }
                }
            }
            printf("]\n");
        }
    }
}

static int bitCount(unsigned short mask) {
    unsigned short localMask = mask; // Create a copy of the mask
    int count = 0;
    while (localMask) {
        count += (localMask & 1);
        localMask >>= 1;
    }
    return count;
}



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



// Technique Implementations
/*FUNZIONA*/
bool applyNakedSingle(Sudoku *sudoku, SolverStats *stats) {
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (sudoku->table[r][c] == 0) { // Empty cell
                unsigned short mask = candidates[r][c];
                if (bitCount(mask) == 1) { // Only one candidate
                    for (int d = 1; d <= 9; d++) {
                        if (maskHasDigit(mask, d)) {
                            // printf("Naked Single: Placing %d in cell (%d, %d)\n", d, r, c);
                            setCell(sudoku, r, c, d);
                            stats->naked_single++;
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}
/*FUNZIONA*/
bool applyHiddenSingle(Sudoku *sudoku, SolverStats *stats) {
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
                // printf("Hidden Single (Row): Placing %d in cell (%d, %d)\n", d, r, col);
                setCell(sudoku, r, col, d);
                stats->hidden_single++;
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
                // printf("Hidden Single (Column): Placing %d in cell (%d, %d)\n", d, row, c);
                setCell(sudoku, row, c, d);
                stats->hidden_single++;
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
                    // printf("Hidden Single (Box): Placing %d in cell (%d, %d)\n", d, row, col);
                    setCell(sudoku, row, col, d);
                    stats->hidden_single++;
                    return true;
                }
            }
        }
    }

    return false;
}

 /*QUESTA FUNZIONA*/
// Helper function to check if two masks represent a naked pair
bool isNakedPair(unsigned short mask1, unsigned short mask2) {
    return (mask1 == mask2) && (bitCount(mask1) == 2);
}

bool applyNakedPair(Sudoku *sudoku, SolverStats *stats) {
    bool progress = false;

    // Debug: Print initial state of candidates
    // printf("Initial candidates:\n");
    // printCandidates(candidates);

    // Check rows for Naked Pairs
    for (int r = 0; r < N; r++) {
        for (int c1 = 0; c1 < N - 1; c1++) {
            if (sudoku->table[r][c1] == 0 && bitCount(candidates[r][c1]) == 2) {
                for (int c2 = c1 + 1; c2 < N; c2++) {
                    if (isNakedPair(candidates[r][c1], candidates[r][c2])) {
                        unsigned short pairMask = candidates[r][c1];
                        // printf("Naked Pair found in Row %d: Column Pair (%d, %d) [%03X]\n", r, c1, c2, pairMask);
                        // print_table(sudoku);

                        for (int c3 = 0; c3 < N; c3++) {
                            if (c3 != c1 && c3 != c2 && sudoku->table[r][c3] == 0) {
                                if (candidates[r][c3] & pairMask) {
                                    // printf("Naked pair: removing candidates [%03X] from cell (%d, %d)\n", pairMask, r, c3);
                                    // printf("Naked pair in row %d: Removing candidates ", r);
                                    // printCandidatesFromMask(pairMask);
                                    // printf(" from cell (%d, %d)\n", r, c3);
                                    candidates[r][c3] &= ~pairMask;
                                    // stats->naked_pair++; /*consider exiting after finding one naked pair*/
                                    progress = true;
                                }
                            }
                        }
                        if (progress) {
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
                        // printf("Naked Pair found in Column %d: Pair (%d, %d) [%03X]\n", c, r1, r2, pairMask);

                        for (int r3 = 0; r3 < N; r3++) {
                            if (r3 != r1 && r3 != r2 && sudoku->table[r3][c] == 0) {
                                if (candidates[r3][c] & pairMask) {
                                    // printf("Naked pair: removing candidates [%03X] from cell (%d, %d)\n", pairMask, r3, c);
                                    // printf("Naked pair in column %d: Removing candidates ", c);
                                    // printCandidatesFromMask(pairMask);
                                    // printf(" from cell (%d, %d)\n", r3, c);                                    
                                    candidates[r3][c] &= ~pairMask;
                                    // stats->naked_pair++;
                                    progress = true;
                                }
                            }
                        }
                        if (progress) {
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
                        // printf("Naked Pair found in Box (%d, %d): Pair (%d, %d) [%03X]\n",
                            //    boxRow, boxCol, i, j, pairMask);

                        for (int k = 0; k < count; k++) {
                            if (k != i && k != j) {
                                int r = cells[k][0], c = cells[k][1];
                                if (candidates[r][c] & pairMask) {
                                    // printf("Naked pair: removing candidates [%03X] from cell (%d, %d)\n", pairMask, r, c);
                                    // printf("Naked pair in box: Removing candidates ");
                                    // printCandidatesFromMask(pairMask);
                                    // printf(" from cell (%d, %d)\n", r, c);                                    
                                    candidates[r][c] &= ~pairMask;
                                    // stats->naked_pair++;
                                    progress = true;
                                }
                            }
                        }
                        if (progress) {
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


/*QUESTA DOVREBBE FUNZIONARE MA DA RIVEDERE*/
bool isHiddenPair(unsigned short mask1, unsigned short mask2) {
    unsigned short pairMask = mask1 & mask2;
    return bitCount(pairMask) == 2 && (bitCount(mask1) > 2 || bitCount(mask2) > 2);
}

void removeOtherCandidates(unsigned short *cellMask, unsigned short pairMask) {
    *cellMask &= pairMask; // Remove all candidates except the pair
}

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

bool applyHiddenPair(Sudoku *sudoku, SolverStats *stats) {
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

                        progress = true;
                        stats->hidden_pair++;
                        // printf("Hidden Pair: (%d, %d) in cells (%d, %d) and (%d, %d), Cleared other candidates\n",
                            //    d1, d2, r1, c1, r2, c2);

                        return progress; // Exit after finding one hidden pair
                    }
                }
            }
        }
    }

    return progress;
}



bool applyPointingPairs(Sudoku *sudoku, SolverStats *stats) {
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
                                            // printf("Pointing Pair (Row Outside Box) at cells (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow, otherCol, d, r, cc);
                                        }
                                    }
                                }
                                if (progress) {
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
                                            // stats->pointing_pair++;
                                            // printf("Pointing Pair (Row Inside Box) at cells (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow, otherCol, d, innerR, innerC);
                                        }
                                    }
                                }
                                if (progress) {
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
                            // if (d == 9 && c == 6) printf("found pair at cells (%d, %d) and (%d, %d): %d\n", r,c,otherRow, otherCol, d);
                            // if (d == 5 && c == 6) printf("found pair at cells (%d, %d) and (%d, %d): %d\n", r,c,otherRow, otherCol, d);

                            bool candidateInBox = false;
                            for (int rr = boxStartRow; rr < boxStartRow + 3; rr++) {
                                for (int cc = boxStartCol; cc < boxStartCol + 3; cc++) {
                                    if ((rr != r || cc != c) && (rr != otherRow || cc != otherCol) &&
                                        maskHasDigit(candidates[rr][cc], d)) {
                                        candidateInBox = true;
                                        // if (d == 9 && c == 6) printf("    But candidate 9 is already present in box: cannot apply outside removal.\n");
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
                                            // printf("Pointing Pair (Col Outside Box) at cells (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow, otherCol, d, rr, c);
                                        }
                                    }
                                }
                                if (progress) {
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
                                        // if (d == 9 && c == 6) printf("    But candidate 9 is already present in the column: cannot apply inside removal.\n");
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
                                            // stats->pointing_pair++;
                                            // printf("Pointing Pair (Col Inside Box) at cells (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow, otherCol, d, innerR, innerC);
                                        }
                                    }
                                }
                                if (progress) {
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





/*QUESTA FUNZIONA*/
// Helper function to check if three masks represent a Naked Triple
bool isNakedTriple(unsigned short mask1, unsigned short mask2, unsigned short mask3) {
    unsigned short combinedMask = mask1 | mask2 | mask3;
    return bitCount(combinedMask) == 3; // The combined mask must contain exactly 3 candidates
}

bool applyNakedTriple(Sudoku *sudoku, SolverStats *stats) {
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
                                                // printf("Naked Triple in Row %d: Removing candidates ", r);
                                                // printCandidatesFromMask(tripleMask);
                                                // printf(" from cell (%d, %d)\n", r, c);

                                                candidates[r][c] &= ~tripleMask;
                                                // stats->naked_triple++;
                                                progress = true;
                                            }
                                        }
                                    }
                                    if (progress){
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
                                                // printf("Naked Triple in Column %d: Removing candidates ", c);
                                                // printCandidatesFromMask(tripleMask);
                                                // printf(" from cell (%d, %d)\n", r, c);

                                                candidates[r][c] &= ~tripleMask;
                                                // stats->naked_triple++;
                                                progress = true;
                                            }
                                        }
                                    }
                                    if (progress){
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
                                        // printf("Naked Triple in Box (%d, %d): Removing candidates ", boxRow, boxCol);
                                        // printCandidatesFromMask(tripleMask);
                                        // printf(" from cell (%d, %d)\n", r, c);

                                        candidates[r][c] &= ~tripleMask;
                                        // stats->naked_triple++;
                                        progress = true;
                                    }
                                }
                            }
                            if (progress){
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


/*FUNZIONA*/
bool isHiddenTriple(unsigned short mask1, unsigned short mask2, unsigned short mask3) {
    unsigned short combinedMask = mask1 | mask2 | mask3;
    return bitCount(combinedMask) == 3; // The combined mask must contain exactly 3 candidates
}

void removeOtherCandidatesForTriple(unsigned short *cellMask, unsigned short tripleMask) {
    unsigned short originalMask = *cellMask;
    *cellMask &= tripleMask; // Keep only the candidates in the triple mask
    // if (*cellMask != originalMask) {
        // printf("Removed candidates [%03X] from cell, remaining candidates: [%03X]\n", originalMask & ~tripleMask, *cellMask);
    // }
}

bool applyHiddenTriple(Sudoku *sudoku, SolverStats *stats) {
    // printf("\n\nEntered Hidden Triple!\n\n");
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
                            // printf("Checking triple (%d, %d, %d) in cells:\n", d1, d2, d3);
                            // for (int k = 0; k < 3; k++) {
                                // printf("Cell (%d, %d): Candidates = [%03X]\n",
                                    //    tripleCells[k][0], tripleCells[k][1],
                                    //    candidates[tripleCells[k][0]][tripleCells[k][1]]);
                            // }

                            int r1 = tripleCells[0][0], c1 = tripleCells[0][1];
                            int r2 = tripleCells[1][0], c2 = tripleCells[1][1];
                            int r3 = tripleCells[2][0], c3 = tripleCells[2][1];

                            // Ensure the tripleMask is valid (only includes the hidden triple candidates)
                            unsigned short actualTripleMask =
                                (candidates[r1][c1] | candidates[r2][c2] | candidates[r3][c3]) &
                                tripleMask;

                            if (bitCount(actualTripleMask) != 3) {
                                // printf("Invalid triple mask [%03X] for triple (%d, %d, %d)\n",
                                    //    actualTripleMask, d1, d2, d3);
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
                                stats->hidden_triple++;
                                // printf("Hidden Triple: (%d, %d, %d) in cells (%d, %d), (%d, %d) and (%d, %d), Cleared other candidates\n",
                                //        d1, d2, d3, r1, c1, r2, c2, r3, c3);
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







/*FUNZIONA*/ /*STESSO EDGE CASE DEL POINTING PAIR*/
bool applyPointingTriples(Sudoku *sudoku, SolverStats *stats) {
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
                                            // match_found_row++;
                                            otherRow1 = rr;
                                            otherCol1 = cc;
                                        } else if (match_found_row == 1) {
                                            // match_found_row++;
                                            otherRow2 = rr;
                                            otherCol2 = cc;
                                        }
                                        match_found_row++;
                                    }
                                }
                            }
                            // if (pairFound) break;
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
                                                // match_found_row++;
                                                otherRow1 = rrr;
                                                otherCol1 = ccc;
                                            } else if (match_found_col == 1) {
                                                // match_found_row++;
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
                        // printf("found pair at cells (%d, %d) and (%d, %d): %d\n", r,c,otherRow, otherCol, d);

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
                                            // stats->pointing_pair++;
                                            // printf("Pointing Triple (Outside Box) at cells (%d, %d), (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow1, otherCol1, otherRow2, otherCol2, d, r, cc);
                                        }
                                    }
                                } 
                                if (progress) {
                                    // print_table(sudoku);
                                    // printCandidates();
                                    stats->pointing_triple++;
                                    return progress;
                                }
                            } else if (c == otherCol1) { // Column-aligned
                                for (int rr = 0; rr < N; rr++) {
                                    if (rr < boxStartRow || rr >= boxStartRow + 3) {
                                        if (maskHasDigit(candidates[rr][c], d)) {
                                            candidates[rr][c] &= ~digitMask(d);
                                            progress = true;
                                            // stats->pointing_pair++;
                                            // printf("Pointing Triple (Outside Box) at cells (%d, %d), (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow1, otherCol1, otherRow2, otherCol2, d, rr, c);
                                        }
                                    }
                                }
                                if (progress) {
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
                                            // stats->pointing_pair++;
                                            // printf("Pointing Triple (Inside Box) at cells (%d, %d), (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow1, otherCol1, otherRow2, otherCol2, d, innerR, innerC);
                                        }
                                    } else if (c == otherCol1) { // Column-aligned
                                        if (innerC != c && maskHasDigit(candidates[innerR][innerC], d)) {
                                            candidates[innerR][innerC] &= ~digitMask(d);
                                            progress = true;
                                            // stats->pointing_pair++;
                                            // printf("Pointing Triple (Inside Box) at cells (%d, %d), (%d, %d) and (%d, %d): Removed %d from cell (%d, %d)\n", r, c, otherRow1, otherCol1, otherRow2, otherCol2, d, innerR, innerC);
                                        }
                                    }
                                }
                            }
                            if (progress) {
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




bool applyXWing(Sudoku *sudoku, SolverStats *stats) {
    // Row-based X-Wing
    for (int d = 1; d <= 9; d++) {
        for (int r1 = 0; r1 < N - 1; r1++) {
            for (int r2 = r1 + 1; r2 < N; r2++) {
                int c1 = -1, c2 = -1;
                for (int c = 0; c < N; c++) {
                    if (maskHasDigit(candidates[r1][c], d) && maskHasDigit(candidates[r2][c], d)) {
                        if (c1 == -1) c1 = c;
                        else if (c2 == -1) c2 = c;
                        else break; // More than 2 columns -> Not an X-Wing
                    }
                }
                if (c1 != -1 && c2 != -1) {
                    // Found a row-based X-Wing for candidate `d` in columns `c1` and `c2`
                    for (int r = 0; r < N; r++) {
                        if (r != r1 && r != r2) {
                            if (maskHasDigit(candidates[r][c1], d)) {
                                candidates[r][c1] &= ~digitMask(d);
                            }
                            if (maskHasDigit(candidates[r][c2], d)) {
                                candidates[r][c2] &= ~digitMask(d);
                            }
                        }
                    }
                    stats->x_wing++;
                    return true;
                }
            }
        }
    }

    // Column-based X-Wing
    for (int d = 1; d <= 9; d++) {
        for (int c1 = 0; c1 < N - 1; c1++) {
            for (int c2 = c1 + 1; c2 < N; c2++) {
                int r1 = -1, r2 = -1;
                for (int r = 0; r < N; r++) {
                    if (maskHasDigit(candidates[r][c1], d) && maskHasDigit(candidates[r][c2], d)) {
                        if (r1 == -1) r1 = r;
                        else if (r2 == -1) r2 = r;
                        else break; // More than 2 rows -> Not an X-Wing
                    }
                }
                if (r1 != -1 && r2 != -1) {
                    // Found a column-based X-Wing for candidate `d` in rows `r1` and `r2`
                    for (int c = 0; c < N; c++) {
                        if (c != c1 && c != c2) {
                            if (maskHasDigit(candidates[r1][c], d)) {
                                candidates[r1][c] &= ~digitMask(d);
                            }
                            if (maskHasDigit(candidates[r2][c], d)) {
                                candidates[r2][c] &= ~digitMask(d);
                            }
                        }
                    }
                    stats->x_wing++;
                    return true;
                }
            }
        }
    }

    return false;
}


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

bool solve_human2(Sudoku *sudoku, SolverStats *stats) {
    initCandidates(sudoku);
    // printCandidates();
    bool progress;
    do {
        // return false;
        progress = false;
        if (applyNakedSingle(sudoku, stats)) {
            if (!validateSudoku(sudoku)) {
                printf("Error: Invalid state after applying Naked Single.\n");
                return false;
            }
            progress = true;
        } else if (applyHiddenSingle(sudoku, stats)) {
            if (!validateSudoku(sudoku)) {
                // print_table(sudoku);
                printf("Error: Invalid state after applying Hidden Single.\n");
                return false;
            }
            progress = true;
        } else if (applyPointingPairs(sudoku,stats)) {
            // printf("Pointing inside the box works!\n");
            if (!validateSudoku(sudoku)) {
                printf("Error: Invalid state after applying Pointing Pair.\n");
                return false;
            }
            progress = true;
            // print_table(sudoku);
        } else if (applyNakedPair(sudoku, stats)) {
            if (!validateSudoku(sudoku)) {
                printf("Error: Invalid state after applying Naked Pair.\n");
                return false;
            }
            progress = true;
        } else if (applyHiddenPair(sudoku, stats)) {
            if (!validateSudoku(sudoku)) {
                printf("Error: Invalid state after applying Hidden Pair.\n");
                return false;
            }
            // break;
            progress = true;
        } else if (applyPointingTriples(sudoku,stats)) {
            if (!validateSudoku(sudoku)) {
                printf("Error: Invalid state after applying Pointing Triple.\n");
                return false;
            }
            progress = true;
        } else if (applyNakedTriple(sudoku, stats)) {
            if (!validateSudoku(sudoku)) {
                printf("Error: Invalid state after applying Naked Triple.\n");
                return false;
            }
            progress = true;
        } else if (applyHiddenTriple(sudoku, stats)){
            if (!validateSudoku(sudoku)) {
                printf("Error: Invalid state after applying Hidden Triple.\n");
                return false;
            }
            // break;
            progress = true;
        }
        // }  else if (applyXWing(sudoku, stats)) {
        //     if (!validateSudoku(sudoku)) {
        //         printf("Error: Invalid state after applying X-Wing.\n");
        //         return false;
        //     }
        //     progress = true;
        // }
    } while (progress);

    return !find_empty(sudoku, &(int){0}, &(int){0});
}


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

// int main(int argc, char *argv[]) {
//     if (argc != 2) {
//         printf("Usage: %s <input_file>\n", argv[0]);
//         return 1;
//     }

//     Sudoku sudoku;
//     SolverStats stats = {0};
//     parse_file(&sudoku, argv[1]);
//     printf("Initial Sudoku:\n");
//     print_table(&sudoku);

//     if (solve_human2(&sudoku, &stats)) {
//         if (validateSudoku(&sudoku)) {
//             printf("Solved Sudoku:\n");
//             print_table(&sudoku);
//             print_stats(&stats);
//         } else {
//             printf("Error: Invalid Sudoku state detected after solving.\n");
//         }
//     } else {
//         // printf("Naked singles: %d\n", stats.naked_single);
//         // printf("Hidden singles: %d\n", stats.hidden_single);
//         // printf("Pointing pair: %d\n", stats.pointing_pair);
//         printf("Cannot solve Sudoku with current strategies.\n");
//         print_table(&sudoku);
//         print_stats(&stats);
//     }
//     write_to_file(&sudoku, "sudoku_solution.txt");

//     return 0;
// }