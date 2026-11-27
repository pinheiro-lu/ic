#!/usr/bin/env python3
"""Santa 2026 binary-powers baseline with register recycling.

Usage:
  python3 santa_binary_baseline.py data/puzzles.csv submission.csv \
      --sample data/sample_submission.csv

The optional sample is used as a fallback when the binary program exceeds a
program limit (and, by default, when the official sample is shorter).
"""

import argparse
import csv
import json
import sys
from pathlib import Path

REGISTERS = 48
ACC = 47
MAX_STEPS = 4096
MAX_BYTES = 131072


def solve(starting_values: list[int], targets: list[int], value_limit: int,
          registers: int = REGISTERS) -> list[list[int | str]]:
    """Use r0=1, r1..r46 for powers of two, and r47 to accumulate targets.

    Powers 2^47 and above are written successively into r1, r2, ... after
    lower bits have been accumulated. If another large target follows,
    regenerate the powers from r0=1.
    """
    if registers != REGISTERS:
        raise ValueError("This baseline assumes exactly 48 registers")
    if not starting_values:
        raise ValueError("At least one starting value is required")

    current: list[int | None] = [None] * REGISTERS
    current[:len(starting_values)] = starting_values
    remaining = set(targets) - set(starting_values)
    if not remaining:
        return []

    instructions: list[list[int | str]] = []

    def emit(op: str, a: int, b: int, d: int) -> None:
        x, y = current[a], current[b]
        if x is None or y is None:
            raise AssertionError(f"Undefined read: {op} {a} {b} -> {d}")
        if op == "+":
            z = x + y
        elif op == "*":
            z = x * y
        elif op == "/":
            if y == 0 or x % y:
                raise AssertionError("Invalid division")
            z = x // y
        else:
            raise AssertionError("Unexpected operation")
        if not 1 <= z <= value_limit:
            raise AssertionError(f"Out-of-range intermediate: {z}")
        current[d] = z
        instructions.append([op, a, b, d])
        remaining.discard(z)

    # The first starting value is positive, so r0/r0 produces 1.
    emit("/", 0, 0, 0)
    max_bit = max(t.bit_length() - 1 for t in remaining) if remaining else 0
    precomputed_through = min(46, max_bit)

    def rebuild() -> None:
        # r0 remains 1 throughout. Doubling fills the low powers.
        for i in range(1, precomputed_through + 1):
            emit("+", i - 1, i - 1, i)

    rebuild()
    powers_overwritten = False

    for target in sorted(set(targets)):
        if target not in remaining:
            continue

        high_bit = target.bit_length() - 1
        if powers_overwritten:
            rebuild()
            powers_overwritten = False
            if target not in remaining:
                continue

        # A power of two is covered by the power-building instructions alone.
        power_of_two = target & (target - 1) == 0
        accumulator_initialized = False

        # Sum the low bits before recycling registers that hold those bits.
        if not power_of_two:
            for i in range(min(high_bit, 46) + 1):
                if (target >> i) & 1:
                    if not accumulator_initialized:
                        emit("*", i, 0, ACC)  # Copy power via multiplication by 1.
                        accumulator_initialized = True
                    else:
                        emit("+", ACC, i, ACC)

        # r46 holds 2^46. r1 gets 2^47, r2 gets 2^48, etc.
        parent = 46
        for bit in range(47, high_bit + 1):
            destination = bit - 46
            emit("+", parent, parent, destination)
            parent = destination
            powers_overwritten = True
            if not power_of_two and ((target >> bit) & 1):
                if not accumulator_initialized:
                    emit("*", destination, 0, ACC)
                    accumulator_initialized = True
                else:
                    emit("+", ACC, destination, ACC)

        if target in remaining:
            raise AssertionError(f"Failed to generate target {target}")

    if remaining:
        raise AssertionError(f"Uncovered targets: {sorted(remaining)[:8]}")
    return instructions


def program_bytes(program: list) -> int:
    return len(json.dumps(program, separators=(",", ":")).encode("utf-8"))


def valid_size(program: list) -> bool:
    return len(program) <= MAX_STEPS and program_bytes(program) <= MAX_BYTES


def read_sample(path: Path) -> dict[str, list]:
    with path.open(newline="") as f:
        rows = csv.DictReader(f)
        if rows.fieldnames != ["id", "program"]:
            raise ValueError("Sample must have columns id,program")
        return {r["id"]: json.loads(r["program"]) for r in rows}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("puzzles", type=Path, help="Official puzzles.csv")
    parser.add_argument("output", type=Path, help="Output submission.csv")
    parser.add_argument("--sample", type=Path,
                        help="Official sample_submission.csv, used as a safer/shorter fallback")
    args = parser.parse_args()

    sample = read_sample(args.sample) if args.sample else None
    count = 0
    binary_wins = 0
    baseline_total = 0
    output_total = 0
    too_long = 0

    with args.puzzles.open(newline="") as inp, args.output.open("w", newline="") as out:
        reader = csv.DictReader(inp)
        writer = csv.writer(out)
        writer.writerow(["id", "program"])
        for row in reader:
            name = row["id"]
            starting = json.loads(row["starting_values"])
            targets = json.loads(row["targets"])
            program = solve(starting, targets, int(row["value_limit"]),
                            int(row["registers"]))
            baseline_total += len(program)
            candidate = program
            if not valid_size(program):
                too_long += 1
                if sample is None:
                    raise ValueError(
                        f"{name}: binary program exceeds Kaggle limits "
                        f"({len(program)} instructions). Supply --sample to allow fallback."
                    )
                candidate = sample[name]
            elif sample is not None and len(sample[name]) <= len(program):
                candidate = sample[name]
            else:
                binary_wins += 1
            if not valid_size(candidate):
                raise ValueError(f"{name}: selected program exceeds Kaggle limits")
            writer.writerow([name, json.dumps(candidate, separators=(",", ":"))])
            output_total += len(candidate)
            count += 1

    print(f"Puzzles: {count}", file=sys.stderr)
    print(f"Raw binary baseline: {baseline_total} instructions", file=sys.stderr)
    print(f"Selected submission: {output_total} instructions", file=sys.stderr)
    print(f"Binary programs selected: {binary_wins}", file=sys.stderr)
    print(f"Binary programs above limits: {too_long}", file=sys.stderr)
    print(f"Output: {args.output}", file=sys.stderr)


if __name__ == "__main__":
    main()
