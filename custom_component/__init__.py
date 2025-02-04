import os
import streamlit.components.v1 as components

_component_func = components.declare_component(
    "custom_component",
    path=os.path.dirname(__file__)
)

def render_sudoku(sudoku, freeze, colors=None):
    """
    Render the Sudoku grid using an HTML streamlit custom component.

    :param sudoku: 2D list for the grid.
    :param freeze: 2D boolean mask for non-editable cells.
    :param colors: Optional 2D list of hex color codes for each cell background.
    :return: Updated cell.
    """

    if colors is None:
        colors = [["#ffffff" for _ in range(9)] for _ in range(9)]

    return _component_func(
        spec={
            "sudoku": sudoku.tolist(),
            "freeze": freeze.tolist(),
            "colors": colors
        },
        default=None
    )