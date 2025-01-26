import os
import streamlit.components.v1 as components

_component_func = components.declare_component(
    "sudoku_component",
    path=os.path.dirname(__file__)
)

def render_sudoku(sudoku):
    """
    Render the Sudoku grid using the custom component.
    :param sudoku: A 2D list representing the Sudoku grid
    :return: Updated cell (row, col, value) as a dictionary
    """
    return _component_func(spec=sudoku, default=None)