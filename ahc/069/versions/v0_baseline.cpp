#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <cmath>

using namespace std;

struct GroupInfo {
    int id;
    int s;
    int t;
    int p;
    long long v;
    double c = -1.0;
    vector<pair<int, int>> pos;
};

const int DIRS[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

vector<pair<int, int>> find_region(const vector<string>& grid, const vector<vector<int>>& owner, int n, int p) {
    int start_x = -1, start_y = -1;
    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
            if (grid[x][y] == '.' && owner[x][y] == -1) {
                start_x = x;
                start_y = y;
                break;
            }
        }
        if (start_x != -1) break;
    }

    if (start_x == -1) return {};

    vector<vector<bool>> visited(n, vector<bool>(n, false));
    visited[start_x][start_y] = true;
    queue<pair<int, int>> q;
    q.push({start_x, start_y});
    vector<pair<int, int>> region;

    while (!q.empty() && (int)region.size() < p) {
        auto [x, y] = q.front();
        q.pop();
        region.push_back({x, y});

        for (int d = 0; d < 4; ++d) {
            int nx = x + DIRS[d][0];
            int ny = y + DIRS[d][1];
            if (nx >= 0 && nx < n && ny >= 0 && ny < n && !visited[nx][ny] && grid[nx][ny] == '.' && owner[nx][ny] == -1) {
                visited[nx][ny] = true;
                q.push({nx, ny});
            }
        }
    }

    if ((int)region.size() == p) {
        return region;
    }
    return {};
}

double calculate_compactness(const vector<pair<int, int>>& region, int p, int n) {
    vector<vector<bool>> is_in_region(n, vector<bool>(n, false));
    for (const auto& cell : region) {
        is_in_region[cell.first][cell.second] = true;
    }

    int perimeter = 0;
    for (const auto& cell : region) {
        int x = cell.first;
        int y = cell.second;
        for (int d = 0; d < 4; ++d) {
            int nx = x + DIRS[d][0];
            int ny = y + DIRS[d][1];
            if (nx < 0 || nx >= n || ny < 0 || ny >= n || !is_in_region[nx][ny]) {
                perimeter++;
            }
        }
    }
    return 4.0 * sqrt(p) / perimeter;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int n, m;
    double r;
    if (!(cin >> n >> m >> r)) return 0;

    vector<string> grid(n);
    for (int i = 0; i < n; ++i) {
        cin >> grid[i];
    }

    vector<vector<int>> owner(n, vector<int>(n, -1));
    vector<GroupInfo> groups(m);

    for (int i = 0; i < m; ++i) {
        int gi, s, t, p;
        long long v;
        cin >> gi >> s >> t >> p >> v;

        groups[i].id = gi;
        groups[i].s = s;
        groups[i].t = t;
        groups[i].p = p;
        groups[i].v = v;

        for (int j = 0; j < i; ++j) {
            if (!groups[j].pos.empty() && groups[j].t < s) {
                for (const auto& cell : groups[j].pos) {
                    owner[cell.first][cell.second] = -1;
                }
                groups[j].pos.clear();
            }
        }

        cout << 0 << "\n";

        vector<pair<int, int>> region = find_region(grid, owner, n, p);
        if (!region.empty()) {
            for (const auto& cell : region) {
                owner[cell.first][cell.second] = i;
            }
            groups[i].pos = region;
            groups[i].c = calculate_compactness(region, p, n);

            cout << "Yes\n";
            for (const auto& cell : region) {
                cout << cell.first << " " << cell.second << "\n";
            }
        } else {
            cout << "No\n";
        }
        cout.flush();
    }

    return 0;
}
