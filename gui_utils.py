import numpy as np
import os
import subprocess


def call_c_file(file_name: str, input_file: str) -> bool:
    run_process = subprocess.run([file_name, input_file], capture_output=True, text=True)
    # return run_process.stdout.decode("utf-8")
    if run_process.returncode != 0:
        return False
    else:
        return True


def check_valid_sudoku(sudoku: np.ndarray) -> bool:
    """
    Check if the current sudoku puzzle is valid.
    This function can be called both if the board is full or just partially completed.

    :param sudoku: sudoku puzzle
    :return: boolean
    """

    for i in range(9):
        no_zeros_row = sudoku[i][sudoku[i] != 0]
        if len(no_zeros_row) != len(set(no_zeros_row)):
            return False
        no_zeros_col = sudoku[:, i][sudoku[:, i] != 0]
        if len(no_zeros_col) != len(set(no_zeros_col)):
            return False

    for i in range(0, 9, 3):
        for j in range(0, 9, 3):
            no_zeros_block = sudoku[i:i+3, j:j+3].flatten()[sudoku[i:i+3, j:j+3].flatten() != 0]
            if len(no_zeros_block) != len(set(no_zeros_block)):
                return False

    return True


def generate_sudoku_html(sudoku: np.ndarray, comparison :np.ndarray=None) -> None:
    """
    Generate a html representation of a sudoku puzzle.
    If a comparison is provided, the cells are colored according to whether the value is the same or differs.

    :param sudoku: sudoku puzzle
    :param comparison: comparison for the sudoku puzzle
    :return: None
    """

    table_header = """
        <style>
            table {
                border-collapse: collapse;
                font-family: Calibri, sans-serif;
            }
            tbody {
                border: solid medium;
            }
            td {
                border: solid thin;
                height: 1.4em;
                width: 1.4em;
                text-align: center;
                padding: 0;
            }
        </style>
        <table>
            <col><col><col>
            <col><col><col>
            <col><col><col>
    """

    table_footer = """
        </table>
    """

    table_body = ""
    for i in range(sudoku.shape[0]):
        table_body += "<tr>"
        for j in range(sudoku.shape[1]):
            if comparison is not None:
                cell_color = "" if sudoku[i, j] == comparison[i, j] else "background-color:#F8BBD0;"
            else:
                cell_color = ""

            border_style = "border-bottom: 3px solid black;" if (i + 1) % 3 == 0 else ""
            border_style += "border-right: 3px solid black;" if (j + 1) % 3 == 0 else ""

            table_body += f"<td style='{cell_color} {border_style}'>" + str(sudoku[i, j]) + "</td>"
        table_body += "</tr>"

    html = "<center>" + table_header + table_body + table_footer + "</center>"
    return html


def is_full(sudoku: np.ndarray) -> bool:
    """
    Check if the sudoku puzzle is full.

    :param sudoku: sudoku puzzle
    :return: boolean
    """

    return not np.any(sudoku == 0)


def list_possible_numbers(sudoku: np.ndarray, row: int, col: int, valid_inputs_only: bool) -> list:
    """
    List the possible numbers for a given position in the sudoku puzzle.
    If requested, only the valid numbers are returned.

    :param sudoku: sudoku puzzle
    :param row: cell row
    :param col: cell column
    :param valid_inputs_only: whether to return just the valid numbers or all of them
    :return: list of possible numbers
    """

    if valid_inputs_only:
        possible = set(range(1, 10))

        # Remove numbers in the same row, column, and 3x3 block
        possible -= set(sudoku[row])
        possible -= set(sudoku[:, col])
        possible -= set(
            sudoku[3*(row//3):3*(row//3)+3, 3*(col//3):3*(col//3)+3].flatten()
        )
    else:
        possible = range(1, 10)

    return list(possible)


def load_solutions(path: str) -> list:
    """
    Load all the solutions from folder to the session state.

    :param path: path to the folder
    :return: list of solutions
    """

    solutions = []
    for file_name in os.listdir(path):
        file_path = os.path.join(path, file_name)
        solutions.append(load_sudoku_board(file_path=file_path))

    return solutions


def load_sudoku_board(file_path) -> np.ndarray:
    """
    Load the sudoku puzzle from a text file, check the correctness, and parse it.

    :param file_path: path to the text file
    :return: None
    """

    correct = True
    sudoku = np.zeros((9, 9), dtype=int)

    with open(file_path, "r") as f:
        n_rows = 0
        for line in f:
            line = line.strip().split()
            if n_rows == 9 or len(line) != 9:
                correct = False
                break
            for num in line:
                if not num.isdigit() or not 0 <= int(num) <= 9:
                    correct = False
                    break
            sudoku[n_rows] = list(map(int, line))
            n_rows += 1
        if n_rows != 9:
            correct = False

    return sudoku if correct else None


def map_board_column(col: int) -> int:
    """
    Map the column index from the table to the actual index in the sudoku puzzle.

    :param col: column index of the printed table
    :return: column index of the board
    """

    return col-(col//4)


def save_sudoku_puzzle(sudoku: np.ndarray, file_path: str) -> None:
    """
    Save the current sudoku puzzle at the path specified.

    :param sudoku: sudoku puzzle
    :param file_path: path to the text file
    :return: None
    """

    with open(file_path, "w") as f:
        for row in sudoku:
            f.write(" ".join(map(str, row)) + "\n")


def get_hint_mask(sudoku: np.ndarray, solutions: list) -> tuple:
    """
    Check whether the inputs are correct or not.
    Place in one mask the cells that are correct in the closest solution,
    and in the other mask the cells that are correct in the others.
    This is a hint for the user.
    
    :param sudoku: sudoku puzzle
    :param solutions: list of solutions
    :return: None
    """

    n_solutions = len(solutions)
    matches = [0 for _ in range(n_solutions)]
    for l in range(n_solutions):
        for i in range(9):
            for j in range(9):
                if sudoku[i, j] == solutions[l][i, j]:
                    matches[l] += 1
    
    ref = matches.index(max(matches))
    mask1 = np.equal(sudoku, solutions[ref])
    mask2 = (
        np.logical_or.reduce([np.equal(sudoku, solutions[l]) for l in range(n_solutions) if l != ref])
        if n_solutions > 1 else np.zeros(sudoku.shape, dtype=bool)
    )
    mask2[np.equal(mask1, mask2)] = False

    return mask1, mask2