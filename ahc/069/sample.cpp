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

struct GroupInfo {
	int id;
	int s;
	int t;
	int p;
	int v;
	double c;
	vector<pair<int, int>> pos;
};

const int INF_TIME = 1e9;

array<array<int, 50>, 50> free_at;
array<GroupInfo, 1000> groups;

bool find_region_from_seed(int n, int p, int sx, int sy, int cur_s, vector<pair<int, int>>& scratch_region_buf) {
	struct Cell {
		int cheb_dist;
		int parent_idx; // Sequence index of parent cell that discovered it
		int x;
		int y;

		bool operator>(const Cell& other) const {
			if (cheb_dist != other.cheb_dist) {
				return cheb_dist > other.cheb_dist;
			}
			return parent_idx > other.parent_idx; // Wavefront spiral tie-breaker!
		}
	};
	static array<array<int, 50>, 50> visited_token;
	static int cur_vis_token = 0;

	scratch_region_buf.clear();
    cur_vis_token++;
    visited_token[sx][sy] = cur_vis_token;

    priority_queue<Cell, vector<Cell>, greater<Cell>> pq;
    pq.push({0, 0, sx, sy});

    while (!pq.empty() && (int)scratch_region_buf.size() < p) {
        Cell curr = pq.top();
        pq.pop();
        int curr_idx = (int)scratch_region_buf.size();
        scratch_region_buf.push_back({curr.x, curr.y});

        for (int d = 0; d < 4; ++d) {
            int nx = curr.x + DIRS[d][0];
            int ny = curr.y + DIRS[d][1];
            if (nx >= 0 && nx < n && ny >= 0 && ny < n && visited_token[nx][ny] != cur_vis_token && free_at[nx][ny] < cur_s) {
                visited_token[nx][ny] = cur_vis_token;
                int cheb = max(abs(nx - sx), abs(ny - sy));
                pq.push({cheb, curr_idx, nx, ny});
            }
        }
    }

	return scratch_region_buf.size() == p;
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
    return 4 * sqrt(p) / perimeter;
}

static int orig_room_id[50][50];
static vector<pair<int, int>> room_cells[2500];
static int room_capacity[2500];
static int room_biggest_pocket_size[2500];
static int room_seed_x[2500];
static int room_seed_y[2500];
static int num_orig_rooms = 0;

void update_room_biggest_pocket(int rid, int n, int cur_s) {
    static array<array<bool, 50>, 50> vis;
    static int vis_token = 0;
    vis_token++;

    int max_p_size = 0;
    int best_sx = -1, best_sy = -1;

    for (const auto& cell : room_cells[rid]) {
        int x = cell.first;
        int y = cell.second;

        if (free_at[x][y] < cur_s && !vis[x][y]) {
            int p_size = 0;
            queue<pair<int, int>> q;
            q.push({x, y});
            vis[x][y] = true;

            while (!q.empty()) {
                auto curr = q.front();
                q.pop();
                p_size++;

                for (int d = 0; d < 4; ++d) {
                    int nx = curr.first + DIRS[d][0];
                    int ny = curr.second + DIRS[d][1];
                    if (nx >= 0 && nx < n && ny >= 0 && ny < n && free_at[nx][ny] < cur_s && !vis[nx][ny]) {
                        vis[nx][ny] = true;
                        q.push({nx, ny});
                    }
                }
            }

            if (p_size > max_p_size) {
                max_p_size = p_size;
                best_sx = x;
                best_sy = y;
            }
        }
    }

    room_biggest_pocket_size[rid] = max_p_size;
    room_seed_x[rid] = best_sx;
    room_seed_y[rid] = best_sy;
}

void init_original_rooms(int n) {
    static array<array<bool, 50>, 50> visited;
    num_orig_rooms = 0;

    for (int rid = 0; rid < 2500; ++rid) {
        room_cells[rid].clear();
    }

    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
            orig_room_id[x][y] = -1;
        }
    }

    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
            if (free_at[x][y] == -1 && !visited[x][y]) {
                int rid = num_orig_rooms++;
                int csize = 0;

                queue<pair<int, int>> q;
                q.push({x, y});
                visited[x][y] = true;
                orig_room_id[x][y] = rid;

                while (!q.empty()) {
                    auto curr = q.front();
                    q.pop();
                    csize++;
                    room_cells[rid].push_back(curr);

                    for (int d = 0; d < 4; ++d) {
                        int nx = curr.first + DIRS[d][0];
                        int ny = curr.second + DIRS[d][1];
                        if (nx >= 0 && nx < n && ny >= 0 && ny < n && free_at[nx][ny] == -1 && !visited[nx][ny]) {
                            visited[nx][ny] = true;
                            orig_room_id[nx][ny] = rid;
                            q.push({nx, ny});
                        }
                    }
                }
                room_capacity[rid] = csize;
                update_room_biggest_pocket(rid, n, 0);
            }
        }
    }
}

bool find_pure_packing_region_from_seed(int n, int p, int cur_s, int sx, int sy, vector<pair<int, int>>& region_out) {
    if (sx < 0 || sx >= n || sy < 0 || sy >= n || free_at[sx][sy] > cur_s) return false;

    auto get_wall_contact = [&](int cx, int cy) {
        int count = 0;
        for (int d = 0; d < 4; ++d) {
            int nx = cx + DIRS[d][0];
            int ny = cy + DIRS[d][1];
            if (nx < 0 || nx >= n || ny < 0 || ny >= n || free_at[nx][ny] > cur_s) {
                count++;
            }
        }
        return count;
    };

    struct WallCell {
        int contact;
        int x;
        int y;
        bool operator<(const WallCell& other) const {
            return contact < other.contact;
        }
    };

    static array<array<int, 50>, 50> vis_token;
    static int cur_vis = 0;
    cur_vis++;

    region_out.clear();
    priority_queue<WallCell> pq;
    pq.push({get_wall_contact(sx, sy), sx, sy});
    vis_token[sx][sy] = cur_vis;

    while (!pq.empty() && (int)region_out.size() < p) {
        WallCell curr = pq.top();
        pq.pop();
        region_out.push_back({curr.x, curr.y});

        for (int d = 0; d < 4; ++d) {
            int nx = curr.x + DIRS[d][0];
            int ny = curr.y + DIRS[d][1];
            if (nx >= 0 && nx < n && ny >= 0 && ny < n && vis_token[nx][ny] != cur_vis && free_at[nx][ny] < cur_s) {
                vis_token[nx][ny] = cur_vis;
                pq.push({get_wall_contact(nx, ny), nx, ny});
            }
        }
    }

    return (int)region_out.size() == p;
}

#ifndef ALPHA_WALL
#define ALPHA_WALL 0.50
#endif

double calculate_wall_contact_relative(const vector<pair<int, int>>& region, int n, int cur_s, int cur_t) {
    double total_persistence = 0;
    int my_duration = cur_t - cur_s;

    for (const pair<int, int>& cell : region) {
        int x = cell.first;
        int y = cell.second;
        for (int d = 0; d < 4; ++d) {
            int nx = x + DIRS[d][0];
            int ny = y + DIRS[d][1];
            if (nx < 0 || nx >= n || ny < 0 || ny >= n) {
                total_persistence += 1.0;
            } else if (free_at[nx][ny] > cur_s) {
                int neighbor_rem = free_at[nx][ny] - cur_s;
                total_persistence += min(1.0, (double)neighbor_rem / my_duration);
            }
        }
    }
    return total_persistence;
}

bool find_pure_packing_region(int n, int p, int cur_s, int cur_t, int v_val, vector<pair<int, int>>& scratch_region_buf) {
    struct RoomCand {
        int rem_free;
        int sx, sy;
        bool operator<(const RoomCand& o) const {
            return rem_free < o.rem_free;
        }
    };

    vector<RoomCand> cand_rooms;
    for (int rid = 0; rid < num_orig_rooms; ++rid) {
        int rem_free = room_biggest_pocket_size[rid];
        if (rem_free >= p) {
            cand_rooms.push_back({rem_free, room_seed_x[rid], room_seed_y[rid]});
        }
    }

    if (cand_rooms.empty()) return false;

    sort(cand_rooms.begin(), cand_rooms.end());

    int k_limit = (int)cand_rooms.size(); // Test EVERY room!
    double best_pts = -1.0;
    vector<pair<int, int>> best_region;

    for (int k = 0; k < k_limit; ++k) {
        int sx = cand_rooms[k].sx;
        int sy = cand_rooms[k].sy;

        vector<pair<int, int>> cur_region;
        if (find_pure_packing_region_from_seed(n, p, cur_s, sx, sy, cur_region)) {
            double c = calculate_compactness(cur_region, p, n);
            double pts = round((double)v_val * c);

            if (pts > best_pts) {
                best_pts = pts;
                best_region = move(cur_region);
            }
        }
    }

    if (best_region.empty()) return false;

    scratch_region_buf = move(best_region);
    return true;
}

pair<double, vector<pair<int, int>>> find_best_region(int n, int p, int cur_s, int cur_t) {
    static vector<pair<int, int>> buf_a = [] {
        vector<pair<int, int>> v; v.reserve(2500); return v;
    }();
    static vector<pair<int, int>> buf_b = [] {
        vector<pair<int, int>> v; v.reserve(2500); return v;
    }();

    vector<pair<int, int>>* cur_buf = &buf_a;
    vector<pair<int, int>>* best_buf = &buf_b;
    best_buf->clear();

    double chosen_c = 0;
    double best_score = 0;

    // 1. 2-Pass Linear O(N^2) Chebyshev Distance Transform D[x][y]
    static array<array<int, 50>, 50> dist;
    const int INF_DIST = 9999;

    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
            if (free_at[x][y] > cur_s) {
                dist[x][y] = 0;
            } else if (x == 0 || x == n - 1 || y == 0 || y == n - 1) {
                dist[x][y] = 1;
            } else {
                dist[x][y] = INF_DIST;
            }
        }
    }

    // Pass 1: Top-Left to Bottom-Right
    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
            if (dist[x][y] == 0 || dist[x][y] == 1) continue;
            int m = dist[x][y];
            if (x > 0) m = min(m, dist[x - 1][y] + 1);
            if (y > 0) m = min(m, dist[x][y - 1] + 1);
            if (x > 0 && y > 0) m = min(m, dist[x - 1][y - 1] + 1);
            if (x > 0 && y < n - 1) m = min(m, dist[x - 1][y + 1] + 1);
            dist[x][y] = m;
        }
    }

    // Pass 2: Bottom-Right to Top-Left
    for (int x = n - 1; x >= 0; --x) {
        for (int y = n - 1; y >= 0; --y) {
            int m = dist[x][y];
            if (x < n - 1) m = min(m, dist[x + 1][y] + 1);
            if (y < n - 1) m = min(m, dist[x][y + 1] + 1);
            if (x < n - 1 && y < n - 1) m = min(m, dist[x + 1][y + 1] + 1);
            if (x < n - 1 && y > 0) m = min(m, dist[x + 1][y - 1] + 1);
            dist[x][y] = m;
        }
    }

#ifndef ALPHA_WALL
#define ALPHA_WALL 0.50
#endif

#ifndef POS_PENALTY_WEIGHT
#define POS_PENALTY_WEIGHT 0.001
#endif

#ifndef NEG_PENALTY_WEIGHT
#define NEG_PENALTY_WEIGHT 0.0005
#endif

#ifndef MAX_CANDIDATES
#define MAX_CANDIDATES 350
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

    static vector<Candidate> candidates = [] {
        vector<Candidate> v; v.reserve(2500); return v;
    }();
    candidates.clear();
    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
            if (free_at[x][y] < cur_s) {
                double diff = dist[x][y] - target_dist;
                double penalty = (diff > 0) ? (POS_PENALTY_WEIGHT * diff) : (NEG_PENALTY_WEIGHT * (-diff));
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

        if (find_region_from_seed(n, p, x, y, cur_s, *cur_buf)) {
            double c = calculate_compactness(*cur_buf, p, n);
            double wall_persistence = calculate_wall_contact_relative(*cur_buf, n, cur_s, cur_t);
            double w_norm = wall_persistence / (2 * p + 2);
            double score = (1 - ALPHA_WALL) * c + ALPHA_WALL * w_norm;

            if (score > best_score) {
                best_score = score;
                swap(cur_buf, best_buf); // O(1) Pointer Swap! Zero copies!
                chosen_c = c;
            }
        }
    }

    return {chosen_c, *best_buf};
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int n, m;
    double r;
    cin >> n >> m >> r;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            char ch;
            cin >> ch;
            if (ch == '.') {
                free_at[i][j] = -1;
            } else {
                free_at[i][j] = INF_TIME;
            }
        }
    }

    init_original_rooms(n);

    for (int i = 0; i < m; ++i) {
        int gi, s, t, p, v;
        cin >> gi >> s >> t >> p >> v;

        groups[i].id = gi;
        groups[i].s = s;
        groups[i].t = t;
        groups[i].p = p;
        groups[i].v = v;

        // Process group departures and update biggest pocket for affected rooms
        for (int g = 0; g < i; ++g) {
            if (!groups[g].pos.empty() && groups[g].t <= s) {
                int rid = orig_room_id[groups[g].pos[0].first][groups[g].pos[0].second];

                for (const auto& cell : groups[g].pos) {
                    free_at[cell.first][cell.second] = -1;
                }
                groups[g].pos.clear();
                if (rid != -1) update_room_biggest_pocket(rid, n, s);
            }
        }

        auto [chosen_c, region] = find_best_region(n, p, s, t);
        double normal_pts = (!region.empty()) ? round((double)v * chosen_c) : -1.0e18;
        vector<pair<int, int>> normal_region = region;

        int best_g_idx = -1;
        vector<pair<int, int>> best_g_new_region;
        vector<pair<int, int>> best_x_new_region;
        double best_g_c = 0;
        double best_x_c = 0;
        double max_gain = normal_pts; // Must beat normal placement points!

            for (int g = 0; g < i; ++g) {
                if (groups[g].pos.empty() || groups[g].t <= s) continue;

                int g_rid = orig_room_id[groups[g].pos[0].first][groups[g].pos[0].second];

                // 1. Temporarily remove group G from free_at and update room biggest pocket
                for (const pair<int, int>& cell : groups[g].pos) {
                    free_at[cell.first][cell.second] = -1;
                }
                if (g_rid != -1) update_room_biggest_pocket(g_rid, n, s);

                if (v >= groups[g].v) {
                    // Case A: Group X is MORE valuable -> FIRST normally pack X on freed lawn, THEN corner-pack G
                    auto [c_x, region_x] = find_best_region(n, p, s, t);
                    if (!region_x.empty()) {
                        for (const pair<int, int>& cell : region_x) free_at[cell.first][cell.second] = t;

                        vector<pair<int, int>> region_g;
                        if (find_pure_packing_region(n, groups[g].p, s, groups[g].t, groups[g].v, region_g)) {
                            double c_g = calculate_compactness(region_g, groups[g].p, n);
                            long long cost = max(1LL, (long long)round(groups[g].v * r));
                            double old_pts = round((double)groups[g].v * groups[g].c);
                            double new_pts_g = round((double)groups[g].v * c_g);
                            double new_pts_x = round((double)v * c_x);
                            double gain = new_pts_x + new_pts_g - old_pts - (double)cost;

                            if (gain > max_gain) {
                                max_gain = gain;
                                best_g_idx = g;
                                best_g_new_region = region_g;
                                best_x_new_region = region_x;
                                best_g_c = c_g;
                                best_x_c = c_x;
                            }
                        }

                        for (const pair<int, int>& cell : region_x) free_at[cell.first][cell.second] = -1;
                    }
                } else {
                    // Case B: Group G is MORE valuable -> FIRST normally pack G on freed lawn, THEN corner-pack X
                    auto [c_g, region_g] = find_best_region(n, groups[g].p, s, groups[g].t);
                    if (!region_g.empty()) {
                        for (const pair<int, int>& cell : region_g) free_at[cell.first][cell.second] = groups[g].t;

                        vector<pair<int, int>> region_x;
                        if (find_pure_packing_region(n, p, s, t, v, region_x)) {
                            double c_x = calculate_compactness(region_x, p, n);
                            long long cost = max(1LL, (long long)round(groups[g].v * r));
                            double old_pts = round((double)groups[g].v * groups[g].c);
                            double new_pts_g = round((double)groups[g].v * c_g);
                            double new_pts_x = round((double)v * c_x);
                            double gain = new_pts_x + new_pts_g - old_pts - (double)cost;

                            if (gain > max_gain) {
                                max_gain = gain;
                                best_g_idx = g;
                                best_g_new_region = region_g;
                                best_x_new_region = region_x;
                                best_g_c = c_g;
                                best_x_c = c_x;
                            }
                        }

                        for (const pair<int, int>& cell : region_g) free_at[cell.first][cell.second] = -1;
                    }
                }

                // Restore G's original position in free_at and room biggest pocket
                for (const pair<int, int>& cell : groups[g].pos) {
                    free_at[cell.first][cell.second] = groups[g].t;
                }
                if (g_rid != -1) update_room_biggest_pocket(g_rid, n, s);
            }

            // 2-Group Move Shifts Search
            int best_g1_idx = -1, best_g2_idx = -1;
            vector<pair<int, int>> best_g1_new_region, best_g2_new_region;
            double best_g1_c = 0, best_g2_c = 0;

            for (int g1 = 0; g1 < i; ++g1) {
                if (groups[g1].pos.empty() || groups[g1].t <= s) continue;
                int g1_rid = orig_room_id[groups[g1].pos[0].first][groups[g1].pos[0].second];

                // Temporarily remove G1
                for (const pair<int, int>& cell : groups[g1].pos) free_at[cell.first][cell.second] = -1;
                if (g1_rid != -1) update_room_biggest_pocket(g1_rid, n, s);

                for (int g2 = g1 + 1; g2 < i; ++g2) {
                    if (groups[g2].pos.empty() || groups[g2].t <= s) continue;
                    int g2_rid = orig_room_id[groups[g2].pos[0].first][groups[g2].pos[0].second];

                    // Temporarily remove G2
                    for (const pair<int, int>& cell : groups[g2].pos) free_at[cell.first][cell.second] = -1;
                    if (g2_rid != -1) update_room_biggest_pocket(g2_rid, n, s);

                    // Value-Prioritized 3-Group Packing:
                    // FIRST corner-pack the 2 LESS valuable groups, THEN normally pack the MOST valuable group
                    int v_x = v, v1 = groups[g1].v, v2 = groups[g2].v;
                    int max_v = max({v_x, v1, v2});

                    if (max_v == v_x) {
                        // Case 1: X is Most Valuable -> Corner-pack G1 & G2, then normally pack X
                        vector<pair<int, int>> region_g1, region_g2;
                        if (find_pure_packing_region(n, groups[g1].p, s, groups[g1].t, groups[g1].v, region_g1)) {
                            for (const pair<int, int>& cell : region_g1) free_at[cell.first][cell.second] = groups[g1].t;

                            if (find_pure_packing_region(n, groups[g2].p, s, groups[g2].t, groups[g2].v, region_g2)) {
                                for (const pair<int, int>& cell : region_g2) free_at[cell.first][cell.second] = groups[g2].t;

                                auto [c_x, region_x] = find_best_region(n, p, s, t);
                                if (!region_x.empty()) {
                                    double c_g1 = calculate_compactness(region_g1, groups[g1].p, n);
                                    double c_g2 = calculate_compactness(region_g2, groups[g2].p, n);
                                    long long cost1 = max(1LL, (long long)round(groups[g1].v * r));
                                    long long cost2 = max(1LL, (long long)round(groups[g2].v * r));

                                    double old_pts = round((double)groups[g1].v * groups[g1].c) + round((double)groups[g2].v * groups[g2].c);
                                    double new_pts = round((double)v * c_x) + round((double)groups[g1].v * c_g1) + round((double)groups[g2].v * c_g2);
                                    double gain = new_pts - old_pts - (double)(cost1 + cost2);

                                    if (gain > max_gain) {
                                        max_gain = gain;
                                        best_g_idx = -2;
                                        best_g1_idx = g1; best_g2_idx = g2;
                                        best_g1_new_region = region_g1; best_g2_new_region = region_g2; best_x_new_region = region_x;
                                        best_g1_c = c_g1; best_g2_c = c_g2; best_x_c = c_x;
                                    }
                                }
                                for (const pair<int, int>& cell : region_g2) free_at[cell.first][cell.second] = -1;
                            }
                            for (const pair<int, int>& cell : region_g1) free_at[cell.first][cell.second] = -1;
                        }
                    } else if (max_v == v1) {
                        // Case 2: G1 is Most Valuable -> Corner-pack X & G2, then normally pack G1
                        vector<pair<int, int>> region_x, region_g2;
                        if (find_pure_packing_region(n, p, s, t, v, region_x)) {
                            for (const pair<int, int>& cell : region_x) free_at[cell.first][cell.second] = t;

                            if (find_pure_packing_region(n, groups[g2].p, s, groups[g2].t, groups[g2].v, region_g2)) {
                                for (const pair<int, int>& cell : region_g2) free_at[cell.first][cell.second] = groups[g2].t;

                                auto [c_g1, region_g1] = find_best_region(n, groups[g1].p, s, groups[g1].t);
                                if (!region_g1.empty()) {
                                    double c_x = calculate_compactness(region_x, p, n);
                                    double c_g2 = calculate_compactness(region_g2, groups[g2].p, n);
                                    long long cost1 = max(1LL, (long long)round(groups[g1].v * r));
                                    long long cost2 = max(1LL, (long long)round(groups[g2].v * r));

                                    double old_pts = round((double)groups[g1].v * groups[g1].c) + round((double)groups[g2].v * groups[g2].c);
                                    double new_pts = round((double)v * c_x) + round((double)groups[g1].v * c_g1) + round((double)groups[g2].v * c_g2);
                                    double gain = new_pts - old_pts - (double)(cost1 + cost2);

                                    if (gain > max_gain) {
                                        max_gain = gain;
                                        best_g_idx = -2;
                                        best_g1_idx = g1; best_g2_idx = g2;
                                        best_g1_new_region = region_g1; best_g2_new_region = region_g2; best_x_new_region = region_x;
                                        best_g1_c = c_g1; best_g2_c = c_g2; best_x_c = c_x;
                                    }
                                }
                                for (const pair<int, int>& cell : region_g2) free_at[cell.first][cell.second] = -1;
                            }
                            for (const pair<int, int>& cell : region_x) free_at[cell.first][cell.second] = -1;
                        }
                    } else {
                        // Case 3: G2 is Most Valuable -> Corner-pack X & G1, then normally pack G2
                        vector<pair<int, int>> region_x, region_g1;
                        if (find_pure_packing_region(n, p, s, t, v, region_x)) {
                            for (const pair<int, int>& cell : region_x) free_at[cell.first][cell.second] = t;

                            if (find_pure_packing_region(n, groups[g1].p, s, groups[g1].t, groups[g1].v, region_g1)) {
                                for (const pair<int, int>& cell : region_g1) free_at[cell.first][cell.second] = groups[g1].t;

                                auto [c_g2, region_g2] = find_best_region(n, groups[g2].p, s, groups[g2].t);
                                if (!region_g2.empty()) {
                                    double c_x = calculate_compactness(region_x, p, n);
                                    double c_g1 = calculate_compactness(region_g1, groups[g1].p, n);
                                    long long cost1 = max(1LL, (long long)round(groups[g1].v * r));
                                    long long cost2 = max(1LL, (long long)round(groups[g2].v * r));

                                    double old_pts = round((double)groups[g1].v * groups[g1].c) + round((double)groups[g2].v * groups[g2].c);
                                    double new_pts = round((double)v * c_x) + round((double)groups[g1].v * c_g1) + round((double)groups[g2].v * c_g2);
                                    double gain = new_pts - old_pts - (double)(cost1 + cost2);

                                    if (gain > max_gain) {
                                        max_gain = gain;
                                        best_g_idx = -2;
                                        best_g1_idx = g1; best_g2_idx = g2;
                                        best_g1_new_region = region_g1; best_g2_new_region = region_g2; best_x_new_region = region_x;
                                        best_g1_c = c_g1; best_g2_c = c_g2; best_x_c = c_x;
                                    }
                                }
                                for (const pair<int, int>& cell : region_g1) free_at[cell.first][cell.second] = -1;
                            }
                            for (const pair<int, int>& cell : region_x) free_at[cell.first][cell.second] = -1;
                        }
                    }

                    // Restore G2
                    for (const pair<int, int>& cell : groups[g2].pos) free_at[cell.first][cell.second] = groups[g2].t;
                    if (g2_rid != -1) update_room_biggest_pocket(g2_rid, n, s);
                }

                // Restore G1
                for (const pair<int, int>& cell : groups[g1].pos) free_at[cell.first][cell.second] = groups[g1].t;
                if (g1_rid != -1) update_room_biggest_pocket(g1_rid, n, s);
            }

            if (best_g_idx == -2) {
                // Output 2 moves
                cout << 2 << "\n";
                cout << best_g1_idx << "\n";
                for (const pair<int, int>& cell : best_g1_new_region) cout << cell.first << " " << cell.second << "\n";
                cout << best_g2_idx << "\n";
                for (const pair<int, int>& cell : best_g2_new_region) cout << cell.first << " " << cell.second << "\n";

                // Update free_at for G1 and G2
                int old_r1 = orig_room_id[groups[best_g1_idx].pos[0].first][groups[best_g1_idx].pos[0].second];
                for (const pair<int, int>& cell : groups[best_g1_idx].pos) free_at[cell.first][cell.second] = -1;
                if (old_r1 != -1) update_room_biggest_pocket(old_r1, n, s);

                int old_r2 = orig_room_id[groups[best_g2_idx].pos[0].first][groups[best_g2_idx].pos[0].second];
                for (const pair<int, int>& cell : groups[best_g2_idx].pos) free_at[cell.first][cell.second] = -1;
                if (old_r2 != -1) update_room_biggest_pocket(old_r2, n, s);

                int new_r1 = orig_room_id[best_g1_new_region[0].first][best_g1_new_region[0].second];
                for (const pair<int, int>& cell : best_g1_new_region) free_at[cell.first][cell.second] = groups[best_g1_idx].t;
                if (new_r1 != -1) update_room_biggest_pocket(new_r1, n, s);

                int new_r2 = orig_room_id[best_g2_new_region[0].first][best_g2_new_region[0].second];
                for (const pair<int, int>& cell : best_g2_new_region) free_at[cell.first][cell.second] = groups[best_g2_idx].t;
                if (new_r2 != -1) update_room_biggest_pocket(new_r2, n, s);

                groups[best_g1_idx].c = best_g1_c;
                groups[best_g1_idx].pos = move(best_g1_new_region);

                groups[best_g2_idx].c = best_g2_c;
                groups[best_g2_idx].pos = move(best_g2_new_region);

                // Output Yes for incoming group X
                cout << "Yes\n";
                int new_x_rid = orig_room_id[best_x_new_region[0].first][best_x_new_region[0].second];
                for (const pair<int, int>& cell : best_x_new_region) {
                    cout << cell.first << " " << cell.second << "\n";
                    free_at[cell.first][cell.second] = t;
                }
                if (new_x_rid != -1) update_room_biggest_pocket(new_x_rid, n, s);

                groups[i].c = best_x_c;
                groups[i].pos = move(best_x_new_region);
            } else if (best_g_idx >= 0) {
                // Output 1 move for group best_g_idx
                cout << 1 << "\n";
                cout << best_g_idx << "\n";
                for (const pair<int, int>& cell : best_g_new_region) {
                    cout << cell.first << " " << cell.second << "\n";
                }

                // Update free_at and room allocation for G's new region
                int old_rid = orig_room_id[groups[best_g_idx].pos[0].first][groups[best_g_idx].pos[0].second];
                for (const pair<int, int>& cell : groups[best_g_idx].pos) {
                    free_at[cell.first][cell.second] = -1;
                }
                if (old_rid != -1) update_room_biggest_pocket(old_rid, n, s);

                int new_g_rid = orig_room_id[best_g_new_region[0].first][best_g_new_region[0].second];
                for (const pair<int, int>& cell : best_g_new_region) {
                    free_at[cell.first][cell.second] = groups[best_g_idx].t;
                }
                if (new_g_rid != -1) update_room_biggest_pocket(new_g_rid, n, s);

                groups[best_g_idx].c = best_g_c;
                groups[best_g_idx].pos = move(best_g_new_region);

                // Output Yes for incoming group X
                cout << "Yes\n";
                int new_x_rid = orig_room_id[best_x_new_region[0].first][best_x_new_region[0].second];
                for (const pair<int, int>& cell : best_x_new_region) {
                    cout << cell.first << " " << cell.second << "\n";
                    free_at[cell.first][cell.second] = t;
                }
                if (new_x_rid != -1) update_room_biggest_pocket(new_x_rid, n, s);

                groups[i].c = best_x_c;
                groups[i].pos = move(best_x_new_region);
            } else if (!normal_region.empty()) {
                // Output 0 moves and normal placement
                cout << 0 << "\n";
                int rid = orig_room_id[normal_region[0].first][normal_region[0].second];

                for (const pair<int, int>& cell : normal_region) {
                    free_at[cell.first][cell.second] = t;
                }
                if (rid != -1) update_room_biggest_pocket(rid, n, s);

                cout << "Yes\n";
                for (const pair<int, int>& cell : normal_region) {
                    cout << cell.first << " " << cell.second << "\n";
                }
                groups[i].c = chosen_c;
                groups[i].pos = move(normal_region);
            } else {
                cout << 0 << "\n";
                cout << "No\n";
            }
        cout.flush();
    }

    return 0;
}
