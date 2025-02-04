import  numpy as np
import  os
import  platform
import  streamlit as st
import  subprocess


def call_exe(file_name: str, input: list, timeout: int=60, retries=5) -> bool:
    """
    Call an executable receiving arguments or input files.

    :param file_name: name of the executable
    :param input: arguments
    :param timeout: time limit
    :param retries: number of retries
    :return:
    """

    system = platform.system().lower()
    file_name = file_name.rsplit('.', 1)[0] if '.exe' in file_name else file_name
    command = ["wine64", file_name] if system != "windows" else [file_name]
    command.extend(input)

    for attempt in range(retries):
        try:
            run_process = subprocess.run(command, capture_output=True, text=True, timeout=timeout)
            return run_process.returncode == 0
        except subprocess.TimeoutExpired:
            if attempt == retries - 1:
                return False
            continue
        except Exception:
            return False


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


def clear_folder(path: str) -> None:
    """
    Clear all the visible files in the given folder.

    :param path: path to the folder
    :return: None
    """

    # Check if the path exists
    if not os.path.exists(path):
        return

    for file_name in os.listdir(path):
        if not file_name.startswith('.'):
            file_path = os.path.join(path, file_name)
            if os.path.isfile(file_path):
                os.remove(file_path)


def clear_sudoku() -> None:
    """
    Clear the sudoku puzzle and its solutions.

    :return: None
    """

    set_session_value(key="sudoku", value=np.zeros((9, 9), dtype=int))
    set_session_value(key="sudoku_frz", value=np.zeros((9, 9), dtype=bool))
    set_session_value(key="freeze_config", value=False)
    set_session_value(key="solutions", value=[])
    set_session_value(key="hints", value=[])


def empty_folder(path: str) -> None:
    """
    Remove all files in the specified folder.

    :param path: path to the folder
    :return: None
    """

    for file_name in os.listdir(path):
        file_path = os.path.join(path, file_name)
        if os.path.isfile(file_path):
            os.remove(file_path)


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
                font-family: Arial, sans-serif;
            }
            # tbody {
            #     border: solid medium;
            # }
            td {
                border: solid thin;
                font-size: 1em;
                text-align: center;
                padding: 0;
                height: 24px;
                width: 26px;
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

            border_style = ""
            if (i + 1) % 3 == 0 and i < 8:
                border_style += "border-bottom: 3px solid black;"
            if (j + 1) % 3 == 0 and j < 8:
                border_style += "border-right: 3px solid black;"

            str_num = str(sudoku[i, j]) if sudoku[i, j] != 0 else " "
            table_body += f"<td style='{cell_color} {border_style}'>" + str_num + "</td>"
        table_body += "</tr>"

    html = "<center>" + table_header + table_body + table_footer + "</center>"
    return html


def get_background_color(hint1: np.ndarray, hint2: np.ndarray) -> list:
    """
    Get the background color for each cell in the sudoku grid.

    :param hint1: first hint mask
    :param hint2: second hint mask
    :return: color
    """

    colors = []
    for i in range(9):
        row_colors = []
        for j in range(9):
            if st.session_state.freeze_config and st.session_state.sudoku_frz[i, j]:
                if hint1[i, j]:
                    row_colors.append("#99dd99")
                elif hint2[i, j]:
                    row_colors.append("#ffd700")
                else:
                    row_colors.append("#f0f0f0")
            else:
                if hint1[i, j]:
                    row_colors.append("#00ff00")
                elif hint2[i, j]:
                    row_colors.append("#ffff00")
                else:
                    row_colors.append("#ffffff")
        colors.append(row_colors)

    return colors


def get_hint_masks(sudoku: np.ndarray, solutions: list) -> tuple:
    """
    Check whether the inputs are correct or not.
    Place in one mask the cells that are correct in the closest solution,
    and in the other mask the cells that are correct in the others.
    This is a hint for the user.

    :param sudoku: sudoku puzzle
    :param solutions: list of solutions
    :return:
    """

    n_solutions = len(solutions)
    matches = [0 for _ in range(n_solutions)]
    for l in range(n_solutions):
        for i in range(9):
            for j in range(9):
                if sudoku[i, j] == solutions[l][i, j]:
                    matches[l] += 1

    reference = matches.index(max(matches))
    mask1 = np.equal(sudoku, solutions[reference])
    mask2 = (
        np.logical_or.reduce([np.equal(sudoku, solutions[l]) for l in range(n_solutions) if l != reference])
        if n_solutions > 1 else np.zeros(sudoku.shape, dtype=bool)
    )
    mask2[np.equal(mask1, mask2)] = False

    return mask1, mask2


def is_full(sudoku: np.ndarray) -> bool:
    """
    Check if the sudoku puzzle is full.

    :param sudoku: sudoku puzzle
    :return: boolean
    """

    return not np.any(sudoku == 0)


def import_sudoku():
    """
    Import the generated sudoku puzzle into the solver section.
    """

    clear_sudoku()
    set_session_value(key="sudoku", value=st.session_state.sudoku_gen.copy())


def list_possible_numbers(sudoku: np.ndarray, row: int, col: int, hide_invalid: bool) -> list:
    """
    List the possible numbers for a given position in the sudoku puzzle.
    If requested, only the valid numbers are returned.

    :param sudoku: sudoku puzzle
    :param row: cell row
    :param col: cell column
    :param hide_invalid: whether to return just the valid numbers or all of them
    :return: list of possible numbers
    """

    if hide_invalid:
        possible = set(range(1, 10))
        possible -= set(sudoku[row])
        possible -= set(sudoku[:, col])
        possible -= set(
            sudoku[3*(row//3):3*(row//3)+3, 3*(col//3):3*(col//3)+3].flatten()
        )
    else:
        possible = range(1, 10)

    return list(possible)


def load_hints(path: str) -> list:
    """
    Load the hints for the current sudoku from the folder to the session state.

    :param path: path to the folder
    :return: list of hints
    """

    with open(path, "r") as file:
        content = file.read()

    lines = [line.strip() for line in content.split('\n') if line.strip()]
    formatted_hints = []
    current_hint = []

    for line in lines:
        if not line:
            continue

        # Process a new hint and add bullet points if it extends to more than one line
        if current_hint and not line.startswith(current_hint[0].split(':')[0]):
            if len(current_hint) == 1:
                formatted_hints.append(current_hint[0])
            else:
                hint_type = current_hint[0].split(':')[0]
                instructions = [f"* {line.split(':', 1)[1].strip()}" for line in current_hint]
                formatted_hints.append(f"{hint_type}:\n" + "\n".join(instructions))
            current_hint = []

        current_hint.append(line)

    # Process the last hint if it exists
    if current_hint:
        if len(current_hint) == 1:
            formatted_hints.append(current_hint[0])
        else:
            hint_type = current_hint[0].split(':')[0]
            instructions = [f"* {line.split(':', 1)[1].strip()}" for line in current_hint]
            formatted_hints.append(f"{hint_type}:\n" + "\n".join(instructions))

    return formatted_hints



def load_solutions(path: str) -> list:
    """
    Load all the solutions from folder to the session state.

    :param path: path to the folder
    :return: list of solutions
    """

    solutions = []
    for file_name in os.listdir(path):
        if not file_name.startswith('.'):
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


def set_freeze_mask() -> None:
    """
    Produce a mask to freeze the configuration of the sudoku puzzle.
    This enables the user to solve the puzzle autonomously without messing up the initial configuration.

    :return: None
    """

    set_session_value(
        key="sudoku_frz",
        value=st.session_state.sudoku != 0 if st.session_state.freeze_config else(
            np.zeros((9, 9), dtype=bool))
    )


def set_session_value(key: str, value=None) -> None:
    """
    Set the value of a session state variable.
    If no value is provided, then it means that the variable is a boolean and will be toggled.

    :param key: name of the session state variable
    :param value: value to set
    :return: None
    """

    if value is None:
        value = not st.session_state[key]
    st.session_state[key] = value
