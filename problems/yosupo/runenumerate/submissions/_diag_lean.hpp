#pragma once
// lyndon_lean_hp (_lyndon_lean.hpp) の段を 1 つだけ run() で測る診断用の中身。段は 0 が接尾辞配列、1 が 2 つの Lyndon 配列 (作業領域の
// 確保と連結リスト)、2 が根を調べて並べる段 (文字列の写しから数え分けまで)。測る段より前は構築で、後ろは answer() で回すので、
// 出力は正しいまま。計算は lyndon_lean_hp と同じ。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "neo/string/suffix_array.hpp"
#include "_lyndon_hybrid.hpp"
#include "_lyndon_lean.hpp"

namespace diag_lean {
struct State {
  struct Node {
    int nx, pv;
  };
  std::string s;
  int n = 0;
  std::size_t nodes = 0, half = 0;
  std::vector<int> sa;
  std::unique_ptr<lyndon_lean::Buffer> buf;
  Node *l0 = nullptr, *l1 = nullptr;
  int *last = nullptr;
  unsigned char *tb = nullptr;
  std::vector<std::array<int, 3>> res;
  void stage(int k) {
    if (n < 2) return;
    if (k == 0) sa = suffix_array(s);
    if (k == 1) {
      nodes = std::size_t(n) + 2, half = std::size_t(n) / 2 + 1;
      const std::size_t bytes = 2 * nodes * sizeof(Node) + ((half * sizeof(int) + 7) & ~std::size_t(7)) + std::size_t(n) + 16;
      buf = std::make_unique<lyndon_lean::Buffer>(bytes, bytes >= (std::size_t(1) << 20));
      l0 = reinterpret_cast<Node *>(buf->get()), l1 = l0 + nodes;
      last = reinterpret_cast<int *>(l1 + nodes);
      tb = reinterpret_cast<unsigned char *>(last) + ((half * sizeof(int) + 7) & ~std::size_t(7));
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
    }
    if (k == 2) {
      using namespace lyndon_lean;
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
        if (2 * p > n || i + p <= last[p]) return;
        const auto [l, r] = extend(i, j, p);
        if (l < 0) return;
        found.push_back({p, l, r});
        last[p] = r;
      };
      for (int i = 0; i < n; ++i) {
        check(i, l0[i + 1].nx);
        if (const int j = l1[i + 1].nx; j < n) check(i, j);
      }
      int *cnt = last;
      std::memset(cnt, 0, half * sizeof(int));
      for (const auto &x : found) ++cnt[x[0] - 1];
      for (int p = 0, acc = 0; p < int(half); ++p) {
        const int c = cnt[p];
        cnt[p] = acc, acc += c;
      }
      res.resize(found.size());
      for (const auto &x : found) res[cnt[x[0] - 1]++] = x;
    }
  }
};
}  // namespace diag_lean

template <int K> struct DiagLeanSolver {
  mutable diag_lean::State st;
  explicit DiagLeanSolver(const std::string &s) {
    st.s = s, st.n = int(s.size());
    for (int k = 0; k < K; ++k) st.stage(k);
  }
  void run() { st.stage(K); }
  const std::vector<std::array<int, 3>> &answer() const {
    for (int k = K + 1; k < 3; ++k) st.stage(k);
    return st.res;
  }
};
