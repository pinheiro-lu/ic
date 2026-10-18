#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

array<int, 5> cnt;

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	int n; cin >> n;

	for (int i = 0; i < n; ++i) {
		int x; cin >> x;
		cnt[x%5]++;
	}
	int ans = cnt[0];
	for (int i0 = 0; i0 < 5; ++i0) {
	for (int i1 = i0; i1 < 5; ++i1) {
	for (int i2 = i1; i2 < 5; ++i2) {
	for (int i3 = i2; i3 < 5; ++i3) {
	for (int i4 = i3; i4 < 5; ++i4) {
		if (!i4 || (i0 + i1 + i2 + i3 + i4) % 5) continue;
		cerr << i0 << ' ';
		cerr << i1 << ' ';
		cerr << i2 << ' ';
		cerr << i3 << ' ';
		cerr << i4 << ' ';
		cerr << endl;
		vi v(5);
		++v[i0];
		++v[i1];
		++v[i2];
		++v[i3];
		++v[i4];
		int mn = n;
		for (int i = 1; i < 5; ++i) {
			if (!v[i]) continue;
			mn = min(mn, cnt[i]/v[i]);
		}
		cerr << "adding " << mn << endl;
		ans += mn;
		for (int i = 1; i < 5; ++i) {
			cnt[i] -= v[i]*mn;
		}
	}}}}}
	cout << ans << '\n';

	return 0;
}
