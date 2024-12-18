import numpy as np
from streamlit_extras.stylable_container import stylable_container
import os
import streamlit as st

from gui_utils import *


### ---------------------------------------------------------------------------------------------------- ###
### --- STREAMLIT CONFIGURATION --- ###


# TODO: try implementing a custom CSS class for the sudoku puzzle section to improve the layout


if "init" not in st.session_state:
    st.session_state.init = True
    st.session_state.sudoku = np.zeros((9, 9), dtype=int)
    st.session_state.freezed_sudoku = np.zeros((9, 9), dtype=bool)
    st.session_state.freeze_configuration = False
    st.session_state.highlight_correct_cells = False
    st.session_state.valid_inputs_only = False
    st.session_state.solutions = []
    st.session_state.sol_idx = 0


### ---------------------------------------------------------------------------------------------------- ###
### --- HELPER FUNCTIONS --- ###


def check_solution(column) -> None:
    """
    Check if the current sudoku puzzle is a completed and the solution is valid.

    :param column: column to display the result
    :return: boolean
    """

    sudoku = st.session_state.sudoku
    with (column):
        if is_full(sudoku):
            st.success("Sudoku is completed and valid.") if check_valid_sudoku(sudoku) else(
                st.error("Sudoku is invalid."))
        else:
            st.warning("Sudoku is incomplete.")


def get_freeze_mask() -> np.ndarray:
    """
    Produce a mask to freeze the configuration of the sudoku puzzle.
    This enables the user to solve the puzzle autonomously without messing up the initial configuration.

    :return: boolean mask
    """

    set_session_value("freezed_sudoku", st.session_state.sudoku != 0 if st.session_state.freeze_configuration else np.zeros((9, 9), dtype=bool))


def handle_file_upload(column) -> None:
    """
    Handle the upload of a sudoku puzzle from a text file through the file uploader.

    :param column: column to display the result
    :return: None
    """

    file = st.session_state.uploaded_file

    if file:
        cwd = os.getcwd()
        file_path = os.path.join(cwd, "sudoku_tmp.txt")
        with open(file_path, "wb") as f:
            f.write(file.getbuffer())

        sudoku = load_sudoku_board(file_path=file_path)

        with column:
            if sudoku is not None:
                set_session_value("sudoku", sudoku)
                st.success("File loaded correctly.")
            else:
                st.error("File is not formatted correctly.")


@st.dialog("Input number")
def input_number(row: int, col: int) -> None:
    """
    Dialog widget to input a number in the selected position of the sudoku puzzle.

    :param row: row index of the printed table
    :param col: column index of the printed table
    :return: None
    """

    num = st.selectbox(
        label="Select number for position ({}, {})".format(row+1, map_board_column(col)+1),
        options=[None] + list_possible_numbers(
            sudoku=st.session_state.sudoku,
            row=row,
            col=map_board_column(col),
            valid_inputs_only=st.session_state.valid_inputs_only
        )
    )

    if st.button(label="Input"):
        if num is None:
            num = 0
        st.session_state.sudoku[row, map_board_column(col)] = num
        st.rerun()


def set_session_value(key: str, value=None) -> None:
    """
    Set the value of a session state variable.

    :param key: name of the session state variable
    :param value: value to set
    :return: None
    """

    if value is None:
        value = not st.session_state[key]
    st.session_state[key] = value


def solve_sudoku(column) -> None:
    """
    Solve the current sudoku puzzle using the backtracking algorithm.

    :param column: column to display the process
    :return: None
    """

    # Check if the board is full
    if is_full(st.session_state.sudoku):
        return None

    # Save the sudoku puzzle to a text file and launch the solver
    with column:
        with st.spinner("Solving..."):

            save_sudoku_puzzle(st.session_state.sudoku, file_path="sudoku_tmp.txt")
            execution = call_c_file(file_name="run_backtrack.exe", input_file="sudoku_tmp.txt")

        if execution:
            solutions = load_solutions(path="Solutions")
            set_session_value("sol_idx", 0)
            set_session_value("solutions", solutions)
            st.success("Solution(s) loaded successfully.")
        else:
            st.error("Error solving the sudoku puzzle.")


### ---------------------------------------------------------------------------------------------------- ###
### --- SUDOKU BOARD --- ###


def generate_sudoku_block(block_idx: int, n_rows: int=3) -> None:
    """
    Generate and display a block of the sudoku puzzle.
    The first 2 blocks are closed with a white space to simulate the separation between blocks.

    :param block_idx: block index
    :param n_rows: number of rows to display
    :return: None
    """

    _, c, _ = st.columns((0.5, 1, 0.5))
    grid = c.columns((0.07, 0.07, 0.07, 0.035, 0.07, 0.07, 0.07, 0.035, 0.07, 0.07, 0.07, 0.01))

    # mask = get_freeze_mask() if st.session_state.freeze_configuration else (
    #     np.zeros((9, 9), dtype=bool))
    
    hint1, hint2 = (
        get_hint_mask(st.session_state.sudoku, st.session_state.solutions)
        if st.session_state.solutions and st.session_state.highlight_correct_cells else (
            np.zeros((9, 9), dtype=bool),
            np.zeros((9, 9), dtype=bool)
        )
    )

    # The problem is that the mask works in the opposite way: mask 1 indicates white background, mask 0 indicates grey
    # print("ciao", mask, mask.shape, hint1, hint1.shape)
    # hint1[np.equal(mask, hint1)] = False
    # hint2[np.equal(mask, hint2)] = False
    # print(hint2)

    for row in range(3*block_idx, 3*block_idx+n_rows):
        for col in range(11):
            with grid[col]:
                if col in (3, 7):
                    continue

                map_col = map_board_column(col)
                num = st.session_state.sudoku[row, map_col]
                text = str(num) if num != 0 else "_"

                if st.session_state.freeze_configuration and st.session_state.freezed_sudoku[row, map_col]:
                    if hint1[row, map_col]:
                        background_color = "#99dd99"
                    elif hint2[row, map_col]:
                        background_color = "#ffd700"
                    else:
                        background_color = "#f0f0f0"
                elif hint1[row, map_col]:
                    background_color = "#00ff00"
                elif hint2[row, map_col]:
                    background_color = "#ffff00"
                else:
                    background_color = "#ffffff" 
                

                with stylable_container(
                        key=f"sudoku_{row}{col}",
                        css_styles=f"""
                                    button {{
                                        font-size: 1.5rem;
                                        font-weight: bold;
                                        background-color: {background_color};
                                        color: black;
                                    }}
                                """
                ):
                    st.button(
                        key=f"sudoku_{row}{col}",
                        label=text,
                        on_click=input_number,
                        args=(row, col),
                        disabled=st.session_state.freeze_configuration and st.session_state.freezed_sudoku[row, map_board_column(col)]
                    )


for block_idx in range(3):
    generate_sudoku_block(block_idx=block_idx)

st.write(st.session_state.freezed_sudoku)
st.write("---")


### ---------------------------------------------------------------------------------------------------- ###
### --- STREAMLIT COMPONENTS --- ###


valid_inputs = st.checkbox(
    label="Show valid inputs only",
    key="valid_inputs_only",
    value=st.session_state.valid_inputs_only
)

cols = st.columns((0.3, 0.7))
with cols[0]:
    st.button(
        label="Check solution",
        on_click=check_solution,
        args=(cols[1],)
    )

cols = st.columns((0.6, 0.4))
with cols[0]:
    st.file_uploader(
        key="uploaded_file",
        label="Upload a sudoku puzzle",
        type=["txt"],
        on_change=handle_file_upload,
        args=(cols[1],)
    )

cols = st.columns((0.3, 0.7))
with cols[0]:
    with stylable_container(
        key="solve_button",
        css_styles="""
            button {
                background-color: green;
                color: white;
            }
        """
    ):
        st.button(
            label="Solve",
            on_click=solve_sudoku,
            args=(cols[1],)
        )

if st.session_state.solutions:
    # with stylable_container(
    #     key="solution_popup",
    #     css_styles="""
    #         popover {
    #             background-color: white;
    #             border: 2px solid black;
    #             border-radius: 10px;
    #             padding: 20px;
    #             box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.3);
    #             width: 100px;
    #             max-width: 90%;
    #         }
    #     """
    # ):
    with st.popover("Show solution"):


        with stylable_container(
            key="solution_popup",
            css_styles="""
                button {
                    border: none;
                }
            """
        ):
            cols = st.columns((0.5, 0.2, 0.1, 0.2), vertical_alignment="center")

            solutions = st.session_state.solutions
            sol_idx = st.session_state.sol_idx
            with cols[0]:
                st.markdown(f"Solution {sol_idx+1} of {len(solutions)}")
            with cols[1]:
                st.button(
                    label="⬅️",
                    on_click=set_session_value,
                    args=("sol_idx", sol_idx-1),
                    disabled=(sol_idx == 0)
                )
            with cols[2]:
                st.button(
                    label="➡️",
                    on_click=set_session_value,
                    args=("sol_idx", sol_idx+1),
                    disabled=(sol_idx == len(solutions)-1)
                )
        st.html(generate_sudoku_html(solutions[sol_idx], st.session_state.sudoku))
        with open(f"Solutions/solution{sol_idx+1}.txt") as file:
            st.download_button(
                label="Download solution",
                data=file,
                file_name="solution.txt"
            )

st.checkbox(
    label="Freeze current configuration",
    key="freeze_configuration",
    value=st.session_state.freeze_configuration,
    on_change=get_freeze_mask
)

st.checkbox(
    label="Highlight correct cells",
    key="highlight_correct_cells",
    value=st.session_state.highlight_correct_cells
)
