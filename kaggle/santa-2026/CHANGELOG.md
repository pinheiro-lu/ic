# Changelog — Santa 2026

## v0.2.0 — 2026-11-27

- Added validated positional-base solvers for bases 2–74.
- Selects the shortest generated program independently for every puzzle.
- Scored dataset: **397,208** instructions across 1,000 puzzles (max 745 per puzzle).
- Practice dataset: **80,511** instructions across 200 puzzles (max 742 per puzzle).
- Best fixed base was 36 (406,983 scored instructions).
- Retained the original v0.1.0 baseline, all experiment branches, and recorded hashes.
- This release records only locally validated results; no official Kaggle score is claimed.

## v0.1.0 — 2026-11-27

- Initial binary powers with register recycling.
- Scored dataset: 902,521 instructions.
- Practice dataset: 197,840 instructions.
- Independent interpreter and unit tests.
