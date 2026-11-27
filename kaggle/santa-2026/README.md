# Santa 2026 — solver experiments

Current milestone: **v0.2.0**, a positional-base portfolio that evaluates bases **2–74** and keeps the shortest valid program for each puzzle.

The previous **v0.1.0** binary-powers baseline remains in Git history as `src/santa_binary_baseline.py`. Experiments that led to v0.2.0 remain available in separate branches and commits.

## Run v0.2.0

Copy the official `puzzles.csv` and `practice_puzzles.csv` into `data/`:

```sh
python3 src/santa2026_bases_2_74.py \
    --data data/puzzles.csv data/practice_puzzles.csv \
    --output-dir results/
```

The script validates arithmetic instructions and puzzle coverage as it runs, and produces:

- `results/portfolio_scored_2_74.csv` — the scored submission.
- `results/portfolio_practice_2_74.csv` — practice programs.
- `results/base_scores_2_74.csv` — individual base scores.

For the initial baseline:

```sh
python3 src/santa_binary_baseline.py data/puzzles.csv results/binary_submission.csv
PYTHONPATH=src python3 -m unittest discover -s tests -v
```

Do not enable the binary baseline's `--sample` fallback when comparing its raw algorithmic score.

## Benchmark milestones

| Version | Strategy | Scored (1,000) | Practice (200) | Max scored program |
|---|---|---:|---:|---:|
| v0.1.0 | Binary powers, register recycling | 902,521 | 197,840 | 2,873 |
| v0.2.0 | Best of positional bases 2–74 per puzzle | **397,208** | **80,511** | 745 |

The best **fixed** base in v0.2.0 is 36: 406,983 scored instructions. All reported programs were independently simulated with the provided datasets and checked for register bounds, exact division, positive intermediate values, target coverage, and program length. Kaggle remains the official final validator.

The source datasets and generated CSV submissions are deliberately **not tracked** in Git. Their checksums and the release-submission checksums are documented under `results/`.

## Versioning and experiment records

- Use **SemVer-style** `0.MINOR.PATCH` milestones for solver changes; maintain experiment branches independently.
- Commit every meaningful change, including failed experiments.
- Record the source commit, dataset, score, and relevant parameters in `results/experiments.csv`.
- Keep old versions and results accessible: don't rewrite previous experimental histories.
- Refer to the `release/santa-2026-v0.2.0` branch for the frozen v0.2.0 release snapshot.
- The monorepo hosts other projects, so release identifiers are prefixed with `santa-2026`.

See `CHANGELOG.md`, `results/base_sweep_2_47.md`, and `results/base_sweep_48_74.md` for the development history.
