#pragma once
// lyndon_lean_hp で、接尾辞配列と見つけた run の列も huge page の作業領域に置き、新しく確保するのを返す配列だけにした版。5 回目
// (a5bf3c9b の run 38127442511、EPYC 7763) では、配列を 2 MB 減らした lyndon_lean が lyndon_ll_w8 より x64-gcc で 2.0 ms 速く、
// 作業領域に huge page を頼む lyndon_lean_hp がさらに 1.4 ms 速かった。CI では、新しく確保した領域に初めて触る代価が 1 MB あたり
// 1 ms ほどある。接尾辞配列は NeoLibrary の suffix_array_internal::sa_is に作業領域の中を渡して書かせる (sa_is の中で確保する印や
// 再帰の配列はそのまま)。run の数は n 未満なので (Bannai ほか 2017)、run の列は n 個分を取る (触った分だけページが用意される)。
// 予算を使い切ったあとの道も、作業領域の接尾辞配列から表を作る。ほかは lyndon_lean_hp と同じ。
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
#include "neo/data_structure/RangeMin.hpp"
#include "_lyndon_lean.hpp"

namespace lyndon_lean_hp2 {
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
  struct Node {
    int nx, pv;
  };
  auto up8 = [](size_t x) { return (x + 7) & ~size_t(7); };
  const size_t nodes = size_t(n) + 2, half = size_t(n) / 2 + 1;
  const size_t o_sa = 0, o_l0 = up8(size_t(n) * 4), o_l1 = o_l0 + nodes * 8, o_last = o_l1 + nodes * 8, o_t = o_last + up8(half * 4),
               o_found = o_t + up8(size_t(n) + 16), bytes = o_found + size_t(n) * 12;
  Buffer buf(bytes, bytes >= (size_t(1) << 20));
  char *w = buf.get();
  int *sa = reinterpret_cast<int *>(w + o_sa);
  Node *l0 = reinterpret_cast<Node *>(w + o_l0), *l1 = reinterpret_cast<Node *>(w + o_l1);  // 位置 x (-1 から n まで) の節は x + 1 番目
  int *last = reinterpret_cast<int *>(w + o_last);
  unsigned char *tb = reinterpret_cast<unsigned char *>(w + o_t);
  array<int, 3> *found = reinterpret_cast<array<int, 3> *>(w + o_found);
  std::memset(tb, 0, 8), std::memcpy(tb + 8, str.data(), n), std::memset(tb + 8 + n, 0, 8);
  const unsigned char *t = tb + 8;
  suffix_array_internal::sa_is(t, n, 256, sa);
  for (int x = 0; x <= n + 1; ++x) l0[x] = l1[x] = {x, x - 2};
  for (int k = 0; k < n; ++k) {
    {
      const int i = sa[n - 1 - k];
      const Node d = l0[i + 1];
      l0[d.pv + 1].nx = d.nx, l0[d.nx + 1].pv = d.pv;
    }
    {
      const int i = sa[k];
      const Node d = l1[i + 1];
      l1[d.pv + 1].nx = d.nx, l1[d.nx + 1].pv = d.pv;
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
    check(i, l0[i + 1].nx);
    if (const int j = l1[i + 1].nx; j < n) check(i, j);
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
}  // namespace lyndon_lean_hp2

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() { ans = lyndon_lean_hp2::runs(s); }

  const vector<array<int, 3>> &answer() const { return ans; }
};
