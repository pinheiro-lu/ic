#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

const int N = 502;
array<array<bool, N>, N> g_in,g_out, vis;
array<map<int,int>, N*N> adj; // x*N+y : {adj.x*N+adj.y,w}

int n, m, m2;

bool dfs(int x, int y) {
	if (vis[x][y]) return false;
	vis[x][y] = true;
	if (x == n+1) return true;
	for (auto &[p,w] : adj[x*N+y]) {
		//cerr << x << ',' << y << " to " << p/N << ',' << p%N << endl;
		if (w == 0) continue;
		//cerr << "will try" << endl;
		if (dfs(p/N,p%N)) {
			w = 0;
			//cerr << adj[x*N+y][p] << endl;
			adj[p][x*N+y] = 1;
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

	for (int i = 0; i < m; ++i) {
		int a,b; cin >> a >> b;
		if (a > b) swap(a,b);
		g_in[a][b] = true;
	}

	for (int i = 0; i < m; ++i) {
		int a,b; cin >> a >> b;
		if (a > b) swap(a,b);
		if (g_in[a][b]) {
			g_in[a][b] = false;
		} else {
			g_out[a][b] = true;
		}
	}

	int ans = 0;
	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= n; ++j) {
			if (g_in[i][j]) {
				adj[0][i*N+j] = 1;
				adj[i*N+j][0] = 0;
				ans += 2;
			}
			if (g_out[i][j]) {
				adj[i*N+j][(n+1)*N+n+1] = 1;
				adj[(n+1)*N+n+1][i*N+j] = 0;
				for (int x = 1; x <= n; ++x) {
					if (g_in[x][j]) {
						adj[x*N+j][i*N+j] = 1;
						adj[i*N+j][x*N+j] = 0;
					}
					if (g_in[j][x]) {
						adj[j*N+x][i*N+j] = 1;
						adj[i*N+j][j*N+n] = 0;
					}
				}
				for (int y = 1; y <= n; ++y) {
					if (g_in[i][y]) {
						adj[i*N+y][i*N+j] = 1;
						adj[i*N+j][i*N+y] = 0;
					}
					if (g_in[y][i]) {
						adj[y*N+i][i*N+j] = 1;
						adj[i*N+j][y*N+i] = 0;
					}
				}
			}
		}
	}

	int cnt = 0;
	while(dfs(0,0)) {
		for (int i = 0; i <= n+1; ++i) {
			vis[i].fill(false);
		}
		++cnt;
	}

	cout << ans - cnt << '\n';

	for (int i = 0; i <= n+1; i++) {
		for (int j = 0; j <= n+1; j++) {
			if (!adj[i*N+j].empty()) {
				//cerr << "[" << i << "," << j << "]: ";
				for (auto [p,w] : adj[i*N+j]) {
					//cerr << "{(" << w << ")," << p/N << "," << p%N << "} ";
				}
				//cerr << endl;
			}
		}
	}

	return 0;
}
