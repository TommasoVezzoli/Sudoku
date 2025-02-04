CC=gcc
SRC=src
COMMON=$(SRC)/helpers.c $(SRC)/io.c $(SRC)/solver_human.c
TARGETS=run_solver.exe run_generator.exe

all: $(TARGETS)

run_solver.exe: $(SRC)/solver_backtrack.c $(COMMON)
	$(CC) -o $@ $(SRC)/solver_backtrack.c $(COMMON)

run_generator.exe: $(SRC)/generator.c $(COMMON)
	$(CC) -o $@ $(SRC)/generator.c $(COMMON)

clean:
	rm -f $(TARGETS)
