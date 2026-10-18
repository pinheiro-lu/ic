#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

const int N = 502;
array<array<bool, N>, N> vis;
array<map<int, int>, N * N> adj; // x*N+y : {adj.x*N+adj.y,w}

int n, m, m2;

bool dfs(int x, int y) {
	if (vis[x][y]) return false;
	vis[x][y] = true;
	if (x == n + 1) return true;
	for (auto &[p, w] : adj[x * N + y]) {
		if (w == 0) continue;
		if (dfs(p / N, p % N)) {
			w = 0;
			adj[p][x * N + y] = 1;
			return true;
		}
	}
	return false;
}

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	cin >> n >> m >> m2;
	if (m != m2) {
		cout << "-1\n";
		return 0;
	}

	set<pair<int, int>> in_edges;
	vector<pair<int, int>> out_edges;

	for (int i = 0; i < m; ++i) {
		int a, b; cin >> a >> b;
		if (a > b) swap(a, b);
		in_edges.insert({a, b});
	}

	for (int i = 0; i < m; ++i) {
		int a, b; cin >> a >> b;
		if (a > b) swap(a, b);
		if (!in_edges.erase({a, b})) {
			out_edges.pb({a, b});
		}
	}

	int ans = 0;
	int sink = (n + 1) * N + n + 1;

	// Connect in_edges from Source (0, 0) and directly to matching out_edges
	for (auto [a, b] : in_edges) {
		int in_node = a * N + b;
		adj[0][in_node] = 1;
		adj[in_node][0] = 0;
		ans += 2;

		for (auto [c, d] : out_edges) {
			if (a == c || a == d || b == c || b == d) {
				int out_node = c * N + d;
				adj[in_node][out_node] = 1;
				adj[out_node][in_node] = 0;
			}
		}
	}

	// Connect out_edges to Sink (n+1, n+1)
	for (auto [c, d] : out_edges) {
		int out_node = c * N + d;
		adj[out_node][sink] = 1;
		adj[sink][out_node] = 0;
	}

	int cnt = 0;
	while (dfs(0, 0)) {
		for (int i = 0; i <= n + 1; ++i) {
			vis[i].fill(false);
		}
		++cnt;
	}

	cout << ans - cnt << '\n';

	return 0;
}
