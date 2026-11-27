# Experiment records

Every candidate algorithm change should be committed before benchmarking. Append a row to `experiments.csv` containing the commit hash, dataset, score, and any noteworthy information. Never replace past rows.

Keep generated submissions in this directory if desired, but these large files are ignored by Git. Record their file hashes if exact byte-level reproduction matters.

The `scored` v0.1.0 baseline was independently simulated across all 1,000 puzzles and checked for target coverage, value range, instruction count, and serialized program-byte limit. The `practice` score was freshly reproduced in this version-control setup. Its puzzle programs have not yet been independently simulated here.
