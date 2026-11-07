#pragma once
#include "_mont.hpp"

// ACL の lazy_segtree から値の側を落とした非再帰の双対セグ木を、この問題の作用に特化して
// 書いたもの。作用は内部の節だけ lz に持ち、葉にかかる作用は値に直接当てる。区間の作用の前に
// 境界の 2 本の道を根から配るので、根から葉への道では上の節ほど新しい作用になる。1 点の取得は
// 道を配らずに、葉から根へ作用を順に当てるだけ (読み出しだけで済む)。値は Montgomery 表現。
struct Solver {
  struct Lz {
    u32 m, a;
  };
  int n, lg, sz;
  HugeArray<u32> v;
  HugeArray<Lz> lz;

  explicit Solver(const vector<i64> &a) : n((int)a.size()) {
    lg = 0;
    while ((1 << lg) < n) ++lg;
    sz = 1 << lg;
    v = HugeArray<u32>(sz);
    lz = HugeArray<Lz>(sz);
    for (int i = 0; i < sz; ++i) v[i] = i < n ? mont::enc(a[i]) : 0;
    for (int k = 0; k < sz; ++k) lz[k] = {mont::R1, 0};
  }

  void apply_node(int k, u32 m, u32 a) {
    if (k < sz) {
      Lz &z = lz[k];
      z.a = mont::mulsum(m, z.a, a, mont::R1);
      z.m = mont::mul(m, z.m);
    } else {
      v[k - sz] = mont::mulsum(m, v[k - sz], a, mont::R1);
    }
  }
  void push(int k) {
    const Lz z = lz[k];
    if (z.m == mont::R1 && z.a == 0) return;
    apply_node(2 * k, z.m, z.a);
    apply_node(2 * k + 1, z.m, z.a);
    lz[k] = {mont::R1, 0};
  }

  void affine(int l, int r, i64 b, i64 c) {
    const u32 m = mont::enc(b), a = mont::enc(c);
    l += sz, r += sz;
    for (int i = lg; i >= 1; --i) {
      if (((l >> i) << i) != l) push(l >> i);
      if (((r >> i) << i) != r) push((r - 1) >> i);
    }
    for (; l < r; l >>= 1, r >>= 1) {
      if (l & 1) apply_node(l++, m, a);
      if (r & 1) apply_node(--r, m, a);
    }
  }

  i64 get(int i) {
    u32 x = v[i];
    for (int k = (i + sz) >> 1; k >= 1; k >>= 1) {
      const Lz z = lz[k];
      x = mont::mulsum(z.m, x, z.a, mont::R1);
    }
    return mont::dec(x);
  }
};
