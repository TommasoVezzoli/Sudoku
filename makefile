CC=gcc
COMMON=helpers.c io.c
TARGETS=run_backtrack.exe run_generator.exe

all: $(TARGETS)

run_backtrack.exe: solver_backtrack.c $(COMMON)
	$(CC) -o $@ solver_backtrack.c $(COMMON)

run_generator.exe: generator.c $(COMMON)
	$(CC) -o $@ generator.c $(COMMON)

# run_human: solver_human.c $(COMMON)
# 	$(CC) $(CFLAGS) -o $@ solver_human.c $(COMMON)

clean:
	rm -f $(TARGETS)
