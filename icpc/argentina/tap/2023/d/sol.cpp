#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

const int K = 1e5+2;
array<ll, K> c;

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	int k; cin >> k;
	for (int i = 0; i < k; ++i) {
		cin >> c[i];
	}
	partial_sum(c.begin(), c.begin()+k, c.begin());
	ll acc = 0;
	int ans = 1e9;
	for (int i = 0; i < k; ++i) {
		int x; cin >> x;
		acc += x;
		ans = min((ll)ans, acc/c[i]);
	}
	cout << ans << '\n';

	return 0;
}
