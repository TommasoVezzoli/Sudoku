import streamlit as st


### ---------------------------------------------------------------------------------------------------- ###
### --- STREAMLIT CONFIGURATION --- ###


# TODO: try implementing a custom CSS class for the sudoku board as this applies to the whole page
st.markdown("""
    <style>
    [data-testid="stVerticalBlock"] {
        padding: 0rem;
        margin: 0rem;
        gap: 0rem;
    }
    </style>
    """,
    unsafe_allow_html=True
)

# white placeholder to simulate the space between items
vline = """
    <div style='width: 80%; height: 15px; background-color: white; margin: 10px auto;'>
    </div>
"""

# TODO: have a look at the streamlit cache, because it might be an alternative to the session state
#  but it's not yet clear to me how to use it
if "init" not in st.session_state:
    st.session_state.init   = True
    st.session_state.sudoku = [[0 for _ in range(9)] for _ in range(9)]


### ---------------------------------------------------------------------------------------------------- ###
### --- HELPER FUNCTIONS --- ###


def map_board_column(col: int) -> int:
    """
    Maps the column index from the table to the actual index in the sudoku board.

    :param col: column index of the printed table
    :return: column index of the board
    """

    return col-(col//4)


@st.dialog("Input number")
def input_number(row: int, col: int) -> None:
    """
    Dialog to input a number in the selected position of the sudoku board.

    :param row: row index of the printed table
    :param col: column index of the printed table
    :return: None
    """

    num = st.number_input(
        label="Select number for position ({}, {})".format(row+1, map_board_column(col)+1),
        min_value=1,
        max_value=9,
        step=1
    )

    st.markdown(vline, unsafe_allow_html=True)

    if st.button(label="Input"):
        st.session_state.sudoku[row][map_board_column(col)] = num
        st.rerun()


### ---------------------------------------------------------------------------------------------------- ###
### --- SUDOKU BOARD --- ###


def generate_sudoku_block(block_idx: int, n_rows: int=3) -> None:
    """
    Generate and display a block of the sudoku board.
    The first 2 blocks are closed with a white space to simulate the separation between blocks.

    :param block_idx: block index
    :param n_rows: number of rows to display
    :return: None
    """

    _, c, _ = st.columns((0.5, 1, 0.5))
    grid = c.columns((0.07, 0.07, 0.07, 0.035, 0.07, 0.07, 0.07, 0.035, 0.07, 0.07, 0.07, 0.01))

    for row in range(3*block_idx, 3*block_idx+n_rows):
        for col in range(11):
            with grid[col]:
                if col in (3, 7):
                    continue

                st.button(
                    key=f"sudoku_{row}{col}",
                    label=str(st.session_state.sudoku[row][map_board_column(col)]),
                    on_click=input_number,
                    args=(row, col)
                )

    if block_idx < 2:
        with c:
            st.markdown(vline, unsafe_allow_html=True)


for block_idx in range(3):
    generate_sudoku_block(block_idx=block_idx)
