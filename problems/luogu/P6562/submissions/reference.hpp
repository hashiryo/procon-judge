#pragma once
// 参照実装 (ライブラリに依存しない)。題解の方針: 操作を B 個ずつの塊に分け、塊の初めの列 v とその部分集合の和 s を持つ。
//   B は sqrt(n 2^n) (小さい n でも塊の作り直しを通る)。
//   問い (x, y) = sum_{x ⊆ c ⊆ y}: 塊より前の分は、y \ x の bit が少なければ v を素朴にたどり、多ければ x の 1 の bit を
//   f(x, y) = f(x - p, y) - f(x - p, y - p) (p は x の 1 の bit) で外して s から引く。塊の中の加算 (x', y', k) の分は、
//   x ∪ x' ⊆ (y ∩ y') なら k 2^{|y ∩ y'| - |x ∪ x'|} を足す。
//   加算: 塊の中の加算として覚え、y \ x の bit が少なければ md_v に素朴に足し、多ければ同じ式で「部分集合全部に足す」印 md_s に分ける。
//   塊の終わりに md_s を上位集合の和で配り、v と s を作り直す。
#include <cmath>
#include "pj.hpp"
struct Solver {
  int n;
  vector<u32> v, s, md_v, md_s, pc;
  vector<array<u32, 4>> ops;
  vector<u32> ans;
  vector<array<u32, 3>> blk;
  Solver(int n_, const vector<u32> &a_, const vector<array<u32, 4>> &ops_) : n(n_), v(a_), ops(ops_) {}
  void build() {
    const size_t N = size_t(1) << n;
    for (int i = 0; i < n; ++i)  // 上位集合の和で印を配る
      for (size_t j = 0; j < N; ++j)
        if (!(j >> i & 1)) md_s[j] += md_s[j | (size_t(1) << i)];
    for (size_t j = 0; j < N; ++j) v[j] += md_v[j] + md_s[j], md_v[j] = md_s[j] = 0, s[j] = v[j];
    for (int i = 0; i < n; ++i)
      for (size_t j = 0; j < N; ++j)
        if (j >> i & 1) s[j] += s[j ^ (size_t(1) << i)];
    blk.clear();
  }
  u32 ask(u32 x, u32 y) const {  // x の 1 の bit を外して s から
    if (!x) return s[y];
    const u32 p = x & -x;
    return ask(x ^ p, y) - ask(x ^ p, y ^ p);
  }
  void tag(u32 x, u32 y, u32 k) {
    if (!x) {
      md_s[y] += k;
      return;
    }
    const u32 p = x & -x;
    tag(x ^ p, y, k), tag(x ^ p, y ^ p, 0u - k);
  }
  void run() {
    const size_t N = size_t(1) << n;
    s.assign(N, 0), md_v.assign(N, 0), md_s.assign(N, 0), pc.assign(N, 0);
    for (size_t i = 1; i < N; ++i) pc[i] = pc[i >> 1] + (i & 1);
    build();
    const size_t B = max<size_t>(2, size_t(sqrt(double(size_t(n) << n))));
    for (auto &o : ops) {
      const u32 x = o[1], y = o[2];
      if ((x & y) != x) {
        if (o[0] == 2) ans.push_back(0);
        continue;
      }
      const bool small = int(pc[x ^ y]) <= n / 2;
      if (o[0] == 1) {
        blk.push_back({x, y, o[3]});
        if (small)
          for (u32 t = x ^ y;; t = (t - 1) & (x ^ y)) {
            md_v[t | x] += o[3];
            if (!t) break;
          }
        else tag(x, y, o[3]);
        if (blk.size() == B) build();
      } else {
        u32 r = 0;
        if (small)
          for (u32 t = x ^ y;; t = (t - 1) & (x ^ y)) {
            r += v[t | x];
            if (!t) break;
          }
        else r = ask(x, y);
        for (auto &[x2, y2, k] : blk) {
          const u32 lo = x | x2, hi = y & y2;
          if ((lo & hi) == lo) r += k << pc[lo ^ hi];
        }
        ans.push_back(r);
      }
    }
  }
  vector<u32> answer() const { return ans; }
};
