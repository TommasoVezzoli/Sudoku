CC=gcc
COMMON=helpers.c io.c solver_human.c
TARGETS=run_backtrack.exe run_generator.exe

all: $(TARGETS)

run_backtrack.exe: solver_backtrack.c $(COMMON)
	$(CC) -o $@ solver_backtrack.c $(COMMON)

run_generator.exe: generator.c $(COMMON)
	$(CC) -o $@ generator.c $(COMMON)

clean:
	rm -f $(TARGETS)
