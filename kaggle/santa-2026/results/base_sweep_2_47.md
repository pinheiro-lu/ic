# Bases 2–47 experiment — 2026-11-27

All 46 bases were evaluated on the official 1,000 scored and 200 practice puzzles. Each output program was simulated independently to verify exact arithmetic, 48-register indices, target coverage, intermediate value limits, <=4096 operations, and <=131072 serialized bytes.

- **Best fixed base:** 36 — 406,983 instructions scored; 82,362 practice.
- **Per-puzzle portfolio (best of bases 2–47):** 397,713 instructions scored; 80,553 practice.
- **Original base-2 version:** 902,521 scored; 197,840 practice.
- **Base 47:** 416,573 scored; 84,520 practice.

The portfolio was separately revalidated from the generated submission CSV files (1,000/1,000 and 200/200 valid).

The solver caches as many coefficients as the registers permit, keeping r46 for temporary values and r47 for accumulation. When a digit d exceeds the cached coefficient count C, it constructs d as C + (d-C) in one instruction. Base 2 still uses the original register-recycling implementation. This is a positional-construction benchmark, *not* heuristic search.

See [all base scores](base_sweep_2_47.csv). Lower is better. Reported scores are local, not an official Kaggle leaderboard score.

## Reproduce

```bash
cd kaggle/santa-2026
python3 src/santa2026_bases_2_47.py --data data/puzzles.csv data/practice_puzzles.csv --output-dir results/
```

The command produces `portfolio_scored_2_47.csv`, `portfolio_practice_2_47.csv`, and a detailed `base_scores_2_47.csv`. Submissions are not committed to Git.

Input dataset SHA-256:

```
b0c70d849eca078a0e1cf9a4f7b47fd40b7435663f6f84de6725f84724500428  data/puzzles.csv
a87e4e0ad263f31af1a9e51459117212b03404339c5940be3aa6afd1596ff6d9  data/practice_puzzles.csv
```
