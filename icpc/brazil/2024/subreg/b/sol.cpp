#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

const int N = 102, M = 1e6+2;

array<array<int, N>, N> adj;
array<vi, M> mov;
array<bool, N> vis;

int n, m;
stack<int> ans;

bool dfs(int u, int v) { 
	if (vis[u]) return false;
	vis[u] = true;
	if (u == v) {
		ans.push(v);
		return true;
	}
	for (int i = 1; i <= n; ++i) {
		if (!adj[u][i]) continue;
		if (dfs(i,v)) {
			ans.push(adj[u][i]);
			ans.push(u);
			return true;
		}
	}
	return false;
}

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	cin >> n >> m;

	for (int i = 1; i <= n; ++i) {
		int ni; cin >> ni;
		while (ni--) {
			int x; cin >> x;
			for (int y : mov[x]) {
				adj[i][y] = adj[y][i] = x;
			}
			mov[x].pb(i);
		}
	}
	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= n; ++j) {
			//cerr << adj[i][j] << ' ';
		}
		//cerr << endl;
	}
	for (int i = 1; i <= m; ++i) {
		//cerr << i << ": ";
		for (int u : mov[i]) {
			//cerr << u << ' ';
		}
		//cerr << endl;
	}
	int q; cin >> q;
	while (q--) {
		int x, y;
		cin >> x >> y;
		for (int u : mov[x]) {
			for (int v : mov[y]) {
				vis.fill(false);
				if (dfs(u,v)) {
					cout << 2+ans.size()/2 << '\n';
					cout << x << ' ';
					while (!ans.empty()) {
						cout << ans.top() << ' ';
						ans.pop();
					}
					cout << y << '\n';
					goto skip;
				} 
			}
		}
		cout << "-1\n";
		skip:
	}

	return 0;
}
