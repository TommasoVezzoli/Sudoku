from    custom_component import render_sudoku
from    gui_utils import *
import  numpy as np
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

    :param column: column to monitor the process
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
                    file_path = os.path.join(os.getcwd(), "src\\Tmp\\sudoku-tmp.txt")
                    with open(file_path, "wb") as f:
                        f.write(file.getbuffer())

                    sudoku = load_sudoku_board(file_path=file_path)
                    if sudoku is not None:
                        clear_sudoku()
                        set_session_value(key="sudoku", value=sudoku)
                        st.success("File loaded.")
                    else:
                        st.error("File is malformed.")
                except:
                    st.error("File is malformed.")


def check_solution(column) -> None:
    """
    Check if the current sudoku puzzle is a completed and the solution is valid.

    :param column: column to monitor the process
    :return: boolean
    """

    sudoku = st.session_state.sudoku
    with column:
        with stylable_container(
            key="check",
            css_styles="""
                .stAlert{
                    text-align: center
                }
            """
        ):
            if is_full(sudoku):
                if check_valid_sudoku(sudoku):
                    st.success("Sudoku is complete and valid.")
                else:
                    st.error("Sudoku is not valid.")
            else:
                st.warning("Sudoku is incomplete.")


def solve_sudoku(column) -> None:
    """
    Solve the current sudoku puzzle using the backtracking algorithm.

    :param column: column to monitor the process
    :return: None
    """

    cwd = os.getcwd()
    solutions_path = os.path.join(cwd, "src", "Solutions")
    tmp_path = os.path.join(cwd, "src", "Tmp")

    clear_folder(path=solutions_path)
    set_session_value(key="solutions", value=[])
    set_session_value(key="hints", value=[])

    if is_full(st.session_state.sudoku):
        return None
    with column:
        with st.spinner("Solving..."):
            save_sudoku_puzzle(
                sudoku=st.session_state.sudoku,
                file_path=os.path.join(tmp_path, "sudoku-tmp.txt")
            )
            execution = call_exe(
                file_name=os.path.join(cwd, "run_solver.exe.exe"),
                input=[str(os.path.join(tmp_path, "sudoku-tmp.txt")), str(solutions_path), str(tmp_path)],
            )

        with stylable_container(
            key="solve",
            css_styles="""
                .stAlert{
                    text-align: center
                }
            """
        ):
            if execution:
                solutions = load_solutions(path=solutions_path)
                hints = load_hints(path=os.path.join(tmp_path, "solver-actions.log"))
                if solutions:
                    set_session_value(key="solutions", value=solutions)
                    set_session_value(key="hints", value=hints)
                    set_session_value(key="sol_idx", value=0)
                    st.success("Solution(s) loaded.")
                else:
                    st.error("Could not solve the sudoku.")


def generate_sudoku(column) -> None:
    """
    Generate a sudoku puzzle with the desired level of difficulty.

    :param column: column to monitor the process
    :return: None
    """

    cwd = os.getcwd()
    seeds_path = os.path.join(cwd, "src", "Seeds")
    tmp_path = os.path.join(cwd, "src", "Tmp")

    with column:
        with stylable_container(
            key="generate",
            css_styles="""
                .stAlert{
                    text-align: center
                }
            """
        ):
            if st.session_state.sudoku_level is None:
                st.error("Please select a level.")
                return None

            with st.spinner("Generating..."):
                execution = call_exe(
                    file_name=os.path.join(cwd, "run_generator.exe"),
                    input=[str(st.session_state.sudoku_level), str(seeds_path), str(tmp_path)],
                    timeout=15
                )

            if execution:
                sudoku_gen = load_sudoku_board(file_path=os.path.join("src", "Tmp", "sudoku-gen.txt"))
                set_session_value(key="sudoku_gen", value=sudoku_gen)
                st.success("Sudoku generated.")


### ---------------------------------------------------------------------------------------------------- ###
### --- INTRODUCTION --- ###


st.markdown("<h1 style='text-align: center; font-weight: bold;'>Sudoku Solver & Generator</h1>", unsafe_allow_html=True)

st.markdown(
    """
    This web page hosts a **Sudoku Solver** in the first part and a **Sudoku Generator** in the second.  
    You can find the source code at this github [repo](https://github.com/ggiuliopirotta/Sudoku).
    
    P.S. Switch to light screen mode from the app setting for better visibility.
    """
)

st.write("---")


### ---------------------------------------------------------------------------------------------------- ###
### --- SOLVER & BOARD --- ###


st.markdown(
    """
    ### Solver
    This tool is able to solve any sudoku puzzle, whether it holds a single solution or multiple ones: in such case just the five that are most similar to each other are loaded. \\
    Here is an example of what a user can do:
    1. Upload a puzzle from a file formatted like [this](https://github.com/ggiuliopirotta/Sudoku/blob/main/sample-file.txt)
    2. Freeze the non-empty cells of the grid to avoid messing up the current configuration
    3. Attempt to solve the puzzle
    4. Ask the computer to provide a solution to the puzzle
    5. Follow the hints to solve it autonomously
    6. Check if the grid is valid
    
    For each non-trivial feature, a tooltip will provide further instructions.
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
        on_click=clear_sudoku
    )
with cols[1]:
    st.checkbox(
        key="freeze_config",
        label="Freeze configuration",
        value=st.session_state.freeze_config,
        on_change=set_freeze_mask,
    )

if st.session_state.solutions and st.session_state.highlight_correct:
    hint1, hint2 = get_hint_masks(sudoku=st.session_state.sudoku, solutions=st.session_state.solutions)
else:
    hint1 = np.zeros((9, 9), dtype=bool)
    hint2 = np.zeros((9, 9), dtype=bool)

colors = get_background_color(hint1=hint1, hint2=hint2)

# Render the grid and process updates
updated_cell = render_sudoku(st.session_state.sudoku, st.session_state.sudoku_frz, colors)
if updated_cell:
    row, col, value = updated_cell["row"], updated_cell["col"], updated_cell["value"]
    st.session_state.sudoku[row, col] = value
    st.rerun()

cols = st.columns((1, 1, 7), vertical_alignment="center")
with cols[0]:
    st.button(
        label="Check",
        help="Check if the current sudoku puzzle is valid",
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

                popup_cols = st.columns((6, 3, 1, 2), vertical_alignment="center")
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

            st.html(generate_sudoku_html(sudoku=solutions[sol_idx], comparison=st.session_state.sudoku))
            with open(os.path.join(os.getcwd(), "src", "Solutions", f"solution{sol_idx + 1}.txt")) as file:
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
                - yellow if the number is correct in at least one of the loaded solutions \\
                - green if any of these situations occur: \\
                    i. the number appears in the solution where most of the numbers are correct \\
                    ii. the number is correct \\
                - white if the number is incorrect
            """,
            value=st.session_state.highlight_correct
        )

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
    ### Generator
    
    This tool can generate sudoku of four different levels.  
    To use it, just insert the desired difficulty (from 1: easy, to 4: hard) and click the **Generate** button. \\
    Differently from most of the online generators, this one does not have pre-loaded puzzles, thus yielding a much more varied output.
    """
)

cols = st.columns((2, 1.5, 7), vertical_alignment="bottom")
col = st.columns((1))
with cols[0]:
    st.number_input(
        key="sudoku_level",
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
    st.html(generate_sudoku_html(st.session_state.sudoku_gen))

    cols = st.columns((1, 1.5, 6))
    with cols[0]:
        st.button(
            label="Import",
            help="Import the sudoku in the above solver section to solve it.",
            use_container_width=True,
            on_click=import_sudoku
        )
    with cols[1]:
        with open(os.path.join(os.getcwd(), "src", "Tmp", "sudoku-gen.txt")) as file:
            st.download_button(
                label="Download",
                data=file,
                file_name="generated.txt"
            )
