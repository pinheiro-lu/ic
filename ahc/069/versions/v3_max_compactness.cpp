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

struct Cell {
    int cheb_dist;
    int man_dist;
    int x;
    int y;

    bool operator>(const Cell& other) const {
        if (cheb_dist != other.cheb_dist) return cheb_dist > other.cheb_dist;
        if (man_dist != other.man_dist) return man_dist > other.man_dist;
        if (x != other.x) return x > other.x;
        return y > other.y;
    }
};

vector<pair<int, int>> find_region_from_seed(const vector<string>& grid, const vector<vector<int>>& owner, int n, int p, int sx, int sy) {
    vector<vector<bool>> visited(n, vector<bool>(n, false));
    visited[sx][sy] = true;

    priority_queue<Cell, vector<Cell>, greater<Cell>> pq;
    pq.push({0, 0, sx, sy});
    vector<pair<int, int>> region;

    while (!pq.empty() && (int)region.size() < p) {
        Cell curr = pq.top();
        pq.pop();
        region.push_back({curr.x, curr.y});

        for (int d = 0; d < 4; ++d) {
            int nx = curr.x + DIRS[d][0];
            int ny = curr.y + DIRS[d][1];
            if (nx >= 0 && nx < n && ny >= 0 && ny < n && !visited[nx][ny] && grid[nx][ny] == '.' && owner[nx][ny] == -1) {
                visited[nx][ny] = true;
                int cheb = max(abs(nx - sx), abs(ny - sy));
                int man = abs(nx - sx) + abs(ny - sy);
                pq.push({cheb, man, nx, ny});
            }
        }
    }

    if ((int)region.size() == p) {
        return region;
    }
    return {};
}

double calculate_compactness(const vector<pair<int, int>>& region, int p, int n) {
    // Fast lookup for cells in region
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

vector<pair<int, int>> find_best_region(const vector<string>& grid, const vector<vector<int>>& owner, int n, int p, double lambda, double& chosen_c) {
    double best_score = -1e18;
    vector<pair<int, int>> best_region;
    chosen_c = 0.0;

    for (int sx = 0; sx < n; ++sx) {
        for (int sy = 0; sy < n; ++sy) {
            if (grid[sx][sy] == '.' && owner[sx][sy] == -1) {
                vector<pair<int, int>> region = find_region_from_seed(grid, owner, n, p, sx, sy);
                if (!region.empty()) {
                    double c = calculate_compactness(region, p, n);
                    double score = -abs(c - lambda);

                    if (score > best_score) {
                        best_score = score;
                        best_region = region;
                        chosen_c = c;
                    }
                }
            }
        }
    }

    return best_region;
}

int main(int argc, char* argv[]) {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    double lambda = 0.85;
    if (argc > 1) {
        lambda = atof(argv[1]);
    }

    int n, m;
    double r;
    if (!(cin >> n >> m >> r)) return 0;

    vector<string> grid(n);
    for (int i = 0; i < n; ++i) {
        cin >> grid[i];
    }

    vector<vector<int>> owner(n, vector<int>(n, -1));
    vector<GroupInfo> groups(m);

    int accepted_cnt = 0;
    int rejected_cnt = 0;
    double sum_compactness = 0.0;

    for (int i = 0; i < m; ++i) {
        int gi, s, t, p;
        long long v;
        cin >> gi >> s >> t >> p >> v;

        groups[i].id = gi;
        groups[i].s = s;
        groups[i].t = t;
        groups[i].p = p;
        groups[i].v = v;

        // Release groups whose departure time is before the current time s
        for (int j = 0; j < i; ++j) {
            if (!groups[j].pos.empty() && groups[j].t < s) {
                for (const auto& cell : groups[j].pos) {
                    owner[cell.first][cell.second] = -1;
                }
                groups[j].pos.clear();
            }
        }

        // Do not perform any move
        cout << 0 << "\n";

        double chosen_c = 0.0;
        vector<pair<int, int>> region = find_best_region(grid, owner, n, p, lambda, chosen_c);
        if (!region.empty()) {
            for (const auto& cell : region) {
                owner[cell.first][cell.second] = i;
            }
            groups[i].pos = region;
            groups[i].c = chosen_c;
            sum_compactness += groups[i].c;
            accepted_cnt++;

            cout << "Yes\n";
            for (const auto& cell : region) {
                cout << cell.first << " " << cell.second << "\n";
            }
        } else {
            rejected_cnt++;
            cout << "No\n";
        }
        cout.flush();
    }

    if (accepted_cnt > 0) {
        cerr << "METRICS: accepted=" << accepted_cnt << " rejected=" << rejected_cnt 
             << " avg_c=" << (sum_compactness / accepted_cnt) << "\n";
    }

    return 0;
}
