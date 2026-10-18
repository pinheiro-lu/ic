#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second

array<int, 101> cnt;

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	int n;
	cin >> n;

	for (int i = 0; i < n; ++i) {
		int x;
		cin >> x;
		cnt[x]++;
	}

	cout << n-*max_element(cnt.begin(), cnt.end()) << '\n';

	return 0;
}
