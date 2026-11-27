# Reverse target ordering experiment — 2026-11-27

The v0.2.0 positional-base solver processes target values in ascending order.
This experiment changes only the loop order to **descending target value**.
Every intermediate instruction already calls `remaining.discard(z)`, so targets
encountered while constructing a larger target do not need a second program.

All 73 bases (2–74) were tested on both official datasets. Base 2 retains its
original implementation. Every candidate was independently simulated and
checked against the 48-register limit, value_limit, exact integer operations,
target coverage and instruction/byte limits.

| Dataset | v0.2.0 portfolio | Descending portfolio | Puzzles improved | Savings |
|---|---:|---:|---:|---:|
| Scored (1,000) | 397,208 | **397,191** | 9 | 17 |
| Practice (200) | 80,511 | **80,507** | 2 | 4 |

The largest programs were 745 and 742 instructions, respectively.
Generated submission SHA-256s:

```text
89f6b6f3f7c9673635d7961a0f4ffa2377c08cde2b247c5c7ea013d15d6fb639  results/portfolio_scored_2_74.csv
5e30711a66509a03b04e827b84879794cda6556c7b9d68a3e7948c6332da37c1  results/portfolio_practice_2_74.csv
```

This only **marks intermediate target coverage**; it does not retain every
intermediate value in a register for future operations. The working accumulator
in r47 is overwritten for each target. Deliberate intermediate reuse would
require register allocation or alternative construction paths and is a separate
experiment.

To reproduce, run `python3 src/santa2026_bases_2_74.py --data data/puzzles.csv data/practice_puzzles.csv --output-dir results/` on this branch. The tagged/branched v0.2.0 release on master is not modified.
