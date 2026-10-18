#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

const int N = 2e5+2;
array<int, N> t;

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	int n;
	cin >> n;
	
	for (int i = 0; i < n; ++i) {
		cin >> t[i];
	}
	sort(t.begin(), t.begin()+n);
	ll ans = 0;
	ll cur = 0;
	for (int i = 0; i < n; ++i) {
		cur += t[i];
		ans += cur;
	}
	cout << ans << '\n';

	return 0;
}
