# Positional bases 48–74 — 2026-11-27

Continuation of the 2–47 positional-base experiment. The same register layout, coefficient construction, and target accumulation algorithm was used. The maximum accepted base was extended from 47 to 74; these bases can still construct the base and any missing digit coefficient from two stored coefficients in one addition.

All 27 bases were evaluated across 1,000 scored and 200 practice puzzles, and all 32,400 generated programs were independently simulated and checked against 48-register bounds, exact integer arithmetic, positive value limits, coverage, the 4,096-instruction limit, and the 131,072-byte program limit.

**Best single base remains 36:** 406,983 instructions on scored puzzles.

**Best new individual base is 48:** 416,182 instructions on scored puzzles (83,980 on practice). Base 74 scores 427,743.

**Per-puzzle portfolio score with bases 2–74:** **397,208**, versus **397,713** with 2–47. The extension improves 108 of 1,000 scored puzzles and saves 505 instructions. On practice, the portfolio improves from 80,553 to 80,511, saving 42 instructions on 14 puzzles.

These numbers are local validation results, not official leaderboard scores.

See [48–74 score breakdown](base_sweep_48_74.csv).

## Run

```bash
cd kaggle/santa-2026
python3 src/santa2026_bases_2_74.py --data data/puzzles.csv data/practice_puzzles.csv --output-dir results/
```

The 2–47 experiment and `master` remain unchanged. New positional bases above 74 would need a different method for constructing the base or the larger digits, so they are not included in this apples-to-apples extension.
