#!/usr/bin/env python3
"""Base-3 analogue of the v0.1.0 Santa 2026 binary-powers baseline.

Keep powers of 3 in registers, sum ternary digits into register 47, and
reinitialize the accumulator for each target. Runs without public heuristics.

Usage:
  python3 santa_base3_experiment.py puzzles.csv output.csv
"""

import argparse
import csv
import json
from pathlib import Path

from santa_binary_baseline import valid_size


def solve(starting_values: list[int], targets: list[int], value_limit: int,
          registers: int = 48) -> list[list]:
    if registers != 48:
        raise ValueError("Expected exactly 48 registers")
    if not starting_values:
        raise ValueError("At least one starting value required")
    if any(t < 1 or t > value_limit for t in targets):
        raise ValueError("Invalid target")

    reg = [None] * registers
    reg[:len(starting_values)] = starting_values
    remaining = set(targets) - set(starting_values)
    if not remaining:
        return []
    program = []

    def emit(op, a, b, dest):
        x, y = reg[a], reg[b]
        assert x is not None and y is not None
        if op == '+':
            v = x + y
        elif op == '*':
            v = x * y
        elif op == '/':
            assert y != 0 and x % y == 0
            v = x // y
        else:
            raise ValueError(op)
        assert 1 <= v <= value_limit, (op, x, y, value_limit)
        reg[dest] = v
        program.append([op, a, b, dest])
        remaining.discard(v)

    # Power layout: r0=1, r1=2; r2=3^1, r3=3^2, ..., r(k+1)=3^k.
    # r47 is exclusively the target accumulator; no recycling needed for
    # the supplied datasets' magnitudes.
    emit('/', 0, 0, 0)
    if not remaining:
        return program

    maximum = max(remaining)
    if maximum >= 2:
        emit('+', 0, 0, 1)
    if maximum >= 3:
        emit('+', 1, 0, 2)
        power = 3
        power_exp = 1
        while power * 3 <= maximum:
            next_reg = power_exp + 2
            if next_reg >= 47:
                raise ValueError("Ternary powers exceeded allotted registers")
            emit('*', power_exp + 1, 2, next_reg)
            power *= 3
            power_exp += 1

    def power_register(exp):
        return 0 if exp == 0 else exp + 1

    for target in sorted(set(targets)):
        if target not in remaining:
            continue
        n = target
        acc_initialized = False
        exponent = 0
        while n:
            n, digit = divmod(n, 3)
            if digit:
                p = power_register(exponent)
                if not acc_initialized:
                    # Multiplication by 1 copies a power; multiplying by 2
                    # handles a ternary digit '2' in one instruction.
                    emit('*', p, 0 if digit == 1 else 1, 47)
                    acc_initialized = True
                else:
                    for _ in range(digit):
                        emit('+', 47, p, 47)
            exponent += 1
        assert target not in remaining, target

    assert not remaining, sorted(remaining)[:8]
    return program


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('puzzles', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    total = 0
    count = 0
    maximum = 0
    with args.puzzles.open(newline='') as fi, args.output.open('w', newline='') as fo:
        reader = csv.DictReader(fi)
        writer = csv.writer(fo)
        writer.writerow(['id', 'program'])
        for row in reader:
            program = solve(json.loads(row['starting_values']),
                            json.loads(row['targets']), int(row['value_limit']),
                            int(row['registers']))
            if not valid_size(program):
                raise ValueError(f"Program exceeds instruction/size limit: {row['id']}")
            writer.writerow([row['id'], json.dumps(program, separators=(',', ':'))])
            count += 1
            total += len(program)
            maximum = max(maximum, len(program))
    print(f'{count} puzzles; {total:,} instructions; largest program {maximum:,}')


if __name__ == '__main__':
    main()
