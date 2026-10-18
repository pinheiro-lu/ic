#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

int n;

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);

	while(cin >> n) {
		bool prime = false;
		while (!prime) {
			prime = true;
			for (int x = 2; x*x <= n; ++x) {
				if (n % x == 0) {
					prime = false;
					break;
				}
			}
			if (prime) break;
			--n;
		}
		cout << n << '\n';
	}

	return 0;
}
