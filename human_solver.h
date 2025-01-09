// human_solver.h
#ifndef HUMAN_SOLVER_H
#define HUMAN_SOLVER_H

#include "io.h" // Include the file where the Sudoku struct is defined
#include <stdbool.h>
/**
 * Struct: SolverStats
 * --------------------
 * Tracks the usage of human-solving techniques during the solving process.
 *
 * Fields:
 * - basic_elimination: Count of times the basic elimination technique is used.
 * - naked_single: Count of times the naked single technique is used.
 * - hidden_single: Count of times the hidden single technique is used.
 * - naked_pair: Count of times the naked pair technique is used.
 * - hidden_pair: Count of times the hidden pair technique is used.
 * - pointing_pair: Count of times the pointing pair technique is used.
 * - naked_triple: Count of times the naked triple technique is used.
 * - hidden_triple: Count of times the hidden triple technique is used.
 * - pointing_triple: Count of times the pointing triple technique is used.
 * - x_wing: Count of times the X-Wing technique is used.
 */
typedef struct {
    int naked_single;
    int hidden_single;
    int naked_pair;
    int hidden_pair;
    int pointing_pair;
    int naked_triple;
    int hidden_triple;
    int pointing_triple;
    int x_wing;
} SolverStats;


bool solve_human(Sudoku *sudoku, SolverStats *stats, bool solving_mode);

void print_stats(SolverStats *stats);

#endif // HUMAN_SOLVER_H
