#pragma once
#include "_mont.hpp"

// ACL の lazy_segtree と同じ非再帰の遅延セグ木で、作用は区間への代入。長さ 2^j の節に g を代入した
// 値 g^(2^j) は、代入のたびに 2 乗を重ねて表 (pool) に作り、節の作用には表の位置を持たせる
// (0 は作用なし)。表は代入が覆う節のいちばん高い段までだけ作る。配るときは子の段の冪を表から引く。
// 節は 1 次関数 x -> m x + a を Montgomery の u32 の組で持ち、大きさは 2 の冪で余りは恒等写像。
struct Solver {
  struct F {
    u32 m, a;
  };
  static F op(F l, F r) { return {mont::mul(r.m, l.m), mont::mulsum(r.m, l.a, r.a, mont::R1)}; }
  int n, lg, sz;
  HugeArray<F> d;
  HugeArray<u32> lz;
  vector<F> pool;

  explicit Solver(const vector<array<i64, 2>> &f) : n((int)f.size()) {
    lg = 0;
    while ((1 << lg) < n) ++lg;
    sz = 1 << lg;
    d = HugeArray<F>(2 * sz);
    lz = HugeArray<u32>(sz);
    for (int i = 0; i < sz; ++i) d[sz + i] = i < n ? F{mont::enc(f[i][0]), mont::enc(f[i][1])} : F{mont::R1, 0};
    for (int k = sz - 1; k >= 1; --k) d[k] = op(d[2 * k], d[2 * k + 1]);
    for (int k = 0; k < sz; ++k) lz[k] = 0;
    pool.reserve(size_t(1) << 24);
    pool.push_back({mont::R1, 0});  // 位置 0 は作用なしの印に使うので埋めておく
  }

  // 段 i の節 k の作用を子 (段 i - 1) へ配る。
  void push(int k, int i) {
    const u32 o = lz[k];
    if (!o) return;
    const F v = pool[o + i - 1];
    d[2 * k] = v, d[2 * k + 1] = v;
    if (i > 1) lz[2 * k] = o, lz[2 * k + 1] = o;
    lz[k] = 0;
  }
  void update(int k) { d[k] = op(d[2 * k], d[2 * k + 1]); }
  void push_paths(int l, int r) {
    for (int i = lg; i >= 1; --i) {
      if (((l >> i) << i) != l) push(l >> i, i);
      if (((r >> i) << i) != r) push((r - 1) >> i, i);
    }
  }

  void assign(int l, int r, i64 c, i64 dd) {
    l += sz, r += sz;
    push_paths(l, r);
    const u32 o = (u32)pool.size();
    pool.push_back({mont::enc(c), mont::enc(dd)});
    int lvl = 0;
    for (int a = l, b = r; a < b; a >>= 1, b >>= 1, ++lvl) {
      if (lvl > 0) {
        const F p = pool.back();
        pool.push_back(op(p, p));
      }
      if (a & 1) {
        d[a] = pool[o + lvl];
        if (a < sz) lz[a] = o;
        ++a;
      }
      if (b & 1) {
        --b;
        d[b] = pool[o + lvl];
        if (b < sz) lz[b] = o;
      }
    }
    for (int i = 1; i <= lg; ++i) {
      if (((l >> i) << i) != l) update(l >> i);
      if (((r >> i) << i) != r) update((r - 1) >> i);
    }
  }

  i64 query(int l, int r, i64 x) {
    l += sz, r += sz;
    push_paths(l, r);
    F sl{mont::R1, 0}, sr{mont::R1, 0};
    for (; l < r; l >>= 1, r >>= 1) {
      if (l & 1) sl = op(sl, d[l++]);
      if (r & 1) sr = op(d[--r], sr);
    }
    const F t = op(sl, sr);
    return mont::dec(mont::mulsum(t.m, mont::enc(x), t.a, mont::R1));
  }
};
