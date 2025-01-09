## Sudoku solver & generator

### 20875 - SWE Project

- Tommaso Vezzoli
- Giulio Pirotta

---

### Description
The **Sudoku solver & generator** is a practical Streamlit web application to help people solve and generate Sudoku puzzles of any difficulty level. 
Detailed instructions on how to use the application are provided in the application itself.

---

### Folder structure

This is the folder structure of the project:

```
Sudoku/
├── .git/
├── .idea/
├── .streamlit/
│   └── config.toml
├── src/
│   ├── generator.c
│   ├── helpers.c
│   ├── helpers.h
│   ├── io.c
│   ├── io.h
│   ├── solver_backtrack.c
│   ├── solver_human.c
│   ├── solver_human.h
│   ├── Seeds/
│   │   ├── level3/
│   │   │   ├── puzzle1.txt
│   │   │   └── ...
│   │   └── level4/
│   │       ├── puzzle1.txt
│   │       └── ...
│   ├── Solutions/
│   └── Tmp/
├── __pycache__/s
├── Makefile
└── README.md
```

All files and functions are carefully documented to make the code easy to understand and maintain.

---

### Usage
Follow these steps to make the application work on your local machine:

#### 1. Install the required packages
Run this command to install the dependencies:
```
pip install -r requirements.txt
```
#### 2. Compile the c files
During the developing phase, the compilation ocurred using `gcc` on `WSL2` (Windows Subsystem for Linux).
However, to make them work as intended on Windows, we recommend using `MinGW`. \
In any case, use the provided makefile to compile. \
If no modifications are needed, it's possible to use the pre-compiled executables that are already in the repository.

#### 3. Run the application
Start the application by running the following command:
```
streamlit run gui_streamlit.py
```

---

### Next steps
- [ ] Solve and generate non 9x9 Sudoku puzzles
- [ ] Improve real-time solving support
- [ ] Add more advanced techniques to the human solver
- [ ] Generate difficult Sudoku from scratch instead of using pre-existing seeds
- [ ] Publish the application online
