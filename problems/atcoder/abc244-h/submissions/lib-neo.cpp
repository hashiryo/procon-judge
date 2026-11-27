// https://atcoder.jp/contests/abc244/tasks/abc244_h
// NeoLibrary の LiChaoTree で、点の集合との内積 X A + Y B の最大を問う。
// 問いを B の符号で 2 つに分け、x = A / B の昇順に並べた順位を座標にする。点 (X, Y) は
// F(r) = X A_r + Y B_r の関数として木に載せる。LiChao 木の要る「どの 2 つも高々 1 回しか交わらない」は、
// A_r / B_r が順位について単調なので成り立つ。B = 0 の問いは X の最大と最小で答える。
#include <algorithm>
#include <cstdio>
#include <limits>
#include <vector>
#include "neo/algo/optimize/line/LiChaoTree.hpp"
using namespace std;
using i64 = long long;
using i128 = __int128;

template <bool MAX> struct Fn {
  i64 x, y;
  const i64 *as, *bs;
  int m;
  bool valid;
  i64 operator()(long long r) const {
    const int i = r < m ? (int)r : m - 1;
    return valid ? x * as[i] + y * bs[i] : (MAX ? numeric_limits<i64>::lowest() : numeric_limits<i64>::max());
  }
  static Fn none() { return Fn{0, 0, nullptr, nullptr, 0, false}; }
};

int main() {
  int Q;
  if (scanf("%d", &Q) != 1) return 0;
  vector<i64> X(Q), Y(Q), A(Q), B(Q);
  for (int i = 0; i < Q; ++i) {
    if (scanf("%lld %lld %lld %lld", &X[i], &Y[i], &A[i], &B[i]) != 4) return 0;
  }
  vector<int> grp[2];
  for (int i = 0; i < Q; ++i) {
    if (B[i] > 0) grp[0].push_back(i);
    else if (B[i] < 0) grp[1].push_back(i);
  }
  vector<i64> as[2], bs[2];
  vector<int> rank[2] = {vector<int>(Q, -1), vector<int>(Q, -1)};
  for (int t = 0; t < 2; ++t) {
    auto &v = grp[t];
    sort(v.begin(), v.end(), [&](int p, int q) {
      i128 np = A[p], dp = B[p], nq = A[q], dq = B[q];
      if (dp < 0) np = -np, dp = -dp;
      if (dq < 0) nq = -nq, dq = -dq;
      return np * dq < nq * dp;
    });
    as[t].resize(v.size()), bs[t].resize(v.size());
    for (size_t r = 0; r < v.size(); ++r) as[t][r] = A[v[r]], bs[t][r] = B[v[r]], rank[t][v[r]] = (int)r;
  }
  using Tree = LiChaoTree<i64, true, Fn<true>>;
  Tree *tree[2] = {nullptr, nullptr};
  for (int t = 0; t < 2; ++t) {
    if (!grp[t].empty()) tree[t] = new Tree(0, (long long)grp[t].size());
  }
  i64 maxX = numeric_limits<i64>::lowest(), minX = numeric_limits<i64>::max();
  for (int i = 0; i < Q; ++i) {
    maxX = max(maxX, X[i]), minX = min(minX, X[i]);
    for (int t = 0; t < 2; ++t) {
      if (tree[t]) tree[t]->add(Fn<true>{X[i], Y[i], as[t].data(), bs[t].data(), (int)grp[t].size(), true}, 0, (long long)grp[t].size());
    }
    i64 ans;
    if (B[i] > 0) ans = tree[0]->query(rank[0][i]);
    else if (B[i] < 0) ans = tree[1]->query(rank[1][i]);
    else ans = A[i] > 0 ? maxX * A[i] : A[i] < 0 ? minX * A[i] : 0;
    printf("%lld\n", ans);
  }
  return 0;
}
