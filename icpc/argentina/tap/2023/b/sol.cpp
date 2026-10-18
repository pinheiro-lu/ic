#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	int n; cin >> n;

	int ans = 0;
	for (int i = 0; i < n; ++i) {
		int cur = 0;
		char c;
		for (int j = 0; j < n; ++j) {
			cin >> c;
			if (c == 'N') {
				++cur;
			} else {
				ans += cur/2;
				cur = 0;
			}
		}
		ans += cur/2;
	}
	cout << ans << '\n';

	return 0;
}
