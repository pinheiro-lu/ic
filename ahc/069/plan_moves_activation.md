# Implementation Plan: Activating 1-Group & 2-Group Move Shifts for Maximum Score

## Goal Description
An empirical audit across 10,000 queries (10 test seeds) revealed that **0 move shifts (0%)** were ever executed. This occurred because `find_best_region` scans 350 Chebyshev candidate seeds across the grid and always finds a non-empty placement (`!region.empty()`), causing the algorithm to output `0` moves unconditionally without ever evaluating move shifts.

This plan outlines how to activate 1-group and 2-group move shifts so that whenever relocating existing groups yields a **net global score increase**, the algorithm executes 1-group or 2-group moves instead of accepting low-compactness normal placements.

---

## Empirical Move Audit Results

```
=== MOVE TYPE AUDIT ACROSS ALL 10 SEEDS ===
| Seed | 0 Moves (Normal Fit) | 1 Move Shifts | 2 Move Shifts | Total Queries |
|:---:|:---:|:---:|:---:|:---:|
| 0000 | 1000                 | 0             | 0             | 1000 |
| 0001 | 1000                 | 0             | 0             | 1000 |
| 0002 | 1000                 | 0             | 0             | 1000 |
| 0003 | 1000                 | 0             | 0             | 1000 |
| 0004 | 1000                 | 0             | 0             | 1000 |
| 0005 | 1000                 | 0             | 0             | 1000 |
| 0006 | 1000                 | 0             | 0             | 1000 |
| 0007 | 1000                 | 0             | 0             | 1000 |
| 0008 | 1000                 | 0             | 0             | 1000 |
| 0009 | 1000                 | 0             | 0             | 1000 |
```

---

## User Review Required

> [!IMPORTANT]
> **Contest Rule Alignment**: Moving existing groups costs $v_G \cdot r$ points per move. A move shift should only be executed if the **Net Global Score Gain** exceeds the move cost:
> $$\Delta \text{Global Score} = \text{Score}_{\text{new}}(X) + \sum \text{Score}_{\text{new}}(G) - \sum \text{Score}_{\text{old}}(G) - \sum \text{Cost}(G) > \text{Score}_{\text{normal}}(X) + \text{MIN\_GAIN}$$

---

## Proposed Changes

### [`sample.cpp`](file:///home/luan/documentosUSP/progCompetitiva/atCoder/heuristic/069/sample.cpp)

#### [MODIFY] `main()` Placement Decision Flow

```diff
-        auto [chosen_c, region] = find_best_region(n, p, s, t);
-        if (!region.empty()) {
-            cout << 0 << "\n";
-            // output normal placement ...
-        } else {
-            // move shift search ...
-        }

+        auto [chosen_c, region] = find_best_region(n, p, s, t);
+        double normal_pts = (!region.empty()) ? round((double)v * chosen_c) : -1.0e18;
+
+        // Always evaluate 1-group & 2-group move shifts if normal placement is empty OR has room for improvement
+        double max_gain = normal_pts + MIN_GAIN;
+        int best_g_idx = -1; // -1: normal placement (0 moves), -2: 2-group move, >=0: 1-group move
+
+        // Run 1-Group & 2-Group Move Shift Search ...
```

---

## Verification Plan

### Automated Verification
Run the 10-seed benchmark script and audit the exact count of `0`, `1`, and `2` move commands executed:

```bash
python3 -c "
import subprocess, time

subprocess.run('g++ -O3 -std=c++17 -DMIN_GAIN=100000.0 sample.cpp -o sample', shell=True, check=True)

for seed in range(10):
    infile = f'tools/in/{seed:04d}.txt'
    res = subprocess.run(f'./tools/target/release/tester ./sample < {infile}', shell=True, capture_output=True, text=True)
    out = res.stderr + '\n' + res.stdout
    score = int(out.split('Score = ')[1].split()[0]) if 'Score = ' in out else 0
    lines = [l.strip() for l in res.stdout.splitlines() if l.strip() in ['0', '1', '2']]
    print(f'Seed {seed:04d}: 0-moves={lines.count(\"0\")}, 1-moves={lines.count(\"1\")}, 2-moves={lines.count(\"2\")}, Score={score:,}')
"
```

---
