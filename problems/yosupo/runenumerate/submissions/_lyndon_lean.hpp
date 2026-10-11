#pragma once
// lyndon_lean_hp の中身で、lyndon_lean_hp2 と lyndon_fused_hp も作業領域と 8 バイトずつの比べに使う。消した lyndon_ll_w8 の
// 計算を 1 つの関数にまとめ、確保する配列を減らした形。
// 連結リストから位置 i を外したあと、i の節の次の位置は、外したときの値 (i より後ろで順位が小さい、または大きい最初の位置) のまま
// 変わらない (外した節が、残っている節の隣になることはない)。そこで 2 つの Lyndon 配列を別に持たず、節から読む。周期 p が n / 2 を
// 超える根は run にならないので見ず、周期ごとの最後の run の終わりの配列を n / 2 + 1 個にし、並べるときの数え分けにも使い回す。
// 節、文字列の写し、周期ごとの配列は 1 つの作業領域に置き、HP が true なら 2 MB 境界に寄せて huge page を頼む (Linux のみ)。
// 根を伸ばす手と、予算を使い切ったあとの道は lyndon_ll_w8 と同じ。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "neo/string/suffix_array.hpp"
#include "_lyndon_hybrid.hpp"

namespace lyndon_lean {
inline uint64_t load(const unsigned char *p) {
  uint64_t x;
  std::memcpy(&x, p, 8);
  return x;
}
// t[i + k] = t[j + k] が続く長さ。e 文字までは一致していると分かっているときに、lim 文字を上限として求める。
inline int forward(const unsigned char *t, int i, int j, int e, int lim) {
  for (; e < lim; e += 8)
    if (const uint64_t x = load(t + i + e) ^ load(t + j + e)) return std::min(e + (std::countr_zero(x) >> 3), lim);
  return lim;
}
// t[i - 1 - k] = t[j - 1 - k] が続く長さ。b 文字までは一致していると分かっているときに、lim 文字を上限とし、stop 文字以上と分かった
// ところで止める (止めたときの値は下限)。
inline int backward(const unsigned char *t, int i, int j, int b, int lim, int stop) {
  for (; b < lim && b < stop; b += 8)
    if (const uint64_t y = load(t + i - 8 - b) ^ load(t + j - 8 - b)) return std::min(b + (std::countl_zero(y) >> 3), lim);
  return std::min(b, lim);
}
// bytes byte の作業領域。0 では埋めない。hp なら 2 MB 境界に寄せて huge page を頼む。
class Buffer {
  static constexpr std::size_t H = std::size_t(1) << 21;
  std::unique_ptr<char[]> raw;
  char *p = nullptr;

 public:
  Buffer(std::size_t bytes, bool hp) {
    const std::size_t a = hp ? H : 64, b = (bytes + a - 1) & ~(a - 1);
    raw.reset(new char[b + a]);
    p = reinterpret_cast<char *>((reinterpret_cast<std::uintptr_t>(raw.get()) + a - 1) & ~std::uintptr_t(a - 1));
#ifdef __linux__
    if (hp) madvise(p, b, MADV_HUGEPAGE);
#endif
  }
  char *get() const { return p; }
};
template <bool HP> std::vector<std::array<int, 3>> runs(const std::string &s) {
  const int n = int(s.size());
  if (n < 2) return {};
  const std::vector<int> sa = suffix_array(s);
  struct Node {
    int nx, pv;
  };
  const std::size_t nodes = std::size_t(n) + 2, half = std::size_t(n) / 2 + 1;
  const std::size_t bytes = 2 * nodes * sizeof(Node) + ((half * sizeof(int) + 7) & ~std::size_t(7)) + std::size_t(n) + 16;
  Buffer buf(bytes, HP && bytes >= (std::size_t(1) << 20));
  Node *l0 = reinterpret_cast<Node *>(buf.get()), *l1 = l0 + nodes;  // 位置 x (-1 から n まで) の節は x + 1 番目
  int *last = reinterpret_cast<int *>(l1 + nodes);
  unsigned char *tb = reinterpret_cast<unsigned char *>(last) + ((half * sizeof(int) + 7) & ~std::size_t(7));
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
  std::memset(tb, 0, 8), std::memcpy(tb + 8, s.data(), n), std::memset(tb + 8 + n, 0, 8);
  const unsigned char *t = tb + 8;
  std::memset(last, 0, half * sizeof(int));
  long long budget = 32LL * n;
  lyndon_hybrid::Slow slow{s, sa, nullptr, nullptr};
  std::vector<std::array<int, 3>> found;
  auto extend = [&](int i, int j, int p) __attribute__((always_inline)) {
    if (__builtin_expect(budget < 0, 0)) return slow(i, j, p);
    int e = std::countr_zero(load(t + i) ^ load(t + j)) >> 3, b = std::countl_zero(load(t + i - 8) ^ load(t + j - 8)) >> 3;
    if (__builtin_expect(e < 8 && b < 8, 1)) {
      e = std::min(e, n - j), b = std::min(b, i);
      return e + b >= p ? std::pair<int, int>(i - b, j + e) : std::pair<int, int>(-1, 0);
    }
    e = e < 8 ? std::min(e, n - j) : forward(t, i, j, 8, n - j);
    b = b < 8 ? std::min(b, i) : backward(t, i, j, 8, i, p - e);
    budget -= e + b;
    if (e + b < p) return std::pair<int, int>(-1, 0);
    if (b >= 8) {
      const int b0 = b;
      b = backward(t, i, j, b, i, i);
      budget -= b - b0;
    }
    return std::pair<int, int>(i - b, j + e);
  };
  auto check = [&](int i, int j) __attribute__((always_inline)) {
    const int p = j - i;
    if (2 * p > n || i + p <= last[p]) return;  // 長さが 2p に届かない根と、同じ周期の前に見つけた run の中に収まる根
    const auto [l, r] = extend(i, j, p);
    if (l < 0) return;
    found.push_back({p, l, r});
    last[p] = r;
  };
  // 逆の順で根が文字列の終わりまで届く候補は見ない (原始的でない根になりうる。その run は文字の順の側で見つかる)。
  for (int i = 0; i < n; ++i) {
    check(i, l0[i + 1].nx);
    if (const int j = l1[i + 1].nx; j < n) check(i, j);
  }
  // 周期で数え分ける (周期ごとには始まりの順に見つかっている)。last を数える配列に使い回す。
  int *cnt = last;
  std::memset(cnt, 0, half * sizeof(int));
  for (const auto &x : found) ++cnt[x[0] - 1];
  for (int p = 0, acc = 0; p < int(half); ++p) {
    const int c = cnt[p];
    cnt[p] = acc, acc += c;
  }
  std::vector<std::array<int, 3>> res(found.size());
  for (const auto &x : found) res[cnt[x[0] - 1]++] = x;
  return res;
}
}  // namespace lyndon_lean
