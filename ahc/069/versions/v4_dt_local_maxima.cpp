#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <cmath>
#include <array>
#include <algorithm>

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

static array<array<char, 50>, 50> grid;
static array<array<int, 50>, 50> owner;
static array<array<int, 50>, 50> visited_token;
static array<array<int, 50>, 50> is_in_region_token;
static int cur_vis_token = 0;
static int cur_reg_token = 0;
static double chosen_c = 0.0;

static vector<pair<int, int>> seed_region_buf;
static vector<pair<int, int>> best_region_buf;

const vector<pair<int, int>>& find_region_from_seed(int n, int p, int sx, int sy) {
    struct Cell {
        int cheb_dist;
        int x;
        int y;

        bool operator>(const Cell& other) const {
            return cheb_dist > other.cheb_dist;
        }
    };

    seed_region_buf.clear();
    cur_vis_token++;
    visited_token[sx][sy] = cur_vis_token;

    priority_queue<Cell, vector<Cell>, greater<Cell>> pq;
    pq.push({0, sx, sy});
    seed_region_buf.reserve(p);

    while (!pq.empty() && (int)seed_region_buf.size() < p) {
        Cell curr = pq.top();
        pq.pop();
        seed_region_buf.push_back({curr.x, curr.y});

        for (int d = 0; d < 4; ++d) {
            int nx = curr.x + DIRS[d][0];
            int ny = curr.y + DIRS[d][1];
            if (nx >= 0 && nx < n && ny >= 0 && ny < n && visited_token[nx][ny] != cur_vis_token && grid[nx][ny] == '.' && owner[nx][ny] == -1) {
                visited_token[nx][ny] = cur_vis_token;
                int cheb = max(abs(nx - sx), abs(ny - sy));
                pq.push({cheb, nx, ny});
            }
        }
    }

    if ((int)seed_region_buf.size() != p) {
        seed_region_buf.clear();
    }
    return seed_region_buf;
}

double calculate_compactness(const vector<pair<int, int>>& region, int p, int n) {
    cur_reg_token++;
    for (const pair<int, int>& cell : region) {
        is_in_region_token[cell.first][cell.second] = cur_reg_token;
    }

    int perimeter = 0;
    for (const pair<int, int>& cell : region) {
        int x = cell.first;
        int y = cell.second;
        for (int d = 0; d < 4; ++d) {
            int nx = x + DIRS[d][0];
            int ny = y + DIRS[d][1];
            if (nx < 0 || nx >= n || ny < 0 || ny >= n || is_in_region_token[nx][ny] != cur_reg_token) {
                perimeter++;
            }
        }
    }
    return 4.0 * sqrt(p) / perimeter;
}

const vector<pair<int, int>>& find_best_region(int n, int p) {
    double best_c = -1.0;
    best_region_buf.clear();
    chosen_c = 0.0;

    // Multi-source BFS for Distance Transform D[x][y]
    array<array<int, 50>, 50> dist;
    queue<pair<int, int>> q;

    // Block 1: Initialize obstacles ('#') and occupied cells as distance 0 sources
    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
            if (grid[x][y] == '#' || owner[x][y] != -1) {
                dist[x][y] = 0;
                q.push({x, y});
            } else {
                dist[x][y] = -1;
            }
        }
    }

    // Block 2: Initialize outer grid boundaries as distance 1 sources
    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
            if (dist[x][y] == -1 && (x == 0 || x == n - 1 || y == 0 || y == n - 1)) {
                dist[x][y] = 1;
                q.push({x, y});
            }
        }
    }

    while (!q.empty()) {
        pair<int, int> curr = q.front();
        q.pop();
        int cx = curr.first;
        int cy = curr.second;

        for (int d = 0; d < 4; ++d) {
            int nx = cx + DIRS[d][0];
            int ny = cy + DIRS[d][1];
            if (nx >= 0 && nx < n && ny >= 0 && ny < n && dist[nx][ny] == -1) {
                dist[nx][ny] = dist[cx][cy] + 1;
                q.push({nx, ny});
            }
        }
    }

    // Traverse grid and launch BFS immediately on local maxima
    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
            if (grid[x][y] == '.' && owner[x][y] == -1) {
                bool is_max = true;
                for (int d = 0; d < 4; ++d) {
                    int nx = x + DIRS[d][0];
                    int ny = y + DIRS[d][1];
                    if (nx >= 0 && nx < n && ny >= 0 && ny < n && grid[nx][ny] == '.' && owner[nx][ny] == -1) {
                        if (dist[nx][ny] > dist[x][y]) {
                            is_max = false;
                            break;
                        }
                    }
                }
                if (is_max) {
                    const vector<pair<int, int>>& region = find_region_from_seed(n, p, x, y);
                    if (!region.empty()) {
                        double c = calculate_compactness(region, p, n);
                        if (c > best_c) {
                            best_c = c;
                            best_region_buf = region;
                            chosen_c = c;
                        }
                    }
                }
            }
        }
    }

    return best_region_buf;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int n, m;
    double r;
    if (!(cin >> n >> m >> r)) return 0;

    for (array<int, 50>& row : owner) {
        row.fill(-1);
    }

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cin >> grid[i][j];
        }
    }

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
                for (const pair<int, int>& cell : groups[j].pos) {
                    owner[cell.first][cell.second] = -1;
                }
                groups[j].pos.clear();
            }
        }

        // Do not perform any move
        cout << 0 << "\n";

        const vector<pair<int, int>>& region = find_best_region(n, p);
        if (!region.empty()) {
            for (const pair<int, int>& cell : region) {
                owner[cell.first][cell.second] = i;
            }
            groups[i].pos = region;
            groups[i].c = chosen_c;
            sum_compactness += groups[i].c;
            accepted_cnt++;

            cout << "Yes\n";
            for (const pair<int, int>& cell : region) {
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
