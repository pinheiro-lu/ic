# 🎄 Kaggle Santa 2025: Christmas Tree Packing Visualizer & Studio

A high-performance interactive visualizer, sandbox editor, and CLI toolkit for the **[Kaggle Santa 2025 - Christmas Tree Packing Challenge](https://www.kaggle.com/competitions/santa-2025)**.

---

## 🌟 Highlights

- **Interactive 60fps Web Studio**: Smooth canvas navigation with zooming, panning, grid coordinates, and dynamic color palettes (Festive Emerald, Official Viridis, Rainbow Spectrum, Natural Pine, Collision Health).
- **Exact Geometry & Collision Detection**: Full 15-vertex tree polygon transformations matching Kaggle specifications, coupled with real-time SAT & ray-casting collision detection for interior polygon overlaps.
- **Direct Canvas Sandbox Editing**: Click and drag trees directly on the canvas, rotate with the circular gizmo or mouse wheel, nudge with arrow keys, and watch bounding square & score update live with undo/redo support.
- **Preloaded Dataset**: Comes preloaded with the **Getting Started Submission (`submission.csv`, ~180.18)**, plus drag-and-drop support for any custom submission CSV.
- **Head-to-Head Comparison Mode**: Compare any two submissions side-by-side or via translucent ghost overlay with per-configuration $\Delta \text{Score}$ and $\Delta \text{Side}$ breakdowns.
- **All 200 Configurations Analytics**: Interactive modal featuring score/side/density curves vs $N$, a 10×20 visual gallery, and sortable problem table.
- **Comprehensive Python CLI**: Evaluate submissions, check collisions, generate publication-ready Matplotlib plots, compare submissions, or launch the local web server.

---

## 🚀 Quickstart

### 1. Launch the Web Studio (1 Command)
```bash
./run.sh
# or
python3 visualizer.py --serve
```
This starts the local web server and opens [http://localhost:8000](http://localhost:8000) in your browser.

> **Zero-Server Standalone Option**: You can also directly double-click `web/index.html` in your file manager to open it in any modern browser without running a server!

---

## 💻 Python CLI Usage

The `visualizer.py` script provides command-line tools for scoring, static rendering, and benchmarking:

### 1. Evaluate Official Score & Overlaps
```bash
python3 visualizer.py --file submission.csv --score
```
Outputs total score $\sum (L_n^2 / n)$, average score per $N$, collision count, and highest/lowest density configurations.

### 2. Compare Two Submissions Head-to-Head
```bash
python3 visualizer.py --compare submission.csv other_submission.csv
```
Outputs win/loss/tie counts across all 200 configurations, total score delta, and top 5 largest improvements.

### 3. Generate Static Image for Configuration N
```bash
# Save high-resolution PNG for N=42
python3 visualizer.py --file submission.csv -n 42 --out tree_42.png
```

### 4. Generate 10×20 Poster of All 200 Configurations
```bash
python3 visualizer.py --file submission.csv --gallery --out santa_2025_all.png
```

### 5. Run Self-Test
```bash
python3 visualizer.py --test
```
Validates analytical area (`0.245625`), rotation transform, and collision detection.

---

## ⌨️ Keyboard Shortcuts (Web Studio)

| Key | Action |
| :--- | :--- |
| `←` / `→` | Previous / Next Configuration $(N \pm 1)$ |
| `[` / `]` | Jump by 10 Configurations $(N \pm 10)$ |
| `Space` or `F` | Auto-fit view to bounding square |
| `R` / `Shift + R` | Rotate selected tree by $+15^\circ$ / $-15^\circ$ |
| `Alt + Wheel` | Rotate selected tree smoothly under cursor |
| `Delete` / `Backspace` | Delete selected tree |
| `Ctrl + Z` | Undo last edit |
| `Ctrl + Y` | Redo last edit |
| `Esc` | Deselect current tree |

---

## 📐 Competition Mathematics & Geometry

### 1. The Tree Shape
A symmetrical 15-vertex polygon consisting of 3 tiered foliage layers and a rectangular trunk:
- **Trunk**: Width = `0.15`, Height = `0.2`, from Bottom at $y = -0.2$ to Base at $y = 0.0$
- **Bottom Tier**: Base Width = `0.7`, from $y = 0.0$ to $y = 0.25$
- **Middle Tier**: Base Width = `0.4`, from $y = 0.25$ to $y = 0.5$
- **Top Tier**: Base Width = `0.25`, from $y = 0.5$ to Tip at $y = 0.8$
- **Analytical Area**: Exactly $0.245625$

### 2. The Evaluation Metric

For each configuration $n \in [1, 200]$:

$$
L_n = \max(x_{\max} - x_{\min}, y_{\max} - y_{\min})
$$

$$
\text{Score}_n = \frac{L_n^2}{n}, \qquad \text{Total Score} = \sum_{n=1}^{200} \text{Score}_n = \sum_{n=1}^{200} \frac{L_n^2}{n}
$$

$$
\text{Packing Density}_n = \frac{n \times 0.245625}{L_n^2} = \frac{0.245625}{\text{Score}_n}
$$

### 3. Collision Constraint

Two trees overlap (invalid) if and only if their interiors intersect with non-zero area:

$$
\text{Intersects}(A, B) \land \neg \text{Touches}(A, B)
$$

Touching at boundary vertices or edges without penetrating the interior is strictly valid.

---

## 📁 Repository Structure

```
├── visualizer.py                      # Python CLI, validator, plotter, and web server
├── run.sh                             # 1-click launcher script
├── santa-2025-getting-started.ipynb   # Getting started notebook
├── submission.csv                     # Getting started submission (Score: 180.18)
├── README.md                          # Project documentation
└── web/
    ├── index.html                     # Interactive web visualizer UI
    ├── style.css                      # Holiday dark/light themes & glassmorphism
    ├── app.js                         # Canvas engine, geometry, SAT collisions & sandbox
    └── data/
        └── datasets.js                # Preloaded dataset for immediate offline viewing
```
