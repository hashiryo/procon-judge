#pragma once
// cf-1728-f の Kuhn 法の提出が共有する、候補の値と辺の組み立てと、値を小さい順に足す貪欲法。
//
// 順番を決めて言う値の和の最小は、漁師 i に a_i の倍数を別々に割り当てたときの和の最小に等しい (割り当てた値の小さい順に
// 言えば、言う値は割り当てた値以下になる)。漁師 i に要る倍数は a_i × k (k ≤ n) だけ (ほかの n - 1 人は高々 n - 1 個しか
// 使わない)。そこで候補の値 a_i × k を小さい順に並べ、同じ値をまとめて左の頂点、漁師を右の頂点にした二部グラフで、値の
// 小さい順に増加路を探し、見つかった値を採る (横断マトロイドの貪欲法)。漁師が全員埋まったら止める。
//
// 候補の値 (2^30 未満) と漁師の番号 (2^10 未満) を 1 つの 40 bit の鍵にし、14 bit ずつ 3 回の基数ソートで並べる。
// Match は _shared/kuhn/inc_matching.hpp の IncMatching のどれかで、提出が決めてから include する。
#include <array>
#include <vector>

struct Solver {
  int n;
  const vector<long long> &a;
  Solver(int n, const vector<long long> &a) : n(n), a(a) {}
  long long run() {
    const size_t N = (size_t)n * n;
    vector<u64> key(N), tmp(N);
    for (int i = 0, p = 0; i < n; ++i)
      for (long long k = 1; k <= n; ++k) key[p++] = (u64)(a[i] * k) << 10 | i;
    for (int sh = 0; sh < 42; sh += 14) {
      vector<unsigned> cnt((1 << 14) + 1, 0);
      for (u64 x : key) ++cnt[(x >> sh & 16383) + 1];
      for (int b = 0; b < (1 << 14); ++b) cnt[b + 1] += cnt[b];
      for (u64 x : key) tmp[cnt[x >> sh & 16383]++] = x;
      key.swap(tmp);
    }
    vector<long long> val;
    vector<array<int, 2>> edges(N);
    for (size_t j = 0; j < N; ++j) {
      const long long v = key[j] >> 10;
      if (val.empty() || val.back() != v) val.push_back(v);
      edges[j] = {int(val.size() - 1), int(key[j] & 1023)};
    }
    Match m(val.size(), n, edges);
    long long sum = 0;
    for (int v = 0; v < (int)val.size() && m.size() < n; ++v)
      if (m.augment(v)) sum += val[v];
    return sum;
  }
};
