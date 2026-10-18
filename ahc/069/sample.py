import math
import sys
from collections import deque
 
DIRS = [(-1, 0), (1, 0), (0, -1), (0, 1)]
 
 
class GroupInfo:
    def __init__(self, s, t, p, v):
        self.s = s        # arrival time
        self.t = t        # departure time
        self.p = p        # number of people
        self.v = v        # base payment
        self.c = None     # compactness (set after placement)
        self.pos = None   # coordinates of occupied cells [(row, col), ...] (set after placement)
 
 
def find_region(grid, owner, n, p):
    """
    Scan from the top row downward for the first empty lawn cell, then collect p cells from there using BFS.
    Return the list of coordinates if p cells can be collected, otherwise return None.
    """
 
    # Find the first empty lawn cell
    start = None
    for x in range(n):
        for y in range(n):
            if grid[x][y] == "." and owner[x][y] == -1:
                start = (x, y)
                break
        if start is not None:
            break
    if start is None:
        return None
 
    # BFS from start. Add each dequeued cell to the region, and stop when p cells are reached.
    visited = [[False] * n for _ in range(n)]
    visited[start[0]][start[1]] = True
    queue = deque([start])
    region = []
    while queue and len(region) < p:
        x, y = queue.popleft()
        region.append((x, y))
        for dx, dy in DIRS:
            nx, ny = x + dx, y + dy
            if 0 <= nx < n and 0 <= ny < n and not visited[nx][ny] \
                    and grid[nx][ny] == "." and owner[nx][ny] == -1:
                visited[nx][ny] = True
                queue.append((nx, ny))
 
    return region if len(region) == p else None
 
 
def compactness(region, p):
    """Return the compactness."""
    cells = set(region)
    perimeter = 0
    for (x, y) in region:
        for dx, dy in DIRS:
            if (x + dx, y + dy) not in cells:
                perimeter += 1
    return 4 * math.sqrt(p) / perimeter
 
 
def main():
    n, m, r = input().split()
    n, m = int(n), int(m)
    grid = [input() for _ in range(n)]
 
    # owner[x][y]: the group number occupying that cell. -1 if empty.
    owner = [[-1] * n for _ in range(n)]
    # all groups that have arrived so far
    groups = []
 
    for i in range(m):
        _gi, s, t, p, v = map(int, input().split())
        group = GroupInfo(s, t, p, v)
        groups.append(group)
 
        # Release groups whose departure time is before the current time s
        for g in groups:
            if g.pos is not None and g.t < s:
                for (x, y) in g.pos:
                    owner[x][y] = -1
                g.pos = None
 
        # Do not perform any move
        print(0)
 
        region = find_region(grid, owner, n, p)
        if region is not None:
            # Assign: update owner, and record the compactness and occupied cells
            for (x, y) in region:
                owner[x][y] = i
            group.pos = region
            group.c = compactness(region, p)
            print("Yes")
            for (x, y) in region:
                print(x, y)
        else:
            # Reject
            print("No")
        sys.stdout.flush()
 
 
if __name__ == "__main__":
    main()
