#pragma once
// _lyndon_lean.hpp が使う中身 (もとは、消した lyndon_ll と lyndon_ll_w8 が共有していた)。2 つの Lyndon 配列を接尾辞配列から
// 連結リストで求める関数と、文字を直接比べる予算を使い切ったあとに LCE と LCS を LCP 配列と NeoLibrary の RangeMin で引く道。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "neo/string/suffix_array.hpp"
#include "neo/data_structure/RangeMin.hpp"

namespace lyndon_hybrid {
// nxt0[i] と nxt1[i] は、i より後ろで順位が i より小さい (大きい) 最初の位置。無ければ n。位置を並べた双方向の連結リストから、
// 順位の大きい順 (小さい順) に位置を外していくと、外すときの次の位置が、順位が小さい (大きい) 最初の位置になる。2 つの順序の
// リストは別々に持ち、1 つのループの中で交互に外す (互いに依存しないので、飛び飛びの読み書きが重なる)。手元の M2 の
// max_random_00 では、FastSet で求める形の 1.68 ms に対して 1.16 ms だった。
inline void lyndon_ends_list(const std::vector<int> &sa, std::vector<int> &nxt0, std::vector<int> &nxt1) {
  const int n = int(sa.size());
  nxt0.resize(n), nxt1.resize(n);
  struct Node {
    int nx, pv;
  };
  std::vector<Node> l0(n + 2), l1(n + 2);  // 位置 x (-1 から n まで) の節は x + 1 番目
  for (int x = 0; x <= n + 1; ++x) l0[x] = l1[x] = {x, x - 2};
  for (int k = 0; k < n; ++k) {
    {
      const int i = sa[n - 1 - k];
      const Node d = l0[i + 1];
      nxt0[i] = d.nx;
      l0[d.pv + 1].nx = d.nx, l0[d.nx + 1].pv = d.pv;
    }
    {
      const int i = sa[k];
      const Node d = l1[i + 1];
      nxt1[i] = d.nx;
      l1[d.pv + 1].nx = d.nx, l1[d.nx + 1].pv = d.pv;
    }
  }
}
// 文字列の任意の 2 つの接尾辞の LCP を O(1) で引く。
struct Lce {
  std::vector<int> isa;
  RangeMin rm;
  Lce(const std::string &s, const std::vector<int> &sa) : isa(s.size()) {
    const int n = int(s.size());
    const std::vector<int> lcp = lcp_array(s, sa);
    for (int i = 0; i < n; ++i) isa[sa[i]] = i;
    rm = RangeMin(std::vector<unsigned>(lcp.begin(), lcp.end()));
  }
  int operator()(int i, int j) const {  // i != j、どちらも n 未満
    const auto [a, b] = std::minmax(isa[i], isa[j]);
    return int(rm.min(a, b));
  }
};
// 予算を使い切ったあとの道。初めて呼ばれたときに LCE の表を作る。根を伸ばす関数の中へ展開させないよう noinline にする。
struct Slow {
  const std::string &s;
  const std::vector<int> &sa;
  std::unique_ptr<Lce> fwd, bwd;
  [[gnu::noinline]] std::pair<int, int> operator()(int i, int j, int p) {
    const int n = int(s.size());
    if (!fwd) {
      const std::string r(s.rbegin(), s.rend());
      fwd = std::make_unique<Lce>(s, sa), bwd = std::make_unique<Lce>(r, suffix_array(r));
    }
    const int e = j < n ? (*fwd)(i, j) : 0, b = i > 0 ? (*bwd)(n - i, n - j) : 0;  // 反転した文字列で、n - i と n - j から始まる接尾辞
    return e + b >= p ? std::pair<int, int>(i - b, j + e) : std::pair<int, int>(-1, 0);
  }
};
}  // namespace lyndon_hybrid
