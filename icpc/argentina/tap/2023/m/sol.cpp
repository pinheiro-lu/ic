#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	int n, t; cin >> n >> t;
	priority_queue<double> pri, n_pri;
	for (int i = 0; i < n; ++i) {
		char c; cin >> c;
		int sz; cin >> sz;
		if (c == 'P') {
			pri.push(-sz);
		} else {
			n_pri.push(-sz);
		}
	}

	double time = 0;
	double sub_p = 0, sub_n = 0; // size i already downloaded
	double sp_p = t*3/4.0;
	while (!pri.empty() && !n_pri.empty()) {
		double sec = min(pri.size()*(-pri.top()-sub_p)/sp_p,
			n_pri.size()*(-n_pri.top()-sub_n)/(t-sp_p));
		time += sec;
		sub_p += sec*sp_p/pri.size();
		sub_n += sec*(t-sp_p)/n_pri.size();
		if (-pri.top() <= sub_p) pri.pop();
		if (-n_pri.top() <= sub_n) n_pri.pop();
		//cerr << time << ' ' << pri.size() << ' ' << n_pri.size() << endl;
		//cerr << -n_pri.top() << ' ' << sub_n << endl;
	}
	while (!pri.empty()) {
		double sec = pri.size()*(-pri.top()-sub_p)/t;
		time += sec;
		sub_p += sec*t/pri.size();
		if (-pri.top() <= sub_p) pri.pop();
	}
	while (!n_pri.empty()) {
		double sec = n_pri.size()*(-n_pri.top()-sub_n)/t;
		time += sec;
		sub_n += sec*t/n_pri.size();
		if (-n_pri.top() <= sub_n) n_pri.pop();
	}
	cout << time << '\n';

	return 0;
}
