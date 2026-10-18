#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

// Integer Geometry with std::complex
// Sources:
// - Antti Laaksonen (Competitive Programmer's Handbook, Ch. 29)
// - Victor Lecomte (Handbook of Geometry for Competitive Programmers)
// - Codeforces Blog 22175 (https://codeforces.com/blog/entry/22175)

using T = int;
using P = complex<T>;
#define X real()
#define Y imag()

namespace std {
	bool operator<(P a, P b) {
		return a.X != b.X ? a.X < b.X : a.Y < b.Y;
	}
}

// Dot & Cross products: conj(a) * b = (a . b) + i * (a x b)
T dot(P a, P b) { return (conj(a) * b).X; }
T cross(P a, P b) { return (conj(a) * b).Y; }

bool zero(T x) {
	return x == 0;
}

// angular comparison in [0, 2pi)
// smallest is (1, 0)
// CORNER: a || b == (0, 0)
bool ang_cmp(P a, P b) {
	auto quad = [](P p) -> bool {
		// 0 if ang in [0, pi), 1 if in [pi, 2pi)
		return p.Y < 0 || (p.Y == 0 && p.X < 0);
	};
	using tup = tuple<bool, T>;
	return tup{quad(a), 0} < tup{quad(b), cross(a, b)};
}

T dist2(P p, P q) { // squared distance
    return norm(p - q);
}

T area2(P a, P b, P c) { // two times signed area of triangle abc
	return cross(b - a, c - a);
}

bool left(P a, P b, P c) {
	return area2(a, b, c) > 0; // counterclockwise
}

bool right(P a, P b, P c) {
	return area2(a, b, c) < 0; // clockwise
}

bool collinear(P a, P b, P c) {
	return zero(area2(a, b, c));
}

// CORNER: a || b == (0, 0)
int parallel(P a, P b) {
	if(!zero(cross(a, b))) return 0;
	return (a.X > 0) == (b.X > 0) && (a.Y > 0) == (b.Y > 0) ? 1 : -1;
}

// CORNER: a == b
struct segment {
	P a, b;
	
	segment(): a(), b() {}
	segment(P _a, P _b): a(_a), b(_b) {}

	P vec() { return b - a; }

	bool contains(P p) {
		return a == p || b == p || parallel(a - p, b - p) == -1;
	}
};

bool intersects(segment r, segment s) {
	if(r.contains(s.a) || r.contains(s.b) || s.contains(r.a) || s.contains(r.b)) return 1;
	return left(r.a, r.b, s.a) != left(r.a, r.b, s.b) && 
		left(s.a, s.b, r.a) != left(s.a, s.b, r.b);
}

bool parallel(segment r, segment s) {
	return parallel(r.vec(), s.vec());
}

const int N = 1e3+2, F = 8+2;
unordered_map<string,int> dic_ind; // "word" : index
array<P, N> coord; // coord[index] = {x,y}
array<string, N> base, dic; // knowledge base, words of dictionary
array<string, F> query; // query text
vector<string> cand; // candidate words

int n, m, f;

bool find_cand(int k) {
	bool ans = false;
	for (int i = 0; i < m-k; ++i) {
		bool found = true;
		//cerr << "testing base[" << i << endl;
		//cerr << "k " << k << endl;
		for (int j = 0; j < k; ++j) {
			if (base[i+j] != query[j+f-k]) {
				found = false;
				break;
			}
			//cerr << "match: " << i << ' ' << j << endl;
		}
		if (found) {
			ans = true;
			cand.pb(base[i+k]); // CHECK
		}
	}
	return ans;
}


signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	cin >> n;

	for (int i = 0; i < n; ++i) {
		string p;
		int x, y;
		cin >> p >> x >> y;
		dic_ind[p] = i;
		dic[i] = p;
		coord[i] = P(x,y);
	}

	cin >> m;
	for (int i = 0; i < m; ++i) {
		cin >> base[i];
	}

	int q, k;
	cin >> q >> k;
	while (q--) {
		cin >> f;
		for (int i = 0; i < f; ++i) {
			cin >> query[i];
			cout << query[i] << ' ';
		}
		cand.clear();
		bool found = false;
		for (int kk = k; kk >= 1; --kk) {
			if (find_cand(kk)) {
				found = true;
				break;
			}
		}
		if (!found) {
			cout << "*\n";
			continue;
		}
		int mx = 0;
		int best = -1;
		for (int i = 0; i < n; ++i) {
			int score = 0;
			for (string c : cand) {
				score += dot(coord[i], coord[dic_ind[c]]);
			}
			if (score > mx) {
				mx = score;
				best = i;
			}
		}
		cout << dic[best] << '\n';
	}


	return 0;
}
