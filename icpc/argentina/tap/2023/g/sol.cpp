#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

int n;

const int N = 1e5+2;
array<pair<int,int>, 2*N> ev_ccw, ev_cw; // {norm_pos,idx}
array<int, N> h;

// Kruskal
//
// Gera e retorna uma AGM e seu custo total a partir do vetor de arestas (edg)
// do grafo
//
// O(m log(m) + m a(m))
// 864875

vector<tuple<int, int, int>> edg; // {peso,[x,y]}

// DSU em O(a(n))
void dsu(int n);
int find(int a);
void unite(int a, int b);

// DSU
//
// Une dois conjuntos e acha a qual conjunto um elemento pertence por seu id
//
// find e unite: O(a(n)) ~= O(1) amortizado

vector<int> id, sz;

void dsu(int n) {
	id.resize(n);
	sz.assign(n, 1); 
	iota(id.begin(), id.end(), 0);
}

int find(int a) { return a == id[a] ? a : id[a] = find(id[a]); }

void unite(int a, int b) {
	a = find(a), b = find(b);
	if (a == b) return;
	if (sz[a] < sz[b]) swap(a, b);
	sz[a] += sz[b], id[b] = a;
}

ll kruskal(int n) {
	dsu(n);
	sort(edg.begin(), edg.end());
	
	ll cost = 0;
	for (auto [w,x,y] : edg) if (find(x) != find(y)) {
		cost += w;
		unite(x,y);
	}
	return cost;
}

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);

	cin >> n;
	edg.reserve(n*4);
	for (int i = 1; i <= n; ++i) {
		int x, l, r;
		cin >> x >> l >> r;
		h[i] = x;
		ev_cw[2*i-2] = {l+x,-i};
		ev_cw[2*i-1] = {r+x,i};
		ev_ccw[2*i-2] = {l-x,-i};
		ev_ccw[2*i-1] = {r-x,i};
	}
	sort(ev_ccw.begin(), ev_ccw.begin()+2*n);
	sort(ev_cw.begin(), ev_cw.begin()+2*n);
	set<pair<int,int>> cur; // {h,idx} in cur interval
	cur.insert({0, 0});
	auto create_edges = [&] (auto &ev) {
		for (int i = 0; i < 2*n; ++i) {
			auto [pos, idx] = ev[i];
			cerr << pos << ' ' << idx << ' ' << h[idx] << endl;
			if (idx > 0) {
				cur.erase({h[idx],idx});
				continue;
			} 
			idx = -idx;
			auto it = cur.upper_bound(make_pair(h[idx],0));
			if (it != cur.end()) {
				auto [ub, kdx] = *(it);
				edg.pb({ub-h[idx],idx,kdx});
				//cerr << "adding {" << ub-h[idx] << "," << h[idx] << ',' << h[kdx] << "}" << endl;
			}
			auto [lb, jdx] = *(--it);
			edg.pb({h[idx]-lb,idx,jdx});
			//cerr << "adding {" << h[idx]-lb << "," << h[idx] << ',' << h[jdx] << "}" << endl;
			cur.insert({h[idx],idx});
		}
	};
	create_edges(ev_ccw);
	create_edges(ev_cw);
	cout << kruskal(n+1) << '\n';

	return 0;
}
