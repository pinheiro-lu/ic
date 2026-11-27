# Santa 2026 — solver experiments

The initial implementation is **v0.1.0**, the user's binary-powers construction with register recycling. This repository records each code version and its measured score before any algorithmic changes.

## Run the existing baseline

Copy the official `puzzles.csv` and `practice_puzzles.csv` to `data/`, then run:

```sh
python3 src/santa_binary_baseline.py data/puzzles.csv results/binary_submission.csv
```

Optional comparison with an official sample submission:

```sh
python3 src/santa_binary_baseline.py data/puzzles.csv results/best_of_binary_and_sample.csv --sample data/sample_submission.csv
```

The `--sample` option **changes the selected programs**; score the raw baseline without this flag when comparing algorithms.

Run tests:

```sh
PYTHONPATH=src python3 -m unittest discover -s tests -v
```

## Versioning rule

For every **meaningful code change**:

1. Commit it on a new branch or with a clearly descriptive Git commit message.
2. Run the same test suite and dataset benchmark.
3. Append the commit hash and result to `results/experiments.csv` (do not overwrite older results).
4. Commit the experiment record and tag milestone versions when useful.

Git preserves prior code even when a newer experiment performs worse. Stable improvements can be merged into the main branch. Do not commit the competition data or generated submissions to Git unless you explicitly decide to do so.

## Initial benchmark

| Version | Dataset | Puzzles | Total instructions | Max instructions/puzzle |
|---|---|---:|---:|---:|
| v0.1.0 | scored | 1,000 | 902,521 | 2,873 |
| v0.1.0 | practice | 200 | 197,840 | 2,838 |

The baseline is not the best-known public approach. These scores are for **your own binary construction**, without an imported public solver.
