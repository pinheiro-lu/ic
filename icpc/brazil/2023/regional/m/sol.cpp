#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

// Dijkstra
//
// encontra menor distancia de x
// para todos os vertices
// se ao final do algoritmo d[i] = LINF,
// entao x nao alcanca i
//
// O(m log(n))

struct dist {
	ll a;
	bool b;

	dist(): a(0),b(false) {}
	dist(ll a, bool b) : a(a), b(b) {}

	dist operator+(dist rhs) { return dist{a+rhs.a, b||rhs.b}; }
	dist operator-() { return dist{-a, b}; }

	bool operator<(const dist& rhs) const { 
		return a != rhs.a ? a < rhs.a : b < rhs.b;
	}
	bool operator>(const dist& rhs) const { 
		return a != rhs.a ? a > rhs.a : b > rhs.b;
	}
};
const int N = 1e5+2;
const ll LINF = 1e15;
dist d[N];
array<vector<pair<int, dist>>, N> graph; // {vizinho, peso}

int n;

void dijkstra(int v) {
	for (int i = 0; i < n; i++) d[i] = {LINF, false};
	d[v] = {0, false};
	priority_queue<pair<dist, int>> pq;
	pq.push({{0, false}, v});

	while (pq.size()) {
		auto [ndist, u] = pq.top(); pq.pop();
		if (-ndist > d[u]) continue;

		for (auto [idx, w] : graph[u]) if (d[idx] > d[u] + w) {
			d[idx] = d[u] + w;
			//cerr <<"u " << u << ' ' << idx << ": " << d[u].b << ' ' << w.b << endl;
			//cerr <<"u " << u << ' ' << idx << ": " << d[u].a << ' ' << w.a << endl;
			pq.push({-d[idx], idx});
		}
	}
}
int m, p, g;

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	cin >> n >> m >> p >> g;
	--p, --g;
	while (m--) {
		int u, v, d;
		cin >> u >> v >> d;
		--u,--v;
		graph[u].pb({v, {d,g==v || g==u}});
		graph[v].pb({u, {d,g==u || g==v}});
	}
	dijkstra(p);
	vi ans;
	for (int i = 0; i < n; ++i) {
		//cerr << i << ": " << d[i].a << ' ' << d[i].b << endl;
		if (d[i].a == d[g].a*2 && d[i].b) {
			ans.pb(i+1);
			cout << ans.back() << ' ';
		}
	}
	if (ans.empty()) {
		cout << "*\n";
	}  

	return 0;
}
