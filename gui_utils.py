from platform import system

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
    command = [file_name] if system == "windows" else ["wine64", file_name]
    command.extend(input)

    for attempt in range(retries):
        try:
            run_process = subprocess.run(command, capture_output=True, text=True, timeout=timeout)
            return run_process.returncode == 0
        except subprocess.TimeoutExpired:
            if attempt == retries - 1:
                return False
            continue
        except FileNotFoundError as e:
            if system != "windows" and "wine64" in str(e):
                # print("Wine is not installed: run 'sudo apt-get install wine64'")
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

def generate_fix_sudoku_html(sudoku: np.ndarray, comparison :np.ndarray=None) -> None:
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

            str_num = str(sudoku[i, j]) if sudoku[i, j] != 0 else " "
            table_body += f"<td style='{cell_color} {border_style}'>" + str_num + "</td>"
        table_body += "</tr>"

    html = "<center>" + table_header + table_body + table_footer + "</center>"
    return html


def generate_editable_sudoku_html(sudoku: np.ndarray) -> str:
    """
    Generate an editable HTML representation of a sudoku puzzle with background colors and disabled cells.

    :param sudoku: sudoku puzzle
    :return: HTML string
    """

    style = """
    <style>
        .sudoku-container {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 1em;
        }
        .sudoku-table {
            border-collapse: collapse;
            font-family: Calibri, sans-serif;
        }
        .sudoku-table tbody {
            border: solid 3px black;
        }
        .sudoku-table td {
            border: solid 1px black;
            height: 2em;
            width: 2em;
            padding: 0;
        }
        .sudoku-input {
            width: 100%;
            height: 100%;
            border: none;
            text-align: center;
            font-family: Arial, sans-serif;
            font-size: 1.2em;
            padding: 0;
            margin: 0;
            box-sizing: border-box;
        }
        .sudoku-input:focus:not([disabled]) {
            outline: none;
            background-color: #e8f0fe !important;
        }
        .sudoku-input:disabled {
            color: black;
            background-color: inherit;
        }
        #output-area {
            width: 30%;
            height: 50px;
            font-family: Calibri, sans-serif;
            white-space: pre;
            margin-top: 1em;
        }
    </style>
    """

    script = """
        <script>
        document.addEventListener('DOMContentLoaded', function() {
            const inputs = document.getElementsByClassName('sudoku-input');
            const outputArea = document.getElementById('output-area');
            let storedValues = {};

            function formatJSON(obj) {
                return JSON.stringify(obj, null, 1);
            }

            function updateOutput() {
                const formattedJson = formatJSON(storedValues);
                outputArea.value = formattedJson;
                window.parent.postMessage({
                    type: 'streamlit:setComponentValue',
                    value: formattedJson
                }, '*');
            }

            function moveFocus(input, direction) {
                let row = parseInt(input.getAttribute('data-row'));
                let col = parseInt(input.getAttribute('data-col'));

                switch(direction) {
                    case 'up':
                        row = (row - 1 + 9) % 9;
                        break;
                    case 'down':
                        row = (row + 1) % 9;
                        break;
                    case 'left':
                        col = (col - 1 + 9) % 9;
                        break;
                    case 'right':
                        col = (col + 1) % 9;
                        break;
                }

                let nextInput = document.querySelector(
                    `[data-row="${row}"][data-col="${col}"]`
                );
                if (nextInput) {
                    nextInput.focus();
                    nextInput.setSelectionRange(nextInput.value.length, nextInput.value.length);
                }
            }

            for (let input of inputs) {
                // Store initial values
                const row = input.getAttribute('data-row');
                const col = input.getAttribute('data-col');
                if (input.value) {
                    storedValues[`${row},${col}`] = input.value;
                }

                input.addEventListener('focus', function(e) {
                    if (!this.disabled) {
                        this.setSelectionRange(this.value.length, this.value.length);
                    }
                });

                input.addEventListener('click', function(e) {
                    if (!this.disabled) {
                        this.setSelectionRange(this.value.length, this.value.length);
                    }
                });

                input.addEventListener('input', function(e) {
                    if (!this.disabled) {
                        let value = this.value.replace(/[^1-9]/g, '');
                        if (value.length > 1) value = value[0];
                        this.value = value;

                        const row = this.getAttribute('data-row');
                        const col = this.getAttribute('data-col');
                        if (value) {
                            storedValues[`${row},${col}`] = value;
                        } else {
                            delete storedValues[`${row},${col}`];
                        }

                        updateOutput();
                    }
                });

                input.addEventListener('keydown', function(e) {
                    switch(e.key) {
                        case 'ArrowUp':
                            e.preventDefault();
                            moveFocus(this, 'up');
                            break;
                        case 'ArrowDown':
                            e.preventDefault();
                            moveFocus(this, 'down');
                            break;
                        case 'ArrowLeft':
                            e.preventDefault();
                            moveFocus(this, 'left');
                            break;
                        case 'ArrowRight':
                        case 'Enter':
                            e.preventDefault();
                            moveFocus(this, 'right');
                            break;
                    }
                });
            }

            // Initial output update
            updateOutput();
        });
        </script>
        """

    if st.session_state.solutions and st.session_state.highlight_correct:
        hint1, hint2 = get_hint_masks(sudoku=st.session_state.sudoku, solutions=st.session_state.solutions)
    else:
        hint1 = np.zeros((9, 9), dtype=bool)
        hint2 = np.zeros((9, 9), dtype=bool)

    table = "<table class='sudoku-table'>"
    for i in range(9):
        table += "<tr>"
        for j in range(9):

            border_style = []
            if (i + 1) % 3 == 0 and i < 8:
                border_style.append("border-bottom: 3px solid black")
            if (j + 1) % 3 == 0 and j < 8:
                border_style.append("border-right: 3px solid black")

            cell_color = get_background_color(hint1, hint2, i, j)
            disabled_attr = "disabled" if (st.session_state.freeze_config and
                                           st.session_state.sudoku_frz[i, j]) else ""
            cell_style = border_style + [f"background-color: {cell_color}"] if cell_color else border_style
            cell_style = "; ".join(cell_style)

            value = str(sudoku[i, j]) if sudoku[i, j] != 0 else ""

            table += f"""
                <td style="{cell_style}">
                    <input 
                        type="text" 
                        class="sudoku-input"
                        value="{value}"
                        data-row="{i}"
                        data-col="{j}"
                        maxlength="1"
                        pattern="[1-9]"
                        style="background-color: {cell_color if cell_color else 'inherit'}"
                        {disabled_attr}
                    >
                </td>
            """
        table += "</tr>"
    table += "</table>"

    return f"""
        <div class="sudoku-container">
            {style}
            {table}
            <textarea id="output-area" placeholder="Grid values will appear here"></textarea>
            {script}
        </div>
    """


def get_background_color(hint1: np.ndarray, hint2: np.ndarray, row: int, col: int) -> str:
    """
    Get the background color of the cell in the sudoku grid.

    :param hint1: first hint mask
    :param hint2: second hint mask
    :param row: row index
    :param col: column index
    :return: color
    """

    if st.session_state.freeze_config and st.session_state.sudoku_frz[row, col]:
        if hint1[row, col]:
            return "#99dd99"
        if hint2[row, col]:
            return "#ffd700"
        return "#f0f0f0"

    if hint1[row, col]:
        return "#00ff00"
    if hint2[row, col]:
        return "#ffff00"
    return "#ffffff"


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
    set_session_value(key="sudoku", value=st.session_state.sudoku_gen)


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
    :return: tuple of hints
    """

    hints = []
    hint = ""
    with open(path, "r") as f:
        for line in f:
            if line == "\n":
                if hint.endswith("\n"):
                    if hint.strip():
                        hints.append(hint)
                    hint = ""
                else:
                    hint += "\n" + line
            else:
                hint += "\n" + line
        if hint:
            hints.append(hint)
    return hints



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
