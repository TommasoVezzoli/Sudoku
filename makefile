CC=gcc
SRC=src
COMMON=$(SRC)/helpers.c $(SRC)/io.c $(SRC)/solver_human.c
TARGETS=run_backtrack run_generator

all: $(TARGETS)

run_backtrack: $(SRC)/solver_backtrack.c $(COMMON)
	$(CC) -o $@ $(SRC)/solver_backtrack.c $(COMMON)

run_generator: $(SRC)/generator.c $(COMMON)
	$(CC) -o $@ $(SRC)/generator.c $(COMMON)

clean:
	rm -f $(TARGETS)
