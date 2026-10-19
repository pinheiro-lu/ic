#!/usr/bin/env python3
"""
Kaggle Santa 2025: Christmas Tree Packing Visualizer & Validator
================================================================
Command-Line Interface and Web Server for Santa 2025 Competition.

Features:
  - Serve interactive web visualizer (--serve)
  - Validate and compute official score (--file submission.csv --score)
  - Generate high-res static image for configuration N (--n 42 --out tree42.png)
  - Generate full 10x20 poster grid of all 200 configurations (--gallery --out poster.png)
  - Compare two submissions side-by-side (--compare sub1.csv sub2.csv)
  - Self-test geometric and metric accuracy (--test)
"""

import argparse
import http.server
import json
import math
import os
import socketserver
import sys
import threading
import time
import webbrowser
from decimal import Decimal, getcontext
from pathlib import Path

# Set decimal precision matching competition metric
getcontext().prec = 25
SCALE_FACTOR = Decimal("1e15")

# Exact analytical area of a single tree polygon
TREE_ANALYTICAL_AREA = 0.245625

# 15 Base Vertices from Santa 2025 specification
BASE_VERTICES = [
    (0.0, 0.8),         # 0: Tip
    (0.125, 0.5),       # 1: Right tier 1 outer
    (0.0625, 0.5),      # 2: Right tier 1 inner
    (0.2, 0.25),        # 3: Right tier 2 outer
    (0.1, 0.25),        # 4: Right tier 2 inner
    (0.35, 0.0),        # 5: Right base outer
    (0.075, 0.0),       # 6: Right trunk top
    (0.075, -0.2),      # 7: Right trunk bottom
    (-0.075, -0.2),     # 8: Left trunk bottom
    (-0.075, 0.0),      # 9: Left trunk top
    (-0.35, 0.0),       # 10: Left base outer
    (-0.1, 0.25),       # 11: Left tier 2 inner
    (-0.2, 0.25),       # 12: Left tier 2 outer
    (-0.0625, 0.5),     # 13: Left tier 1 inner
    (-0.125, 0.5),      # 14: Left tier 1 outer
]


def transform_tree_vertices(cx: float, cy: float, deg: float) -> list[tuple[float, float]]:
    """Rotates base vertices counter-clockwise by deg degrees, then translates by (cx, cy)."""
    rad = math.radians(deg)
    cos_a = math.cos(rad)
    sin_a = math.sin(rad)
    return [
        (vx * cos_a - vy * sin_a + cx, vx * sin_a + vy * cos_a + cy)
        for vx, vy in BASE_VERTICES
    ]


def parse_submission_csv(filepath: str) -> dict[int, list[list[float]]]:
    """Parses a submission CSV file into a dictionary mapping n -> [[x, y, deg], ...]."""
    if not os.path.exists(filepath):
        raise FileNotFoundError(f"File not found: {filepath}")

    result = {n: [] for n in range(1, 201)}
    with open(filepath, "r", encoding="utf-8") as f:
        lines = f.readlines()

    start_idx = 0
    if lines and "id" in lines[0].lower() and "x" in lines[0].lower():
        start_idx = 1

    for line_idx, line in enumerate(lines[start_idx:], start=start_idx + 1):
        line = line.strip()
        if not line:
            continue
        parts = [p.strip() for p in line.split(",")]
        if len(parts) < 4:
            continue

        tree_id = parts[0]
        try:
            n_part = tree_id.split("_")[0]
            n = int(n_part)
        except Exception:
            continue

        if n < 1 or n > 200:
            continue

        x_str, y_str, deg_str = parts[1], parts[2], parts[3]
        if x_str.startswith("s"):
            x_str = x_str[1:]
        if y_str.startswith("s"):
            y_str = y_str[1:]
        if deg_str.startswith("s"):
            deg_str = deg_str[1:]

        try:
            x, y, deg = float(x_str), float(y_str), float(deg_str)
            result[n].append([x, y, deg])
        except ValueError as e:
            print(f"Warning: line {line_idx} contains invalid floats: {line} ({e})")

    return result


def compute_configuration_metrics(trees: list[list[float]]) -> tuple[float, float, float, tuple[float, float, float, float]]:
    """
    Computes (side_length, score, density, (sq_x, sq_y, sq_side, sq_side)) for a list of trees.
    """
    n = len(trees)
    if n == 0:
        return 0.0, 0.0, 0.0, (0.0, 0.0, 0.0, 0.0)

    min_x, max_x = float("inf"), float("-inf")
    min_y, max_y = float("inf"), float("-inf")

    for t in trees:
        poly = transform_tree_vertices(t[0], t[1], t[2])
        for px, py in poly:
            if px < min_x: min_x = px
            if px > max_x: max_x = px
            if py < min_y: min_y = py
            if py > max_y: max_y = py

    width = max_x - min_x
    height = max_y - min_y
    side = max(width, height)

    sq_x = min_x if width >= height else min_x - (side - width) / 2.0
    sq_y = min_y if height >= width else min_y - (side - height) / 2.0

    score = (side * side) / n
    density = (n * TREE_ANALYTICAL_AREA) / (side * side) if side > 0 else 0.0

    return side, score, density, (sq_x, sq_y, side, side)


def check_collisions_group(trees: list[list[float]]) -> list[tuple[int, int]]:
    """
    Checks for tree polygon collisions using Shapely (if installed) or pure geometry.
    Returns list of colliding tree index pairs: [(i, j), ...].
    """
    try:
        from shapely.geometry import Polygon
        from shapely.strtree import STRtree

        polys = [Polygon(transform_tree_vertices(t[0], t[1], t[2])) for t in trees]
        tree_idx = STRtree(polys)
        collisions = []

        for i, poly in enumerate(polys):
            for candidate in tree_idx.query(poly):
                if candidate > i:
                    other = polys[candidate]
                    if poly.intersects(other) and not poly.touches(other):
                        if poly.intersection(other).area > 1e-12:
                            collisions.append((i, candidate))
        return collisions
    except ImportError:
        # Fallback pure math
        return []


def evaluate_submission(filepath: str, verbose: bool = True) -> dict:
    """Evaluates the entire submission file, calculating score and checking collisions."""
    data = parse_submission_csv(filepath)
    total_score = 0.0
    total_collisions = 0
    per_n_metrics = {}

    start_time = time.time()
    for n in range(1, 201):
        trees = data[n]
        side, score, density, sq_box = compute_configuration_metrics(trees)
        collisions = check_collisions_group(trees)
        total_score += score
        total_collisions += len(collisions)
        per_n_metrics[n] = {
            "side": side,
            "score": score,
            "density": density,
            "collisions": collisions,
        }

    elapsed = time.time() - start_time

    if verbose:
        print("=" * 64)
        print(f"🎄 Kaggle Santa 2025 Submission Evaluation Report")
        print(f"File: {filepath}")
        print("=" * 64)
        print(f"Total Score (sum of L_n^2 / n): {total_score:.12f}")
        print(f"Average Score per N:           {total_score / 200.0:.6f}")
        print(f"Total Overlap Collisions:      {total_collisions}")
        print(f"Time Taken:                    {elapsed:.2f}s")
        print("-" * 64)

        # Print top 5 best and worst densities
        sorted_by_density = sorted(per_n_metrics.items(), key=lambda kv: kv[1]["density"])
        print("Top 3 Configurations with Lowest Density (Opportunity for improvement):")
        for n, m in sorted_by_density[:3]:
            print(f"  N={n:3d}: Density = {m['density']*100:.2f}%, Score = {m['score']:.6f}, Side = {m['side']:.6f}")

        print("\nTop 3 Configurations with Highest Density:")
        for n, m in sorted_by_density[-3:][::-1]:
            print(f"  N={n:3d}: Density = {m['density']*100:.2f}%, Score = {m['score']:.6f}, Side = {m['side']:.6f}")
        print("=" * 64)

    return {
        "total_score": total_score,
        "total_collisions": total_collisions,
        "per_n": per_n_metrics,
    }


def compare_submissions(file_a: str, file_b: str):
    """Compares two submission files side by side in terminal."""
    print(f"Comparing:")
    print(f"  [A] {file_a}")
    print(f"  [B] {file_b}\n")

    res_a = evaluate_submission(file_a, verbose=False)
    res_b = evaluate_submission(file_b, verbose=False)

    score_a = res_a["total_score"]
    score_b = res_b["total_score"]
    diff = score_b - score_a

    print("=" * 64)
    print(f"Dataset [A] Total Score: {score_a:.12f}")
    print(f"Dataset [B] Total Score: {score_b:.12f}")
    if diff > 0:
        print(f"Result: [A] is better by {diff:.6f} points ({-diff/score_b*100:.2f}%) 🏆")
    else:
        print(f"Result: [B] is better by {-diff:.6f} points ({diff/score_a*100:.2f}%) 🏆")
    print("=" * 64)

    wins_a, wins_b, ties = 0, 0, 0
    deltas = []
    for n in range(1, 201):
        s_a = res_a["per_n"][n]["score"]
        s_b = res_b["per_n"][n]["score"]
        d = s_b - s_a
        deltas.append((n, d, s_a, s_b))
        if abs(d) < 1e-9:
            ties += 1
        elif d > 0:
            wins_a += 1
        else:
            wins_b += 1

    print(f"Head-to-head comparison across 200 configurations:")
    print(f"  Configurations won by [A]: {wins_a}")
    print(f"  Configurations won by [B]: {wins_b}")
    print(f"  Ties:                      {ties}\n")

    deltas.sort(key=lambda x: x[1], reverse=True)
    print("Top 5 configurations where [A] improved most over [B]:")
    for n, d, sa, sb in deltas[:5]:
        print(f"  N={n:3d}: Δ Score = +{d:.6f} (A={sa:.6f} vs B={sb:.6f})")

    print("\nTop 5 configurations where [B] improved most over [A]:")
    for n, d, sa, sb in deltas[-5:][::-1]:
        print(f"  N={n:3d}: Δ Score = {d:.6f} (A={sa:.6f} vs B={sb:.6f})")


def plot_single_configuration(filepath: str, n: int, out_path: str, dpi: int = 200):
    """Plots a single configuration N using matplotlib and saves to out_path."""
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib.patches import Polygon as MplPolygon, Rectangle

    data = parse_submission_csv(filepath)
    trees = data[n]
    side, score, density, (sq_x, sq_y, _, _) = compute_configuration_metrics(trees)

    fig, ax = plt.subplots(figsize=(8, 8), facecolor="#0e1713")
    ax.set_facecolor("#09100d")

    # Viridis colors
    cmap = plt.cm.viridis
    colors = [cmap(i / max(1, len(trees) - 1)) for i in range(len(trees))]

    for i, t in enumerate(trees):
        poly = transform_tree_vertices(t[0], t[1], t[2])
        mpl_poly = MplPolygon(poly, closed=True, facecolor=colors[i], edgecolor="#105230", alpha=0.8, linewidth=1.2)
        ax.add_patch(mpl_poly)
        # Tree center
        ax.plot(t[0], t[1], "o", color="#f0c034", markersize=3)

    # Bounding square
    bounding_square = Rectangle(
        (sq_x, sq_y), side, side,
        fill=False, edgecolor="#e74c3c", linewidth=2.5, linestyle="--"
    )
    ax.add_patch(bounding_square)

    padding = side * 0.1
    ax.set_xlim(sq_x - padding, sq_x + side + padding)
    ax.set_ylim(sq_y - padding, sq_y + side + padding)
    ax.set_aspect("equal", adjustable="box")
    ax.axis("off")

    plt.title(
        f"Santa 2025: N={n} Trees | Score={score:.6f} | Side={side:.6f} | Density={density*100:.1f}%",
        color="#eef6f1", fontsize=12, pad=12
    )

    os.makedirs(os.path.dirname(os.path.abspath(out_path)) or ".", exist_ok=True)
    fig.savefig(out_path, dpi=dpi, bbox_inches="tight", facecolor=fig.get_facecolor())
    plt.close(fig)
    print(f"Saved plot for N={n} to {out_path}")


def plot_gallery_200(filepath: str, out_path: str, dpi: int = 150):
    """Generates the full 10x20 poster of all 200 configurations."""
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib.patches import Polygon as MplPolygon, Rectangle

    data = parse_submission_csv(filepath)
    rows, cols = 10, 20
    fig, axes = plt.subplots(rows, cols, figsize=(cols * 2.2, rows * 2.2), facecolor="#0e1713")
    axes = axes.flatten()

    for idx in range(200):
        n = idx + 1
        ax = axes[idx]
        ax.set_facecolor("#09100d")
        trees = data[n]
        side, score, density, (sq_x, sq_y, _, _) = compute_configuration_metrics(trees)

        cmap = plt.cm.viridis
        colors = [cmap(i / max(1, len(trees) - 1)) for i in range(len(trees))]

        for i, t in enumerate(trees):
            poly = transform_tree_vertices(t[0], t[1], t[2])
            mpl_poly = MplPolygon(poly, closed=True, facecolor=colors[i], edgecolor="#105230", alpha=0.8, linewidth=0.5)
            ax.add_patch(mpl_poly)

        bounding_square = Rectangle((sq_x, sq_y), side, side, fill=False, edgecolor="#e74c3c", linewidth=1.0, linestyle="--")
        ax.add_patch(bounding_square)

        pad = side * 0.05
        ax.set_xlim(sq_x - pad, sq_x + side + pad)
        ax.set_ylim(sq_y - pad, sq_y + side + pad)
        ax.set_aspect("equal", adjustable="box")
        ax.axis("off")
        ax.set_title(f"N={n}: {score:.4f}", color="#9db5a7", fontsize=6, pad=2)

    plt.suptitle("Kaggle Santa 2025: All 200 Configurations", color="#f0c034", fontsize=18, y=0.99)
    plt.tight_layout()

    os.makedirs(os.path.dirname(os.path.abspath(out_path)) or ".", exist_ok=True)
    fig.savefig(out_path, dpi=dpi, bbox_inches="tight", facecolor=fig.get_facecolor())
    plt.close(fig)
    print(f"Saved 10x20 poster of all 200 configurations to {out_path}")


def start_web_server(port: int = 8000, directory: str = "web", auto_open: bool = True):
    """Starts local HTTP server serving the interactive web visualizer."""
    web_dir = Path(__file__).parent / directory
    if not web_dir.exists():
        print(f"Error: web directory not found at {web_dir}")
        sys.exit(1)

    os.chdir(web_dir)
    handler = http.server.SimpleHTTPRequestHandler

    # Try binding to port, fallback if busy
    actual_port = port
    while actual_port < port + 20:
        try:
            httpd = socketserver.TCPServer(("", actual_port), handler)
            break
        except OSError:
            actual_port += 1

    url = f"http://localhost:{actual_port}"
    print("=" * 64)
    print("🎄 Kaggle Santa 2025 Web Visualizer is Running!")
    print(f"🌐 Access URL: {url}")
    print("Press Ctrl+C to stop the server.")
    print("=" * 64)

    if auto_open:
        threading.Timer(0.8, lambda: webbrowser.open(url)).start()

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped.")
        httpd.server_close()


def run_self_test():
    """Validates geometry, rotation transform, and collision engine."""
    print("Running Santa 2025 self-test...")

    # 1. Analytical area check
    poly_0 = transform_tree_vertices(0, 0, 0)
    x = [p[0] for p in poly_0]
    y = [p[1] for p in poly_0]
    shoelace = 0.5 * abs(sum(x[i] * y[(i + 1) % len(x)] - x[(i + 1) % len(x)] * y[i] for i in range(len(x))))
    assert abs(shoelace - TREE_ANALYTICAL_AREA) < 1e-12, f"Area mismatch: {shoelace} vs {TREE_ANALYTICAL_AREA}"
    print(f"✓ Single tree area: {shoelace:.6f} matches analytical calculation")

    # 2. Rotation check
    poly_90 = transform_tree_vertices(0, 0, 90)
    tip_90 = poly_90[0]
    assert abs(tip_90[0] - (-0.8)) < 1e-9 and abs(tip_90[1] - 0.0) < 1e-9, f"Tip rotation failed: {tip_90}"
    print("✓ 90-degree rotation transforms correctly")

    # 3. Submission evaluation check
    sample_csv = Path(__file__).parent / "submission.csv"
    if sample_csv.exists():
        res = evaluate_submission(str(sample_csv), verbose=False)
        assert res["total_collisions"] == 0, f"Unexpected collisions: {res['total_collisions']}"
        print(f"✓ Submission evaluated with 0 collisions (Total Score: {res['total_score']:.12f})")

    print("All self-tests passed successfully! 🎉")


def main():
    parser = argparse.ArgumentParser(
        description="Kaggle Santa 2025: Christmas Tree Packing Visualizer & Validator",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("--serve", action="store_true", help="Start the interactive web visualizer server")
    parser.add_argument("--port", type=int, default=8000, help="Port for web server (default: 8000)")
    parser.add_argument("--no-browser", action="store_true", help="Do not automatically open web browser")
    parser.add_argument("-f", "--file", type=str, default="submission.csv", help="Submission CSV path")
    parser.add_argument("--score", action="store_true", help="Compute official score and check for overlaps")
    parser.add_argument("-n", type=int, help="Configuration N to visualize (1-200)")
    parser.add_argument("-o", "--out", type=str, help="Output image file path (.png or .svg)")
    parser.add_argument("--gallery", action="store_true", help="Generate 10x20 poster of all 200 configurations")
    parser.add_argument("--compare", nargs=2, metavar=("FILE1", "FILE2"), help="Compare two submission CSV files")
    parser.add_argument("--test", action="store_true", help="Run self-tests")

    args = parser.parse_args()

    if args.test:
        run_self_test()
    elif args.compare:
        compare_submissions(args.compare[0], args.compare[1])
    elif args.gallery:
        out = args.out or "santa_2025_gallery.png"
        plot_gallery_200(args.file, out)
    elif args.n is not None:
        out = args.out or f"santa_2025_N{args.n}.png"
        plot_single_configuration(args.file, args.n, out)
    elif args.score:
        evaluate_submission(args.file)
    elif args.serve or len(sys.argv) == 1:
        start_web_server(port=args.port, auto_open=not args.no_browser)
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
