#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

ll sum;

/**
 * Author: Simon Lindholm
 * Date: 2016-10-08
 * License: CC0
 * Source: https://github.com/kth-competitive-programming/kactl/blob/main/content/data-structures/LazySegmentTree.h
 * Description: Segment tree with ability to add or set values of large intervals, and compute max of intervals.
 * Can be changed to other things.
 * Use with a bump allocator for better performance, and SmallPtr or implicit indices to save memory.
 * Time: O(\log N).
 * Usage: Node* tr = new Node(v, 0, v.size());
 * Intervals are half-open: [L, R)
 * Status: stress-tested a bit
 */

const int inf = 1e9 + 2;

struct Node {
	Node *l = 0, *r = 0;
	int lo, hi, madd = 0, mset = inf;
	ll val = 0, mx;
	//val=(mx-val)*flip+val*~flip
	Node(int lo, int hi) : lo(lo), hi(hi) {} // Large interval of -inf
	//int query(int L, int R) {
	//	if (R <= lo || hi <= L) return 0;
	//	if (L <= lo && hi <= R) return val;
	//	push();
	//	return l->query(L, R) + r->query(L, R);
	//}
	void set(int L, int R, int x) {
		if (R <= lo || hi <= L) return;
		if (L <= lo && hi <= R) mset = mx = x;
		else {
			push(), l->set(L, R, x), r->set(L, R, x);
			mx = l->mx + r->mx;
		}
	}
	void add(int L, int R, bool upd = false) {
		if (R <= lo || hi <= L) return;
		if (L <= lo && hi <= R) {
			if (!upd) sum -= val;
			madd = ~madd;
			val = mx-val;
			if (!upd) sum += val;
		} else {
			push(), l->add(L, R), r->add(L, R);
			val = l->val + r->val;
		}
	}
	void push() {
		if (!l) {
			int mid = lo + (hi - lo) / 2;
			l = new Node(lo, mid); r = new Node(mid, hi);
		}
		if (mset != inf)
			l->set(lo,hi,mset), r->set(lo,hi,mset), mset = inf;
		else if (madd)
			l->add(lo, hi, true), r->add(lo, hi, true), madd = 0;
	}
};
/**
 * Author: Benjamin Qi, Oleksandr Kulkov, chilli
 * Date: 2020-01-12
 * License: CC0
 * Source: https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/HLD.h
 * Description: Decomposes a tree into vertex disjoint heavy paths and light
 * edges such that the path from any leaf to the root contains at most log(n)
 * light edges. Code does additive modifications and max queries, but can
 * support commutative segtree modifications/queries on paths and subtrees.
 * Takes as input the full adjacency list. VALS_EDGES being true means that
 * values are stored in the edges, as opposed to the nodes. All values
 * initialized to the segtree default. Root must be 0.
 * Time: O((log N)^2)
 * Status: stress-tested against old HLD
 */

template <bool VALS_EDGES = false> struct HLD {
	int N, tim = 0;
	vector<vi> adj;
	vi par, siz, rt, pos;
	Node *tree;

	HLD(vector<vi> adj_)
		: N(adj_.size()), adj(adj_), par(N, -1), siz(N, 1),
		  rt(N), pos(N), tree(new Node(0, N)) {
		dfsSz(0); dfsHld(0);
	}
	void dfsSz(int v) {
		if (par[v] != -1) adj[v].erase(find(adj[v].begin(), adj[v].end(), par[v]));
		for (int& u : adj[v]) {
			par[u] = v;
			dfsSz(u);
			siz[v] += siz[u];
			if (siz[u] > siz[adj[v][0]]) swap(u, adj[v][0]);
		}
	}
	void dfsHld(int v) {
		pos[v] = tim++;
		for (int u : adj[v]) {
			rt[u] = (u == adj[v][0] ? rt[v] : u);
			dfsHld(u);
		}
	}
	template <class B> void process(int u, int v, B op) {
		for (; rt[u] != rt[v]; v = par[rt[v]]) {
			if (pos[rt[u]] > pos[rt[v]]) swap(u, v);
			op(pos[rt[v]], pos[v] + 1);
		}
		if (pos[u] > pos[v]) swap(u, v);
		op(pos[u] + VALS_EDGES, pos[v] + 1);
	}
	void modifyPath(int u, int v) {
		process(u, v, [&](int l, int r) { tree->add(l, r); });
	}
	void setPath(int u, int v, int x) {
		process(u, v, [&](int l, int r) { tree->set(l, r, x); });
	}
	//int queryPath(int u, int v) {
	//	int res = -inf;
	//	process(u, v, [&](int l, int r) {
	//		res = max(res, tree->query(l, r));
	//	});
	//	return res;
	//}
	//int querySubtree(int v) {
	//	return tree->query(pos[v] + VALS_EDGES, pos[v] + siz[v]);
	//}
};

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	int n, q; cin >> n >> q;

	vector<vi> g(n);
	map<pair<int, int>, int> mp; // [u, v] = w

	for (int i = 1; i < n; ++i) {
		int u, v, w;
		cin >> u >> v >> w;
		--u, --v;
		g[u].pb(v);
		g[v].pb(u);
		mp[{u,v}] = w;
	}
	
	HLD<true> hld(g);

	for (auto &[edge, w] : mp) {
		auto &[u, v] = edge;
		hld.setPath(u,v,w);
	}

	while (q--) {
		int u, v;
		cin >> u >> v;
		--u, --v;
		hld.modifyPath(u,v);
		cout << sum << '\n';
	}

	return 0;
}
