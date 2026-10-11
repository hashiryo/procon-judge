#pragma once
// lyndon_lean_hp2 で、2 つの Lyndon 配列を連結リストでなく FastSet で求める版。6 回目と 7 回目 (700cb890 の run 38128344191、
// EPYC 7763) の診断では、lyndon_lean_hp の連結リストの段が x64-gcc の max_random で 2.48 ms かかった (接尾辞配列は 4.50 ms、根を調べて
// 並べる段は 1.29 ms)。連結リストの 3.2 MB は 7763 の L2 (512 KB) に収まらず、飛び飛びの読み書きが L3 まで行く。FastSet の 2 つの
// 集合は合わせて 50 KB ほどで L2 に載る。4 回目には、連結リストの lyndon_ll が FastSet の lyndon_fs_hybrid2 より 2.5 ms 遅く、そのうち
// 初めて触るページの代価で説明できるのは 1.4 ms ほどだった。Lyndon 配列は作業領域 (huge page) の nxt0 と nxt1 に書き、根は
// lyndon_lean_hp2 と同じく位置の順に調べる。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"
#include "neo/data_structure/FastSet.hpp"
#include "neo/data_structure/RangeMin.hpp"
#include "_lyndon_lean.hpp"

namespace lyndon_fs_hp2 {
// 文字列の任意の 2 つの接尾辞の LCP を O(1) で引く。
struct Lce {
  vector<int> isa;
  RangeMin rm;
  Lce(const unsigned char *s, int n, const int *sa) : isa(n) {
    const vector<int> lcp = suffix_array_internal::lcp_phi(s, n, sa);
    for (int i = 0; i < n; ++i) isa[sa[i]] = i;
    rm = RangeMin(vector<unsigned>(lcp.begin(), lcp.end()));
  }
  int operator()(int i, int j) const {  // i != j、どちらも n 未満
    const auto [a, b] = minmax(isa[i], isa[j]);
    return int(rm.min(a, b));
  }
};
// 予算を使い切ったあとの道。初めて呼ばれたときに LCE の表を作る。
struct Slow {
  const unsigned char *s;
  int n;
  const int *sa;
  unique_ptr<Lce> fwd, bwd;
  [[gnu::noinline]] pair<int, int> operator()(int i, int j, int p) {
    if (!fwd) {
      const vector<unsigned char> rv(make_reverse_iterator(s + n), make_reverse_iterator(s));
      vector<int> rsa(n);
      suffix_array_internal::sa_is(rv.data(), n, 256, rsa.data());
      fwd = make_unique<Lce>(s, n, sa), bwd = make_unique<Lce>(rv.data(), n, rsa.data());
    }
    const int e = j < n ? (*fwd)(i, j) : 0, b = i > 0 ? (*bwd)(n - i, n - j) : 0;  // 反転した文字列で、n - i と n - j から始まる接尾辞
    return e + b >= p ? pair<int, int>(i - b, j + e) : pair<int, int>(-1, 0);
  }
};
inline vector<array<int, 3>> runs(const string &str) {
  using namespace lyndon_lean;
  const int n = int(str.size());
  if (n < 2) return {};
  auto up8 = [](size_t x) { return (x + 7) & ~size_t(7); };
  const size_t half = size_t(n) / 2 + 1;
  const size_t o_sa = 0, o_n0 = up8(size_t(n) * 4), o_n1 = o_n0 + up8(size_t(n) * 4), o_last = o_n1 + up8(size_t(n) * 4),
               o_t = o_last + up8(half * 4), o_found = o_t + up8(size_t(n) + 16), bytes = o_found + size_t(n) * 12;
  Buffer buf(bytes, bytes >= (size_t(1) << 20));
  char *w = buf.get();
  int *sa = reinterpret_cast<int *>(w + o_sa);
  int *nxt0 = reinterpret_cast<int *>(w + o_n0), *nxt1 = reinterpret_cast<int *>(w + o_n1);
  int *last = reinterpret_cast<int *>(w + o_last);
  unsigned char *tb = reinterpret_cast<unsigned char *>(w + o_t);
  array<int, 3> *found = reinterpret_cast<array<int, 3> *>(w + o_found);
  std::memset(tb, 0, 8), std::memcpy(tb + 8, str.data(), n), std::memset(tb + 8 + n, 0, 8);
  const unsigned char *t = tb + 8;
  suffix_array_internal::sa_is(t, n, 256, sa);
  {
    // 順位の小さい順に位置を読み、順位が今より小さい位置の集合と大きい位置の集合から、i より後ろで最初の位置を引く。
    FastSet smaller(n), larger(n, [](int) { return true; });
    for (int k = 0; k < n; ++k) {
      const int i = sa[k];
      larger.erase(i);
      const int a = smaller.next(i + 1), b = larger.next(i + 1);
      nxt0[i] = a < 0 ? n : a, nxt1[i] = b < 0 ? n : b;
      smaller.insert(i);
    }
  }
  std::memset(last, 0, half * 4);
  long long budget = 32LL * n;
  Slow slow{t, n, sa, nullptr, nullptr};
  int nf = 0;
  auto extend = [&](int i, int j, int p) __attribute__((always_inline)) {
    if (__builtin_expect(budget < 0, 0)) return slow(i, j, p);
    int e = countr_zero(load(t + i) ^ load(t + j)) >> 3, b = countl_zero(load(t + i - 8) ^ load(t + j - 8)) >> 3;
    if (__builtin_expect(e < 8 && b < 8, 1)) {
      e = min(e, n - j), b = min(b, i);
      return e + b >= p ? pair<int, int>(i - b, j + e) : pair<int, int>(-1, 0);
    }
    e = e < 8 ? min(e, n - j) : forward(t, i, j, 8, n - j);
    b = b < 8 ? min(b, i) : backward(t, i, j, 8, i, p - e);
    budget -= e + b;
    if (e + b < p) return pair<int, int>(-1, 0);
    if (b >= 8) {
      const int b0 = b;
      b = backward(t, i, j, b, i, i);
      budget -= b - b0;
    }
    return pair<int, int>(i - b, j + e);
  };
  auto check = [&](int i, int j) __attribute__((always_inline)) {
    const int p = j - i;
    if (2 * p > n || i + p <= last[p]) return;  // 長さが 2p に届かない根と、同じ周期の前に見つけた run の中に収まる根
    const auto [l, r] = extend(i, j, p);
    if (l < 0) return;
    found[nf++] = {p, l, r};
    last[p] = r;
  };
  for (int i = 0; i < n; ++i) {
    check(i, nxt0[i]);
    if (const int j = nxt1[i]; j < n) check(i, j);
  }
  int *cnt = last;  // 周期で数え分ける (周期ごとには始まりの順に見つかっている)
  std::memset(cnt, 0, half * 4);
  for (int k = 0; k < nf; ++k) ++cnt[found[k][0] - 1];
  for (int p = 0, acc = 0; p < int(half); ++p) {
    const int c = cnt[p];
    cnt[p] = acc, acc += c;
  }
  vector<array<int, 3>> res(nf);
  for (int k = 0; k < nf; ++k) res[cnt[found[k][0] - 1]++] = found[k];
  return res;
}
}  // namespace lyndon_fs_hp2

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() { ans = lyndon_fs_hp2::runs(s); }

  const vector<array<int, 3>> &answer() const { return ans; }
};
