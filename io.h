#ifndef IO_H
#define IO_H

#define N 9

typedef struct {
    int table[N][N];
} Sudoku;

void parse_file();
void print_table();
void write_to_file();

#endif