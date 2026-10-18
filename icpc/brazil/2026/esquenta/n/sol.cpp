#include <bits/stdc++.h>

using namespace std;

typedef long long ll;
typedef vector<int> vi;

#define f first
#define s second
#define pb push_back

// 0-1 Knapsack
// Description: Solves 0-1 Knapsack using bottom-up DP.
// Finds the maximum value that fits into a knapsack of capacity W.
// Complexity: Time O(n * W), Space O(n * W)
// Source: https://github.com/stevenhalim/cpbook-code/blob/master/ch3/dp/UVa10130.cpp

const int N = 1002, MAXW = 10002;

int n;
array<int, N> v; // values
array<int, N> w; // weights
array<array<int, MAXW>, N> memo; // N = max items, MAXW = max capacity

int knapsack(int w_cap) {
	for (int i = 1; i <= n; i++) {
		for (int j = 0; j <= w_cap; j++) {
			if (w[i - 1] > j) {
				memo[i][j] = memo[i - 1][j];
			} else {
				memo[i][j] = max(memo[i - 1][j], v[i - 1] + memo[i - 1][j - w[i - 1]]);
			}
		}
	}
	return memo[n][w_cap];
}

signed main() {
	ios_base::sync_with_stdio(0); cin.tie(0);
	int W;
	cin >> n >> W;
	for (int i = 0; i < n; ++i) {
		cin >> w[i] >> v[i];
	}

	cout << knapsack(W) << '\n';

	return 0;
}
