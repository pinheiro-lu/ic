#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

const int MOD = 1e9+7;

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	int acc = 0, ans = 0;
	string s; cin >> s;
	for (char c : s) {
		if (c == 'a') {
			++acc;
		} else {
			ans = (ans+acc) % MOD;
		}
	}
	cout << ans << '\n';

	return 0;
}
