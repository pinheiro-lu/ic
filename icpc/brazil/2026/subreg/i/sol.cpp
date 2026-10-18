#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

unordered_map<int, int> sz;
unordered_map<int, tuple<int,int,int>> groups; // key is the friend
unordered_map<int, vi> dep;

const int N = 1e5 + 2;
array<int, N> fri; // fri[i] is the key of the i+1-th invited friend

ll dfs(int f) {
	ll p = sz[f];
	for (int d : dep[f]) {
		p += dfs(d);
	}
	return p;
}

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	int n, _; cin >> _ >> n;
	for (int i = 0; i < n; ++i) {
		int k, p;
		cin >> k >> p;
		fri[i] = k;
		sz[k] = p;
	}

	for (int i = 0; i < n; ++i) {
		char ans;
		cin >> ans;
		if (ans == 'A') {
			int l, r; cin >> l >> r;
			r += l;
			groups[fri[i]] = {l, r, sz[fri[i]]};
		} if (ans == 'T') {
			int x; cin >> x;
			dep[x].pb(fri[i]);
		}
	}

	vector<pair<int, ll>> ev;
	for (auto [f, tp] : groups) {
		auto [l, r, p] = tp;
		ll pp = p;
		for (int d : dep[f]) {
			pp += dfs(d);
		}
		ev.pb({l, pp});
		ev.pb({r, -pp});
	}
	sort(ev.begin(), ev.end());
	ll cur = 0, mx = 0;
	for (auto[_, p] : ev) {
		cur += p;
		mx = max(mx, cur);
	}
	cout << mx << '\n';

	return 0;
}
