import  numpy as np
from    gui_utils import *
import  json
import  os
from    streamlit_extras.stylable_container import stylable_container
import  streamlit as st


### ---------------------------------------------------------------------------------------------------- ###
### --- STREAMLIT CONFIGURATION --- ###


if "init" not in st.session_state:
    st.session_state.init               = True
    st.session_state.sudoku             = np.zeros((9, 9), dtype=int)
    st.session_state.sudoku_frz         = np.zeros((9, 9), dtype=bool)
    st.session_state.sudoku_gen         = np.zeros((9, 9), dtype=int)
    st.session_state.tmp_values         = ""
    st.session_state.freeze_config      = False
    st.session_state.highlight_correct  = False
    st.session_state.solutions          = []
    st.session_state.sol_idx            = 0
    st.session_state.hints              = []
    st.session_state.hints_idx          = 0


### ---------------------------------------------------------------------------------------------------- ###
### --- HELPER FUNCTIONS --- ###


def handle_file_upload(column) -> None:
    """
    Handle the upload of a sudoku puzzle from a text file through the file uploader.

    :param column: column to display the result
    :return: None
    """

    file = st.session_state.uploaded_file
    if file:
        with column:
            with stylable_container(
                key="upload",
                css_styles="""
                .stAlert{
                    text-align: center
                }
                """
            ):
                try:
                    cwd = os.getcwd()
                    file_path = os.path.join(cwd, "sudoku_tmp.txt")
                    with open(file_path, "wb") as f:
                        f.write(file.getbuffer())

                    sudoku = load_sudoku_board(file_path=file_path)
                    if sudoku is not None:
                        set_session_value(key="solutions", value=[])
                        set_session_value(key="sudoku", value=sudoku)
                        # set_session_value(key="freeze_config", value=True)
                        set_freeze_mask()
                        st.success("File loaded correctly.")
                    else:
                        st.error("File is malformed.")
                except:
                    st.error("File is malformed.")


def check_solution(column) -> None:
    """
    Check if the current sudoku puzzle is a completed and the solution is valid.

    :param column: column to display the result
    :return: boolean
    """

    sudoku = st.session_state.sudoku
    with column:
        with stylable_container(
                key="check",
                css_styles="""
                .stAlert{
                    text-align: center
                }"""
        ):
            if is_full(sudoku):
                st.success("The sudoku is complete and valid.") if check_valid_sudoku(sudoku) else(
                    st.error("The sudoku is invalid."))
            else:
                st.warning("The sudoku is incomplete.")


@st.dialog("Input number")
def input_number(row: int, col: int) -> None:
    """
    Dialog widget to input a number in the selected position of the sudoku puzzle.

    :param row: row index of the printed table
    :param col: column index of the printed table
    :return: None
    """

    cols = st.columns(9)
    for i in range(1, 10):
        with cols[i-1]:
            if i in list_possible_numbers(
                sudoku=st.session_state.sudoku,
                row=row,
                col=map_board_column(col),
                hide_invalid=st.session_state.hide_invalid
            ):
                if st.button(label=str(i)):
                    st.session_state.sudoku[row, map_board_column(col)] = i
                    st.rerun()

    if st.session_state.sudoku[row, map_board_column(col)]:
        if st.button(label="Clear"):
            st.session_state.sudoku[row, map_board_column(col)] = 0
            st.rerun()


# def print_sudoku_block(block_idx: int, n_rows: int=3) -> None:
#     """
#     Generate and display a block of the sudoku puzzle.
#     The first 2 blocks are closed with a white space to simulate the separation between blocks.
#
#     :param block_idx: block index
#     :param n_rows: number of rows to display
#     :return: None
#     """
#
#     _, c, _ = st.columns((5, 10, 5))
#     grid = c.columns((7, 7, 7, 3.5, 7, 7, 7, 3.5, 7, 7, 7, 1))
#
#     if st.session_state.solutions and st.session_state.highlight_correct:
#         hint1, hint2 = get_hint_masks(sudoku=st.session_state.sudoku, solutions=st.session_state.solutions)
#     else:
#         hint1 = np.zeros((9, 9), dtype=bool)
#         hint2 = np.zeros((9, 9), dtype=bool)
#
#     for row in range(3*block_idx, 3*block_idx+n_rows):
#         for col in range(11):
#             with grid[col]:
#                 if col in (3, 7):
#                     continue
#
#                 map_col = map_board_column(col)
#                 num = st.session_state.sudoku[row, map_col]
#                 text = str(num) if num != 0 else "_"
#
#                 background_color = get_background_color(hint1, hint2, row, map_col)
#                 margin_top = "0rem" if row%3 == 0 else "-1rem"
#                 margin_bottom = "0rem" if (row-2)%3 == 0 else "-1rem"
#
#                 with stylable_container(
#                     key=f"sudoku_{row}{col}",
#                     css_styles=f"""
#                         button {{
#                             font-size: 1rem;
#                             font-weight: bold;
#                             background-color: {background_color};
#                             color: black;
#                             margin-top: {margin_top};
#                             margin-bottom: {margin_bottom};
#                         }}
#                     """
#                 ):
#                     st.button(
#                         key=f"sudoku_{row}{col}",
#                         label=text,
#                         disabled=st.session_state.freeze_config and st.session_state.sudoku_frz[row, map_board_column(col)],
#                         on_click=input_number,
#                         args=(row, col)
#                     )


def solve_sudoku(column) -> None:
    """
    Solve the current sudoku puzzle using the backtracking algorithm.
    To do so, save the puzzle to a text file, call the solver, and load back the solutions.

    :param column: column to display the process
    :return: None
    """

    set_session_value(key="solutions", value=[])
    set_session_value(key="hints", value=[])
    empty_folder("Solutions")
    if is_full(st.session_state.sudoku):
        return None

    with column:
        with st.spinner("Solving..."):
            save_sudoku_puzzle(
                sudoku=st.session_state.sudoku,
                file_path="sudoku_tmp.txt"
            )
            execution = call_exe(
                file_name="run_backtrack.exe",
                input_file="sudoku_tmp.txt"
            )

        with stylable_container(
                key="solve",
                css_styles="""
                .stAlert{
                    text-align: center
                }"""
        ):
            if execution:
                solutions = load_solutions(path="Solutions")
                hints = load_hints(path="solver_actions.log")
                if solutions:
                    set_session_value(key="solutions", value=solutions)
                    set_session_value(key="hints", value=hints)
                    set_session_value(key="sol_idx", value=0)
                    st.success("Solution(s) loaded successfully.")
                else:
                    st.error("Could not solve the sudoku.")


def generate_sudoku(column) -> None:
    """
    Generate a sudoku puzzle with the desired level of difficulty.

    :param column: column to display the process
    :return: None
    """

    with column:
        with stylable_container(
                key="generate",
                css_styles="""
                        .stAlert{
                            text-align: center
                        }"""
        ):
            if st.session_state.difficulty_level is None:
                st.error("Please select a difficulty level.")
                return None

            with st.spinner("Generating..."):
                execution = call_exe(
                    file_name="run_generator.exe",
                    input_file=str(st.session_state.difficulty_level)
                )

            if execution:
                sudoku_gen = load_sudoku_board(file_path="sudoku_gen.txt")
                set_session_value(key="sudoku_gen", value=sudoku_gen)
                st.success("Sudoku generated successfully.")


### ---------------------------------------------------------------------------------------------------- ###
### --- INTRODUCTION --- ###


st.markdown("<h1 style='text-align: center; font-weight: bold;'>Sudoku Solver & Generator</h1>", unsafe_allow_html=True)

st.markdown(
    """
    This web page hosts a **Sudoku Solver** in the first part and a **Sudoku Generator** in the second.  
    Check out the instructions!
    
    **P.S.** We suggest setting the browser zoom level to 100% (default). \\
    You can find the source code at this github [repo](https://github.com/ggiuliopirotta/Sudoku).
    """
)

st.write("---")


### ---------------------------------------------------------------------------------------------------- ###
### --- SOLVER & BOARD --- ###


st.markdown(
    """
    #### Solver
    This tool is able to solve any sudoku puzzle, whether it holds a single solution or multiple ones: in such case just the five that are most similar to each other are loaded. \\
    When the **Solve** button is pressed, a popup window with the solutions will appear.
    
    Here is a recap of the main functionalities in this section:
    - Upload a puzzle from a text file like [this](https://github.com/ggiuliopirotta/Sudoku/blob/main/test.txt)
    - Freeze the non-empty cells of the grid to avoid messing up the current configuration
    - Check if the grid is valid
    - Solve the sudoku
    
    For the other features, read the tooltip hovering over with the mouse.
    """
)

cols = st.columns((6, 4), vertical_alignment="bottom")
with cols[0]:
    st.file_uploader(
        key="uploaded_file",
        label="Upload a sudoku puzzle",
        type=["txt"],
        on_change=handle_file_upload,
        args=(cols[1],)
    )

cols = st.columns((1, 9), vertical_alignment="center")
with cols[0]:
    st.button(
        label="Clear",
        help="Clear the whole sudoku grid",
        use_container_width=True,
        on_click=clear
    )
with cols[1]:
    st.checkbox(
        key="freeze_config",
        label="Freeze current configuration",
        value=st.session_state.freeze_config,
        on_change=set_freeze_mask,
    )

# for block_idx in range(3):
#     print_sudoku_block(block_idx=block_idx)

cols = st.columns([3, 2])
with cols[0]:
    grid_html = generate_editable_sudoku_html(st.session_state.sudoku)
    st.components.v1.html(grid_html, height=400)

with cols[1]:
    st.markdown("")
    with st.expander("Confirm grid values", expanded=True):
        tmp_values = st.text_area(
            "Grid Values",
            st.session_state.tmp_values,
            height=100,
            label_visibility="collapsed"
        )
        if st.button("Update"):
            with stylable_container(
                    key="update",
                    css_styles="""
                            .stAlert{
                                text-align: center
                            }
                            """
            ):
                try:
                    if tmp_values:
                        updates = json.loads(tmp_values)
                        for pos, value in updates.items():
                            row, col = map(int, pos.split(","))
                            st.session_state.sudoku[row, col] = int(value)
                        st.success("Grid updated successfully!")
                        st.rerun()
                    else:
                        st.warning("No values to update.")
                except json.JSONDecodeError:
                    st.error("Invalid grid values format.")
                except Exception as e:
                    st.error(f"Error updating the grid")

cols = st.columns((1, 1, 7), vertical_alignment="center")
with cols[0]:
    st.button(
        label="Check",
        use_container_width=True,
        on_click=check_solution,
        args=(cols[2],)
    )
with cols[1]:
        st.button(
            label="Solve",
            use_container_width=True,
            on_click=solve_sudoku,
            args=(cols[2],)
        )


### ---------------------------------------------------------------------------------------------------- ###
### --- SOLUTIONS --- ###


if st.session_state.solutions:
    cols = st.columns((4, 5, 2, 2), vertical_alignment="center")

    with cols[0]:
        with st.popover(
            label="Show possible solutions",
            use_container_width=True
        ):
            with stylable_container(
                key="solution_popup",
                css_styles="""
                    button {
                        border: none;
                    }
                """
            ):
                solutions = st.session_state.solutions
                sol_idx = st.session_state.sol_idx

                popup_cols = st.columns((5, 2, 1, 2), vertical_alignment="center")
                with popup_cols[0]:
                    st.markdown(f"Solution {sol_idx+1} of {len(solutions)}")
                with popup_cols[1]:
                    st.button(
                        key="sol_prev",
                        label="⬅️",
                        disabled=(sol_idx == 0),
                        on_click=set_session_value,
                        args=("sol_idx", sol_idx-1)
                    )
                with popup_cols[2]:
                    st.button(
                        key="sol_next",
                        label="➡️",
                        disabled=(sol_idx == len(solutions)-1),
                        on_click=set_session_value,
                        args=("sol_idx", sol_idx+1)
                    )

            st.html(generate_fix_sudoku_html(solutions[sol_idx], st.session_state.sudoku))
            with open(f"Solutions/solution{sol_idx+1}.txt") as file:
                st.download_button(
                    label="Download",
                    data=file,
                    file_name="solution.txt"
                )

    with cols[1]:
        st.checkbox(
            key="highlight_correct",
            label="Highlight correct cells",
            help="""Color the grid cells according to the following rules: \\
                - white if the number is incorrect \\
                - yellow if the number is correct in at least one of the loaded solutions \\
                - green if any of these situations occur: \\
                    i. the number appears in the solution where most of the numbers are correct \\
                    ii. the number is correct
            """,
            value=st.session_state.highlight_correct
        )
    with cols[2]:
        if st.button(
            label="Reload",
            use_container_width=True,
        ):
            st.rerun()

    if st.session_state.hints:
        with st.popover(
                label="Show hints",
                use_container_width=True
            ):
                with stylable_container(
                    key="solution_popup",
                    css_styles="""
                        button {
                            border: none;
                        }
                    """
                ):
                    hints = st.session_state.hints
                    hints_idx = st.session_state.hints_idx

                    popup_cols = st.columns((20, 2, 1, 1), vertical_alignment="center")
                    with popup_cols[0]:
                        st.markdown(f"Hint {hints_idx+1} of {len(hints)}")
                    with popup_cols[1]:
                        st.button(
                            key="hint_prev",
                            label="⬅️",
                            disabled=(hints_idx == 0),
                            on_click=set_session_value,
                            args=("hints_idx", hints_idx-1)
                        )
                    with popup_cols[2]:
                        st.button(
                            key="hint_next",
                            label="➡️",
                            disabled=(hints_idx == len(hints)-1),
                            on_click=set_session_value,
                            args=("hints_idx", hints_idx+1)
                        )
                st.markdown(st.session_state.hints[hints_idx])

st.write("---")


### ---------------------------------------------------------------------------------------------------- ###
### --- GENERATOR --- ###


st.markdown(
    """
    #### Generator
    
    This tool can generate sudoku of four different levels. \\
    To use it, just insert the desired difficulty (from 1: easy, to 4: hard) and click the **Generate** button.
    """
)

cols = st.columns((2, 1.5, 7), vertical_alignment="bottom")
col = st.columns((1))
with cols[0]:
    st.number_input(
        key="difficulty_level",
        label="Enter level",
        min_value=1,
        max_value=4,
    )
with cols[1]:
    st.button(
        label="Generate",
        use_container_width=True,
        on_click=generate_sudoku,
        args=(col[0],)
    )

if np.any(st.session_state.sudoku_gen):
    st.html(generate_fix_sudoku_html(st.session_state.sudoku_gen))

    cols = st.columns((1, 1.5, 6))
    with cols[0]:
        st.button(
            label="Import",
            help="Import the sudoku in the above solver section to solve it.",
            use_container_width=True,
            on_click=import_sudoku
        )
    with cols[1]:
        with open(f"sudoku_gen.txt") as file:
            st.download_button(
                label="Download",
                data=file,
                file_name="generated.txt"
            )
