#pragma once
#include "_mont.hpp"

// ACL の lazy_segtree と同じ非再帰の遅延セグ木を、この問題の作用に特化して書いたもの。
// 値は Montgomery 表現 (2p 未満) の u32。和は d に、作用は内部の節だけ lz に別々に持つ。
// 節の長さは深さで決まるので、深さごとに長さの Montgomery 表現を持ち、作用は
// sum = m sum + a len を 1 回の還元で求める。作用が恒等なら子へ配らない。
struct Solver {
  struct Lz {
    u32 m, a;
  };
  int n, lg, sz;
  HugeArray<u32> d;
  HugeArray<Lz> lz;
  u32 lenc[32];

  explicit Solver(const vector<i64> &a) : n((int)a.size()) {
    lg = 0;
    while ((1 << lg) < n) ++lg;
    sz = 1 << lg;
    d = HugeArray<u32>(2 * sz);
    lz = HugeArray<Lz>(sz);
    for (int i = 0; i < 2 * sz; ++i) d[i] = 0;
    for (int i = 0; i < n; ++i) d[sz + i] = mont::enc(a[i]);
    for (int k = sz - 1; k >= 1; --k) d[k] = mont::add(d[2 * k], d[2 * k + 1]);
    for (int k = 0; k < sz; ++k) lz[k] = {mont::R1, 0};
    for (int h = 0; h <= lg; ++h) lenc[h] = mont::enc(u32(sz >> h));
  }

  void apply_node(int k, u32 m, u32 a, int depth) {
    d[k] = mont::mulsum(m, d[k], a, lenc[depth]);
    if (k < sz) {
      Lz &z = lz[k];
      z.a = mont::mulsum(m, z.a, a, mont::R1);
      z.m = mont::mul(m, z.m);
    }
  }
  void push(int k, int depth) {
    const Lz z = lz[k];
    if (z.m == mont::R1 && z.a == 0) return;
    apply_node(2 * k, z.m, z.a, depth + 1);
    apply_node(2 * k + 1, z.m, z.a, depth + 1);
    lz[k] = {mont::R1, 0};
  }
  void update(int k) { d[k] = mont::add(d[2 * k], d[2 * k + 1]); }

  void affine(int l, int r, i64 b, i64 c) {
    const u32 m = mont::enc(b), a = mont::enc(c);
    l += sz, r += sz;
    for (int i = lg; i >= 1; --i) {
      if (((l >> i) << i) != l) push(l >> i, lg - i);
      if (((r >> i) << i) != r) push((r - 1) >> i, lg - i);
    }
    for (int x = l, y = r, depth = lg; x < y; x >>= 1, y >>= 1, --depth) {
      if (x & 1) apply_node(x++, m, a, depth);
      if (y & 1) apply_node(--y, m, a, depth);
    }
    for (int i = 1; i <= lg; ++i) {
      if (((l >> i) << i) != l) update(l >> i);
      if (((r >> i) << i) != r) update((r - 1) >> i);
    }
  }

  i64 sum(int l, int r) {
    l += sz, r += sz;
    for (int i = lg; i >= 1; --i) {
      if (((l >> i) << i) != l) push(l >> i, lg - i);
      if (((r >> i) << i) != r) push((r - 1) >> i, lg - i);
    }
    u32 sl = 0, sr = 0;
    for (; l < r; l >>= 1, r >>= 1) {
      if (l & 1) sl = mont::add(sl, d[l++]);
      if (r & 1) sr = mont::add(sr, d[--r]);
    }
    return mont::dec(mont::add(sl, sr));
  }
};
