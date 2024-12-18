## Sudoku solver & generator

This is a simple Streamlit web application hosting a Sudoku solver and generator.
On the platform, it is possible to ...

---

### Roadmap

##### Interface features:
- [x] Design the Sudoku interactive grid
- [x] Upload and download Sudoku puzzles into text files
- [x] Show only valid inputs
- [x] Freeze the configuration for manual solving
- [x] Call the solver
- [x] Handle multiple possible solutions
- [ ] Hint 1: Unveil a chosen cell

##### Development features:
- [x] Solve a Sudoku puzzle with the backtracking algorithm
- [x] Solve a Sudoku puzzle using human resolution strategies
- [x] Receive the path to the Sudoku puzzle
- [ ] Correct output procedure in human solver
- [x] Return multiple possible solutions (upper bounded)
- [ ] Solve a non-9x9 Sudoku with backtracking and with human solver
- [ ] Generate a Sudoku puzzle (difficulty level) with a unique solution
- [ ] Generate a Sudoku puzzle with multiple solutions

### Ideas on how to handle multiple solutions
- generate a file for each possible solution and include them all in a folder
- display solutions separately in the popup
- if the first entry is correct (in any solution), color it green, then color green if the new entry is correct in the same solution, and yellow otherwise
- make it a challenge: 3 lives, then new sudoku

### Resources for the generation part
- [Standard approach](https://www.101computing.net/sudoku-generator-algorithm/) \
Add a random number in a random cell, check solution existence, and repeat until the board is filled.
Then, remove one at each iteration and check if the solution is still unique.

#### Attempting different difficulties
- [Dig-hole strategy, Las Vegas algorithm](https://zhangroup.aporc.org/images/files/Paper_3485.pdf) \
Difficulty depends on: number of hints, position of the hints, applicability of logical strategies. The algorithm does the following:
  1. create a valid grid using Las Vegas algorithm
  2. erase some digits using five operators:
        - determine a sequence of digging holes according to the desired difficulty
        - set restrictions to guide the distribution of the given cells
        - check solution existence
        - prune to avoid digging invalid holes
        - propagate at a dug-out puzzle to raise diversity
- [BFS strategy](https://sites.math.washington.edu/~morrow/mcm/team2306.pdf)
- [Dig-hole strategy 2](https://www.irjmets.com/uploadedfiles/paper/issue_4_april_2022/21485/final/fin_irjmets1651644665.pdf) \
Levels of difficulty are based solely on the number opf hints given, not on the position. \
The paper estimates: easy (36-46), medium (23-35), hard (28-31). \
The paper goes over some methods (exchanging digits, bands, stacks, ...) for creating a puzzle starting from another one, but it does not provide a method for generating a puzzle from scratch. Very bad paper.

#### Common findings
- Digit exchanging consists in swapping all occurrences of a digit with another one (e.g. swap all 9s with 2s and all 2s with 9s). This does impact on the uniqueness of the solution.
- Minimal amount of hints for a unique solution: 17