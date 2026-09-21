#pragma once
// ライブラリを使う提出が共有するもの。Li Chao 木で回す DP はどちらの実装でも同じ
// なので、ここに置く。比べたいのは A と B の各接尾辞との最長共通接頭辞の取り方。
#include <algorithm>
#include <utility>
#include "pj.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// Match は次を実装する。
//   // B の i 文字目から始まる接尾辞と A の最長共通接頭辞を、i ごとに返す (A の長さが上限)。
//   static vector<int> prefix_matches(const vector<int> &a, const vector<int> &b);
template <class Match> struct SequenceSolver {
  vector<int> a, b;
  vector<i64> c;
  i64 ans = 0;

  SequenceSolver(const vector<i64> &a, const vector<i64> &b, const vector<i64> &c)
      : a(a.begin(), a.end()), b(b.begin(), b.end()), c(c) {}

  void run() {
    const int m = (int)b.size();
    vector<int> z = Match::prefix_matches(a, b);
    // 位置 l から k 個足す費用 dp[l] + k * C_l を、x = l + k の直線 C_l x + (dp[l] - l C_l)
    // として x in [l + 1, l + z[l] + 1) に入れる。dp[x] はその最小。
    auto f = [](int x, i64 p, i64 q) { return p * x + q; };
    LiChaoTree lct(f, 0, (int)1e9 + 10);
    auto tree = lct.template make_tree<MINIMIZE>();
    i64 dp = 0;  // dp[i]。作れなければ -1
    for (int i = 0; i < m; ++i) {
      if (dp >= 0) tree.insert(c[i], dp - c[i] * i, i + 1, i + z[i] + 1);
      auto [v, id] = tree.query(i + 1);
      dp = id < 0 ? -1 : v;
    }
    ans = dp;
  }

  i64 answer() const { return ans; }
};
