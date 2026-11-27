import random
import unittest
from santa_binary_baseline import solve


def replay(starts, targets, limit, program):
    r = [None] * 48
    r[:len(starts)] = starts
    covered = set(starts)
    for op, a, b, d in program:
        assert 0 <= a < 48 and 0 <= b < 48 and 0 <= d < 48
        assert r[a] is not None and r[b] is not None
        x, y = r[a], r[b]
        if op == "/":
            assert y and x % y == 0
            value = x // y
        elif op == "+":
            value = x + y
        elif op == "*":
            value = x * y
        else:
            raise AssertionError(op)
        assert 1 <= value <= limit
        r[d] = value
        covered.add(value)
    assert set(targets) <= covered, (set(targets) - covered)


class TestBinaryBaseline(unittest.TestCase):
    def test_example(self):
        s, t, lim = [3], [1, 2, 5, 8], 9
        p = solve(s, t, lim)
        replay(s, t, lim, p)

    def test_power_recycling(self):
        s = [3]
        t = [3, 5, 100, (1 << 48) + 5, (1 << 49) + 7]
        p = solve(s, t, 1 << 55)
        replay(s, t, 1 << 55, p)
        self.assertTrue(any(a == 46 and b == 46 and d == 1
                            for op, a, b, d in p))

    def test_high_power_only(self):
        for bit in (0, 1, 46, 47, 59):
            p = solve([7], [1 << bit], 10**18)
            replay([7], [1 << bit], 10**18, p)

    def test_random(self):
        rng = random.Random(2026)
        for _ in range(300):
            starts = [rng.randint(1, 1000) for _ in range(rng.randint(1, 5))]
            limit = rng.choice([1000, 1000000, 10**15, 10**18])
            targets = [rng.randint(1, limit) for _ in range(rng.randint(1, 12))]
            p = solve(starts, targets, limit)
            replay(starts, targets, limit, p)


if __name__ == "__main__":
    unittest.main()
