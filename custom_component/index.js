document.addEventListener("DOMContentLoaded", () => {
  const container = document.getElementById("sudoku-container");

  function createGrid(data) {
    container.innerHTML = "";
    const table = document.createElement("table");
    table.style.borderCollapse = "collapse";

    const sudoku = data.sudoku;
    const freeze = data.freeze;
    const colors = data.colors || Array(9).fill().map(() => Array(9).fill("#ffffff"));

    for (let i = 0; i < 9; i++) {
      const row = document.createElement("tr");
      for (let j = 0; j < 9; j++) {
        const cell = document.createElement("td");
        cell.style.border = "1px solid #000";
        if ((j + 1) % 3 === 0 && j < 8)
          cell.style.borderRight = "3px solid #000";
        if ((i + 1) % 3 === 0 && i < 8)
          cell.style.borderBottom = "3px solid #000";

        const input = document.createElement("input");
        input.type = "text";
        input.value = sudoku[i][j] || "";
        input.setAttribute("data-row", i);
        input.setAttribute("data-col", j);
        input.maxLength = 1;
        input.pattern = "[1-9]";
        input.style.width = "30px";
        input.style.height = "30px";
        input.style.backgroundColor = colors[i][j] || "#ffffff";
        input.style.textAlign = "center";
        input.style.fontSize = "18px";
        input.style.border = "none";
        input.style.outline = "none";

        // Disable input if the corresponding freeze value is true
        if (freeze[i][j]) {
          input.disabled = true;
          input.style.color = "#666";
        }

        // Send back the updated value
        input.addEventListener("input", (e) => {
          if (!freeze[i][j]) {
            const value = e.target.value.replace(/[^1-9]/g, "");
            e.target.value = value;
            const row = parseInt(e.target.getAttribute("data-row"));
            const col = parseInt(e.target.getAttribute("data-col"));

            Streamlit.setComponentValue({
              row,
              col,
              value: value ? parseInt(value) : 0,
//              freeze: freeze[row][col],
//              color: colors[row][col]
            });
          }
        });

        cell.appendChild(input);
        row.appendChild(cell);
      }
      table.appendChild(row);
    }
    container.appendChild(table);
  }

  Streamlit.events.addEventListener(Streamlit.RENDER_EVENT, (event) => {
    createGrid(event.detail.args.spec);
    Streamlit.setFrameHeight(container.scrollHeight);
  });

  Streamlit.setComponentReady();
});