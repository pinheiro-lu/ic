#!/usr/bin/env python3
"""Sweep positional bases 2..74 for the Santa 2026 Elf Calculator.

Uses the original fixed-register positional construction, with no search:
- r0=1, consecutive coefficient registers, and precomputed base powers;
- r46 is scratch, r47 the target accumulator;
- large coefficients d are formed as C+(d-C) in one operation;
- base 2 retains the original binary register-recycling implementation.

Run:
  python3 src/santa2026_bases_2_74.py --data data/puzzles.csv \
      data/practice_puzzles.csv --output-dir results/
"""
import argparse
import csv
import json
from collections import Counter
from pathlib import Path

from santa_binary_baseline import solve as original_binary


def solve(starts, targets, limit, base, registers=48):
    if not 2 <= base <= 74:
        raise ValueError("base outside [2, 74]")
    if registers != 48 or not starts:
        raise ValueError("Expected 48 registers and nonempty starts")
    if base == 2:
        return original_binary(starts, targets, limit, registers)
    r = [None] * registers
    r[:len(starts)] = starts
    remaining = set(targets) - set(starts)
    program = []
    if not remaining:
        return program

    def emit(op, a, b, dst):
        x, y = r[a], r[b]
        if x is None or y is None:
            raise ValueError("uninitialized read")
        if op == "+":
            z = x + y
        elif op == "*":
            z = x * y
        elif op == "/":
            if y == 0 or x % y:
                raise ValueError("non-exact division")
            z = x // y
        else:
            raise ValueError(op)
        if not 1 <= z <= limit:
            raise ValueError(f"value out of range: {z} (limit {limit})")
        r[dst] = z
        remaining.discard(z)
        program.append([op, a, b, dst])

    emit("/", 0, 0, 0)
    if not remaining:
        return program
    max_target = max(remaining)

    powers = 0
    value = 1
    while value <= max_target // base:
        value *= base
        powers += 1

    # Coefficients 1..C, powers b^1..b^P, scratch r46, accumulator r47.
    coefficient_count = min(base - 1, 46 - powers, max_target)
    if coefficient_count < 1:
        raise ValueError(f"Not enough registers for base {base}")
    if powers and base - coefficient_count > coefficient_count:
        raise ValueError(f"Cannot form base {base} from cached coefficients")

    for c in range(2, coefficient_count + 1):
        emit("+", c - 2, 0, c - 1)
    if not remaining:
        return program
    if powers:
        c = coefficient_count
        emit("+", c - 1, base - c - 1, c)
        for exp in range(2, powers + 1):
            emit("*", c + exp - 2, c, c + exp - 1)

    for target in sorted(set(targets)):
        if target not in remaining:
            continue
        x, exp, has_acc = target, 0, False
        while x:
            x, digit = divmod(x, base)
            if digit:
                if digit <= coefficient_count:
                    coeff = digit - 1
                else:
                    emit("+", coefficient_count - 1,
                         digit - coefficient_count - 1, 46)
                    coeff = 46
                if exp == 0:
                    if has_acc:
                        emit("+", 47, coeff, 47)
                    else:
                        emit("*", coeff, 0, 47)
                        has_acc = True
                else:
                    pow_reg = coefficient_count + exp - 1
                    if not has_acc:
                        emit("*", coeff, pow_reg, 47)
                        has_acc = True
                    elif digit == 1:
                        emit("+", 47, pow_reg, 47)
                    else:
                        emit("*", coeff, pow_reg, 46)
                        emit("+", 47, 46, 47)
            exp += 1
        if target in remaining:
            raise AssertionError(f"Target not generated: {target}")
    if remaining:
        raise AssertionError(f"Uncovered targets: {remaining}")
    return program


def verify_independently(starts, targets, limit, registers, program):
    state = starts + [None] * (registers - len(starts))
    covered = set(starts)
    for op, a, b, dst in program:
        assert 0 <= a < registers and 0 <= b < registers and 0 <= dst < registers
        v, w = state[a], state[b]
        assert v is not None and w is not None
        if op == "+":
            z = v + w
        elif op == "-":
            z = v - w
        elif op == "*":
            z = v * w
        elif op == "/":
            assert w != 0 and v % w == 0
            z = v // w
        else:
            raise AssertionError("Invalid operator")
        assert 1 <= z <= limit
        state[dst] = z
        covered.add(z)
    assert set(targets).issubset(covered)
    assert len(program) <= 4096
    assert len(json.dumps(program, separators=(",", ":")).encode()) <= 131072


def load_data(filename):
    data = []
    with filename.open(newline="") as f:
        for row in csv.DictReader(f):
            data.append((row["id"], json.loads(row["starting_values"]),
                         json.loads(row["targets"]), int(row["value_limit"]),
                         int(row["registers"])))
    return data


def evaluate(puzzles, bases, portfolio_path):
    totals, maxima, per_base = {}, {}, {}
    best_score = [float("inf")] * len(puzzles)
    best_base = [None] * len(puzzles)
    best_program = [None] * len(puzzles)
    for base in bases:
        scores = []
        worst = 0
        for idx, (_, starts, targets, limit, registers) in enumerate(puzzles):
            program = solve(starts, targets, limit, base, registers)
            verify_independently(starts, targets, limit, registers, program)
            score = len(program)
            scores.append(score)
            worst = max(worst, score)
            if score < best_score[idx]:
                best_score[idx] = score
                best_base[idx] = base
                best_program[idx] = program
        totals[base] = sum(scores)
        maxima[base] = worst
        per_base[base] = scores
        print(f"{base:2d}: {totals[base]:9,d} instructions", flush=True)
    with portfolio_path.open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["id", "program"])
        for (pid, _, _, _, _), program in zip(puzzles, best_program):
            writer.writerow([pid, json.dumps(program, separators=(",", ":"))])
    return totals, maxima, best_score, best_base


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--data", type=Path, nargs="+", required=True)
    p.add_argument("--output-dir", type=Path, default=Path("results"))
    args = p.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    rows = []
    for filename in args.data:
        dataset = "scored" if filename.name == "puzzles.csv" else "practice"
        puzzles = load_data(filename)
        totals, maxima, best, winning = evaluate(
            puzzles, range(2, 75),
            args.output_dir / f"portfolio_{dataset}_2_74.csv")
        wins = Counter(winning)
        print(f"Portfolio: {sum(best):,} instructions")
        print(f"Best fixed base: {min(totals, key=totals.get)}")
        for base in range(2, 75):
            rows.append([dataset, base, totals[base], maxima[base],
                         wins.get(base, 0), len(puzzles)])
    with (args.output_dir / "base_scores_2_74.csv").open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["dataset", "base", "total_instructions",
                         "max_per_puzzle", "puzzles_won_by_base", "valid_puzzles"])
        writer.writerows(rows)


if __name__ == "__main__":
    main()
