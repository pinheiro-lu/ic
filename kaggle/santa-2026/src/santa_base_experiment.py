#!/usr/bin/env python3
"""Compare positional bases for Santa 2026.

Modes:
  repeat:    mimic original base-3 approach by adding the selected power
             repeatedly for later nonzero digits.
  multiply:  precompute coefficients, then construct d*b^i with one
             multiplication and add the result to r47.

Preserves the constructive algorithm, with no heuristic search.
"""
import csv
import json
import argparse
from pathlib import Path


def solve(initial, targets, limit, base, mode="repeat"):
    assert 2 <= base <= 36
    r = [None] * 48
    r[:len(initial)] = initial
    covered = set(initial)
    remaining = set(targets) - covered
    inst = []
    if not remaining:
        return inst

    def emit(op, a, b, d):
        x, y = r[a], r[b]
        assert x is not None and y is not None
        if op == "+":
            v = x + y
        elif op == "*":
            v = x * y
        elif op == "/":
            assert y and x % y == 0
            v = x // y
        else:
            raise ValueError(op)
        assert 1 <= v <= limit, (op, x, y, v, limit)
        inst.append((op, a, b, d))
        r[d] = v
        covered.add(v)
        remaining.discard(v)

    emit("/", 0, 0, 0)
    if not remaining:
        return inst
    maximum = max(remaining)

    # r[0]=1, r[1]=2, ..., r[base-1]=base, when needed.
    for value in range(2, min(base, maximum) + 1):
        emit("+", value - 2, 0, value - 1)
    if not remaining:
        return inst
    assert maximum >= base

    # Powers: r[base-1]=base, r[base]=base^2, ...
    power = base
    exponent = 1
    while power * base <= maximum:
        destination = base - 1 + exponent
        assert destination < 46, (base, destination)
        emit("*", base - 1 + exponent - 1, base - 1, destination)
        power *= base
        exponent += 1

    for target in sorted(set(targets)):
        if target not in remaining:
            continue
        n, initialized, exponent = target, False, 0
        while n:
            n, digit = divmod(n, base)
            if digit:
                preg = 0 if exponent == 0 else base - 2 + exponent
                assert r[preg] == base ** exponent
                if not initialized:
                    emit("*", preg, digit - 1, 47)
                    initialized = True
                elif mode == "repeat":
                    for _ in range(digit):
                        emit("+", 47, preg, 47)
                elif mode == "multiply":
                    if digit == 1:
                        emit("+", 47, preg, 47)
                    else:
                        emit("*", preg, digit - 1, 46)
                        emit("+", 47, 46, 47)
                else:
                    raise ValueError(mode)
            exponent += 1
        assert target in covered, target

    assert not remaining, sorted(remaining)
    assert len(inst) <= 4096, len(inst)
    assert len(json.dumps(inst, separators=(",", ":")).encode()) <= 131072
    return inst


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("puzzles", nargs="+", type=Path)
    parser.add_argument("--bases", nargs="+", type=int,
                        default=[3, 4, 5, 8, 9, 16, 25, 32, 36])
    args = parser.parse_args()
    for file in args.puzzles:
        puzzles = list(csv.DictReader(file.open()))
        print(file.name, len(puzzles))
        for mode in ("repeat", "multiply"):
            for base in args.bases:
                scores = []
                for row in puzzles:
                    initial = json.loads(row["starting_values"])
                    targets = json.loads(row["targets"])
                    try:
                        instructions = solve(initial, targets,
                                             int(row["value_limit"]), base, mode)
                    except AssertionError as exc:
                        print("invalid for", base, mode, row["id"], exc)
                        break
                    scores.append(len(instructions))
                if len(scores) == len(puzzles):
                    print(f"base {base:2d} {mode:8s}: {sum(scores):>9,d} instructions; "
                          f"max {max(scores)}")


if __name__ == "__main__":
    main()
