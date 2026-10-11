#pragma once
// lyndon_fused で、接尾辞配列、文字列の写し、見つけた run の列と基数ソートの作業の列を、huge page を頼む 1 つの作業領域に置く版
// (lyndon_lean_hp2 と同じ置き方)。6 回目と 7 回目 (700cb890 の run 38128344191、EPYC 7763) では、lyndon_fused が x64-gcc で 7.98 ms、
// 作業領域に huge page を頼む lyndon_lean_hp2 が 7.76 ms だった。診断では、新しく確保した 4 MB を埋めるのに 1.85 ms、huge page を
// 頼むと 0.34 ms、触っておいた 4 MB を埋め直すだけなら 0.12 ms かかった。基数ソートの最後の段は、返す配列へ直に書く。
// ほかは lyndon_fused と同じ。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
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

namespace lyndon_fused_hp {
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
// 予算を使い切ったあとの道。初めて呼ばれたときに LCE の表を作り、根 [i, j) の {手前の一致, 後ろの一致} を返す。
struct Slow {
  const unsigned char *s;
  int n;
  const int *sa;
  unique_ptr<Lce> fwd, bwd;
  [[gnu::noinline]] pair<int, int> operator()(int i, int j) {
    if (!fwd) {
      const vector<unsigned char> rv(make_reverse_iterator(s + n), make_reverse_iterator(s));
      vector<int> rsa(n);
      suffix_array_internal::sa_is(rv.data(), n, 256, rsa.data());
      fwd = make_unique<Lce>(s, n, sa), bwd = make_unique<Lce>(rv.data(), n, rsa.data());
    }
    return {i > 0 ? (*bwd)(n - i, n - j) : 0, j < n ? (*fwd)(i, j) : 0};  // 反転した文字列で、n - i と n - j から始まる接尾辞
  }
};
inline vector<array<int, 3>> runs(const string &str) {
  using namespace lyndon_lean;
  const int n = int(str.size());
  if (n < 2) return {};
  auto up8 = [](size_t x) { return (x + 7) & ~size_t(7); };
  const size_t o_t = up8(size_t(n) * 4), o_found = o_t + up8(size_t(n) + 16), o_tmp = o_found + size_t(n) * 12,
               bytes = o_tmp + size_t(n) * 12;
  Buffer buf(bytes, bytes >= (size_t(1) << 20));
  char *w = buf.get();
  int *sa = reinterpret_cast<int *>(w);
  unsigned char *tb = reinterpret_cast<unsigned char *>(w + o_t);
  array<int, 3> *found = reinterpret_cast<array<int, 3> *>(w + o_found), *tmp = reinterpret_cast<array<int, 3> *>(w + o_tmp);
  std::memset(tb, 0, 8), std::memcpy(tb + 8, str.data(), n), std::memset(tb + 8 + n, 0, 8);
  const unsigned char *t = tb + 8;
  suffix_array_internal::sa_is(t, n, 256, sa);
  long long budget = 32LL * n;
  Slow slow{t, n, sa, nullptr, nullptr};
  int nf = 0;
  auto check = [&](int i, int j) __attribute__((always_inline)) {
    const int p = j - i;
    if (2 * p > n) return;  // 長さが 2p に届かない
    if (__builtin_expect(budget < 0, 0)) {
      const auto [b, e] = slow(i, j);
      if (b < p && e + b >= p) found[nf++] = {p, i - b, j + e};
      return;
    }
    // 根の手前の 8 バイトと後ろの 8 バイト
    int e = countr_zero(load(t + i) ^ load(t + j)) >> 3, b = countl_zero(load(t + i - 8) ^ load(t + j - 8)) >> 3;
    if (__builtin_expect(e < 8 && b < 8, 1)) {
      e = min(e, n - j), b = min(b, i);
      if (b < p && e + b >= p) found[nf++] = {p, i - b, j + e};
      return;
    }
    b = b < 8 ? min(b, i) : backward(t, i, j, 8, i, p);
    budget -= b;
    if (b >= p) return;  // run の中の根。run は手前へずらした根から見つかる
    e = e < 8 ? min(e, n - j) : forward(t, i, j, 8, n - j);
    budget -= e;
    if (e + b >= p) found[nf++] = {p, i - b, j + e};
  };
  {
    FastSet smaller(n), larger(n, [](int) { return true; });
    for (int k = 0; k < n; ++k) {
      const int i = sa[k];
      larger.erase(i);
      const int a = smaller.next(i + 1), c = larger.next(i + 1);
      smaller.insert(i);
      check(i, a < 0 ? n : a);
      if (c >= 0) check(i, c);  // 逆の順で根が文字列の終わりまで届く候補は見ない (_lyndon_runs.hpp の collect と同じ)
    }
  }
  // (周期, 始まり) の昇順に、12 bit ずつの基数ソートで並べる。周期は n / 2 以下、始まりは n 未満。最後の段は返す配列へ書く。
  vector<array<int, 3>> res(nf);
  const int lb = bit_width(unsigned(n)), bits = lb + bit_width(unsigned(n / 2)), passes = (bits + 11) / 12;
  array<int, 3> *src = found;
  for (int q = 0; q < passes; ++q) {
    const int sh = 12 * q;
    array<int, 3> *dst = q == passes - 1 ? res.data() : src == found ? tmp : found;
    array<int, 4097> cnt{};
    auto digit = [&](const array<int, 3> &x) { return int(((uint64_t(x[0]) << lb | unsigned(x[1])) >> sh) & 4095); };
    for (int k = 0; k < nf; ++k) ++cnt[digit(src[k]) + 1];
    for (int d = 0; d < 4096; ++d) cnt[d + 1] += cnt[d];
    for (int k = 0; k < nf; ++k) dst[cnt[digit(src[k])]++] = src[k];
    src = dst;
  }
  return res;
}
}  // namespace lyndon_fused_hp

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() { ans = lyndon_fused_hp::runs(s); }

  const vector<array<int, 3>> &answer() const { return ans; }
};
