// anti monge — NeoLibrary の LiChaoTree に、関数 dp[j] + D[j][i] を載せる (lib.cpp と同じ解き方)。
// 関数の型は、木が範囲の外を評価しても添字が溢れないように N で止める。
#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>
#include "neo/algo/optimize/line/LiChaoTree.hpp"
using namespace std;
using i64 = long long;

template <bool MAX> struct Fn {
  int j, n;
  const vector<int> *row;
  const i64 *dp;
  bool valid;
  i64 operator()(long long x) const {
    const int i = x > n ? n : (int)x;
    return valid ? dp[j] + (*row)[i] : (MAX ? numeric_limits<i64>::lowest() : numeric_limits<i64>::max());
  }
  static Fn none() { return Fn{0, 0, nullptr, nullptr, false}; }
};

signed main() {
  cin.tie(0);
  ios::sync_with_stdio(false);
  int N;
  cin >> N;
  int P[N];
  for (int i = 0; i < N; ++i) cin >> P[i];
  static constexpr int INF = 1e8;
  vector D(N, vector(N + 1, INF));
  for (int i = 0; i < N - 1; ++i) {
    int R[N + 1];
    fill_n(R, N + 1, 0);
    for (int j = 0; j <= i; ++j) ++R[P[j]];
    for (int j = 0; j < N; ++j) R[j + 1] += R[j];
    int sum = 0;
    for (int j = i + 1; j < N; ++j) D[i + 1][j + 1] = sum += R[N] - R[P[j]];
  }
  vector<i64> dp(N + 1, 0);
  cout << 0 << '\n';
  for (int k = 1; k < N; ++k) {
    vector<i64> ndp(N + 1, INF);
    LiChaoTree<i64, false, Fn<false>> tree(k, N + 1);
    for (int i = k; i <= N; ++i) {
      ndp[i] = i == k ? 0 : tree.query(i);
      if (i < N) tree.add(Fn<false>{i, N, &D[i], dp.data(), true}, i + 1, N + 1);
    }
    copy(ndp.begin() + k, ndp.end(), dp.begin() + k);
    cout << dp[N] << '\n';
  }
  return 0;
}
