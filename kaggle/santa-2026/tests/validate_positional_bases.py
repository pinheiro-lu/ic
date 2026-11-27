#!/usr/bin/env python3
"""Independently validate generated arithmetic programs against official puzzles."""
import argparse
import csv
import json
from pathlib import Path
from santa_base_experiment import solve


def verify(initial, targets, limit, registers, program):
    values = initial + [None] * (registers - len(initial))
    generated = set(initial)
    for op, a, b, dest in program:
        assert 0 <= a < registers and 0 <= b < registers and 0 <= dest < registers
        x, y = values[a], values[b]
        assert x is not None and y is not None
        if op == "+":
            value = x + y
        elif op == "-":
            value = x - y
        elif op == "*":
            value = x * y
        elif op == "/":
            assert y != 0 and x % y == 0
            value = x // y
        else:
            raise ValueError(op)
        assert 1 <= value <= limit
        values[dest] = value
        generated.add(value)
    assert set(targets) <= generated
    assert len(program) <= 4096
    assert len(json.dumps(program, separators=(",", ":")).encode()) <= 131072


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("puzzles", type=Path)
    ap.add_argument("--bases", type=int, nargs="+", default=[3, 4, 8, 9, 16, 32, 36])
    args = ap.parse_args()
    puzzles = list(csv.DictReader(args.puzzles.open()))
    for base in args.bases:
        total = 0
        for row in puzzles:
            initial = json.loads(row["starting_values"])
            targets = json.loads(row["targets"])
            limit = int(row["value_limit"])
            program = solve(initial, targets, limit, base, mode="multiply")
            verify(initial, targets, limit, int(row["registers"]), program)
            total += len(program)
        print(f"base {base}: {total:,} instructions; {len(puzzles)} valid puzzles")


if __name__ == "__main__":
    main()
