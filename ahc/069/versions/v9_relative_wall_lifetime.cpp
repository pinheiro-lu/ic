#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <cmath>
#include <array>
#include <algorithm>

using namespace std;

const int DIRS[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
const int DIRS8[8][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

array<array<char, 50>, 50> grid;
array<array<int, 50>, 50> owner;

vector<pair<int, int>> find_region_from_seed(int n, int p, int sx, int sy) {
    struct Cell {
        int cheb_dist;
        int x;
        int y;

        bool operator>(const Cell& other) const {
            return cheb_dist > other.cheb_dist;
        }
    };
    static array<array<int, 50>, 50> visited_token;
    static int cur_vis_token = 0;

    vector<pair<int, int>> seed_region_buf;
    seed_region_buf.reserve(p);
    cur_vis_token++;
    visited_token[sx][sy] = cur_vis_token;

    priority_queue<Cell, vector<Cell>, greater<Cell>> pq;
    pq.push({0, sx, sy});

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
    static array<array<int, 50>, 50> is_in_region_token;
    static int cur_reg_token = 0;

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

array<int, 1000> group_depart_time;

double calculate_wall_contact_relative(const vector<pair<int, int>>& region, int n, int cur_s, int cur_t) {
    double total_persistence = 0.0;
    int my_duration = cur_t - cur_s;

    for (const pair<int, int>& cell : region) {
        int x = cell.first;
        int y = cell.second;
        for (int d = 0; d < 4; ++d) {
            int nx = x + DIRS[d][0];
            int ny = y + DIRS[d][1];
            if (nx < 0 || nx >= n || ny < 0 || ny >= n || grid[nx][ny] == '#') {
                total_persistence += 1.0;
            } else if (owner[nx][ny] != -1) {
                int owner_id = owner[nx][ny];
                int neighbor_rem = group_depart_time[owner_id] - cur_s;
                double ratio = min(1.0, (double)neighbor_rem / my_duration);
                total_persistence += ratio;
            }
        }
    }
    return total_persistence;
}

pair<double, vector<pair<int, int>>> find_best_region(int n, int p, int cur_s, int cur_t) {
    vector<pair<int, int>> best_region_buf;
    double chosen_c = 0;
    double best_score = -1e9;

    // 1. Multi-source 8-directional BFS for Chebyshev Distance Transform D[x][y]
    array<array<int, 50>, 50> dist;
    queue<pair<int, int>> q;

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

        for (int d = 0; d < 8; ++d) {
            int nx = cx + DIRS8[d][0];
            int ny = cy + DIRS8[d][1];
            if (nx >= 0 && nx < n && ny >= 0 && ny < n && dist[nx][ny] == -1) {
                dist[nx][ny] = dist[cx][cy] + 1;
                q.push({nx, ny});
            }
        }
    }

#ifndef ALPHA_WALL
#define ALPHA_WALL 0.5
#endif

#ifndef POS_PENALTY_WEIGHT
#define POS_PENALTY_WEIGHT 0.001
#endif

#ifndef NEG_PENALTY_WEIGHT
#define NEG_PENALTY_WEIGHT 0.0005
#endif

#ifndef MAX_CANDIDATES
#define MAX_CANDIDATES 667
#endif

    // 2. Analytical center clearance to obstacle: D_target = R + 1 = (sqrt(p) - 1) / 2 + 1
    double target_dist = (sqrt(p) + 1) / 2;

    // 3. Collect and sort candidate seeds by directional diff penalty
    struct Candidate {
        double penalty;
        int x, y;
        bool operator<(const Candidate& other) const {
            return penalty < other.penalty;
        }
    };

    vector<Candidate> candidates;
    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
            if (grid[x][y] == '.' && owner[x][y] == -1) {
                double diff = dist[x][y] - target_dist;
                double penalty = (diff >= 0) ? (POS_PENALTY_WEIGHT * diff) : (NEG_PENALTY_WEIGHT * (-diff));
                candidates.push_back({penalty, x, y});
            }
        }
    }

    sort(candidates.begin(), candidates.end());

    // 4. Evaluate up to MAX_CANDIDATES top seeds
    int limit = min((int)candidates.size(), MAX_CANDIDATES);
    for (int i = 0; i < limit; ++i) {
        int x = candidates[i].x;
        int y = candidates[i].y;

        const auto region = find_region_from_seed(n, p, x, y);
        if (!region.empty()) {
            double c = calculate_compactness(region, p, n);
            double wall_persistence = calculate_wall_contact_relative(region, n, cur_s, cur_t);
            double w_norm = wall_persistence / (2 * p + 2);
            double score = (1.0 - ALPHA_WALL) * c + ALPHA_WALL * w_norm;

            if (score > best_score) {
                best_score = score;
                best_region_buf = region;
                chosen_c = c;
            }
        }
    }

    return {chosen_c, best_region_buf};
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

    int remaining_free_lawn = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cin >> grid[i][j];
            if (grid[i][j] == '.') {
                remaining_free_lawn++;
            }
        }
    }

    struct GroupInfo {
        int id;
        int s;
        int t;
        int p;
        int v;
        double c;
        vector<pair<int, int>> pos;
    };

    array<GroupInfo, 1000> groups;

    int accepted_cnt = 0;
    int rejected_cnt = 0;
    double sum_compactness = 0.0;
    long long total_score = 0;

    for (int i = 0; i < m; ++i) {
        int gi, s, t, p, v;
        cin >> gi >> s >> t >> p >> v;

        groups[i].id = gi;
        groups[i].s = s;
        groups[i].t = t;
        groups[i].p = p;
        groups[i].v = v;
        group_depart_time[i] = t;

        // Release groups whose departure time is before the current time s
        for (int j = 0; j < i; ++j) {
            if (!groups[j].pos.empty() && groups[j].t < s) {
                for (const pair<int, int>& cell : groups[j].pos) {
                    owner[cell.first][cell.second] = -1;
                    remaining_free_lawn++;
                }
                groups[j].pos.clear();
            }
        }

        // Do not perform any move
        cout << 0 << "\n";

        const auto& [chosen_c, region] = find_best_region(n, p, s, t);
        if (!region.empty()) {
            for (const pair<int, int>& cell : region) {
                owner[cell.first][cell.second] = i;
            }
            remaining_free_lawn -= region.size();
            groups[i].pos = region;
            groups[i].c = chosen_c;
            sum_compactness += groups[i].c;
            accepted_cnt++;
            total_score += (long long)floor(v * chosen_c);

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
        cerr << "METRICS: score=" << total_score << " accepted=" << accepted_cnt << " rejected=" << rejected_cnt 
             << " avg_c=" << (sum_compactness / accepted_cnt) << "\n";
    }

    return 0;
}
