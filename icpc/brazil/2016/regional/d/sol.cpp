#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

int n;
const int N = 1e5+2;
array<int, N> s;
deque<int> q;

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);

	while (cin >> n) {
		q.clear();
		for (int i = 0; i < n; ++i) {
			cin >> s[i];
		}
		sort(s.begin(), s.begin()+n);
		for (int i = 0; i < n; ++i) {
			if (i % 2 == 0) q.pb(s[i]);
			else q.push_front(s[i]);
		}
		q.pb(q.front());

		double ans = 0;
		for (int i = 1; i <= n; ++i) {
			ans += q[i] * q[i-1] * sin(2*M_PI/n);
		}
		cout << fixed << setprecision(3) << ans/2 << '\n';
	}

	return 0;
}
