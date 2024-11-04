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
- [ ] Handle multiple possible solutions
- [ ] Hint 1: Unveil a chosen cell

##### Development features:
- [x] Solve a Sudoku puzzle with the backtracking algorithm
- [x] Solve a Sudoku puzzle using human resolution strategies
- [ ] Receive the path to the Sudoku puzzle
- [ ] Correct output procedure in human solver
- [ ] Return multiple possible solutions (upper bounded)
- [ ] Solve a non-9x9 Sudoku with backtracking and with human solver
- [ ] Generate a Sudoku puzzle (difficulty level) with a unique solution
- [ ] Generate a Sudoku puzzle with multiple solutions

### Ideas on how to handle multiple solutions
- generate a file for each possible solution and include them all in a folder
- display solutions separately in the popup
- if the first entry is correct (in any solution), color it green, then color green if the new entry is correct in the same solution, and yellow otherwise
- make it a challenge: 3 lives, then new sudoku