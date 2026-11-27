# Positional-base experiment — November 27, 2026

Exploratory measurements for `src/santa_base_experiment.py`, independently
checked by `tests/validate_positional_bases.py`.

Only the positional base and digit accumulation strategy change. All puzzles
use 48 registers with r47 as accumulator, r46 as scratch for digit products,
and registers below r46 for coefficients and powers.

## Scored set: 1,000 puzzles

| Base | Repeated addition | Multiply by digit | Largest program (multiply) |
| ---: | ---: | ---: | ---: |
| 3 | 657,147 | 657,147 | 1,252 |
| 4 | 757,973 | 642,991 | 1,255 |
| 8 | 1,115,714 | 554,454 | 1,075 |
| 9 | 1,197,329 | 540,114 | 1,027 |
| 16 | 1,716,087 | 470,088 | 896 |
| 32 | Some puzzles exceed 4,096 | 409,962 | 760 |
| 36 | Some puzzles exceed 4,096 | 406,983 | 760 |

## Practice set: 200 puzzles

| Base | Multiply by digit |
| ---: | ---: |
| 3 | 133,729 |
| 4 | 130,300 |
| 8 | 112,343 |
| 9 | 109,848 |
| 16 | 95,261 |
| 32 | 82,884 |
| 36 | 82,362 |

Dataset checksums (SHA-256):

```
b0c70d849eca078a0e1cf9a4f7b47fd40b7435663f6f84de6725f84724500428  data/puzzles.csv
a87e4e0ad263f31af1a9e51459117212b03404339c5940be3aa6afd1596ff6d9  data/practice_puzzles.csv
```

## Reproduce

```bash
python3 src/santa_base_experiment.py data/puzzles.csv data/practice_puzzles.csv --bases 3 4 8 9 16 32 36
PYTHONPATH=src python3 tests/validate_positional_bases.py data/puzzles.csv
PYTHONPATH=src python3 tests/validate_positional_bases.py data/practice_puzzles.csv
```

The validation is an independent register-by-register simulation checking exact
division, positive bounded intermediates, target coverage, and program-size
limits. These are local results, not an official Kaggle score.

No changes were made to the original binary baseline or to master.
