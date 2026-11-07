#pragma once
#include "_mont.hpp"

// 葉を [n, 2n) に置く非再帰のセグメント木 (n は 2 の冪に揃えない)。節は 1 次関数 x -> m x + a
// を Montgomery 表現の u32 の組で持つ。op(l, r) は l を先に当てる合成で、区間の取得は左右の
// 累積を別々に持てば可換でなくても正しい。query の 3 通りの書き方を提出ごとに選ぶ。
struct SegMont {
  struct F {
    u32 m, a;
  };
  int n;
  HugeArray<F> d;

  static F op(F l, F r) { return {mont::mul(r.m, l.m), mont::mulsum(r.m, l.a, r.a, mont::R1)}; }
  static u32 apply(F f, u32 x) { return mont::mulsum(f.m, x, f.a, mont::R1); }

  explicit SegMont(const vector<array<i64, 2>> &f) : n((int)f.size()), d(2 * f.size()) {
    for (int i = 0; i < n; ++i) d[n + i] = {mont::enc(f[i][0]), mont::enc(f[i][1])};
    for (int k = n - 1; k >= 1; --k) d[k] = op(d[2 * k], d[2 * k + 1]);
  }

  void set(int p, i64 c, i64 dd) {
    p += n;
    d[p] = {mont::enc(c), mont::enc(dd)};
    for (p >>= 1; p >= 1; p >>= 1) d[p] = op(d[2 * p], d[2 * p + 1]);
  }

  // 左右の累積を合成してから x に当てる。左と右の 2 本の鎖が並んで進む。
  i64 query_compose(int l, int r, i64 x) const {
    F sl{mont::R1, 0}, sr{mont::R1, 0};
    for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
      if (l & 1) sl = op(sl, d[l++]);
      if (r & 1) sr = op(d[--r], sr);
    }
    return mont::dec(apply(op(sl, sr), mont::enc(x)));
  }

  // 左の節は見つけた順に x へ直接当て、右の節は積んでおいて最後に逆順で当てる。1 本の鎖。
  i64 query_apply(int l, int r, i64 x) const {
    u32 v = mont::enc(x);
    int st[64], top = 0;
    for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
      if (l & 1) v = apply(d[l++], v);
      if (r & 1) st[top++] = --r;
    }
    while (top) v = apply(d[st[--top]], v);
    return mont::dec(v);
  }

  // 節を左から順に並べ、隣どうしを合成する段を重ねて 1 つにする。鎖の長さが log になる。
  i64 query_tree(int l, int r, i64 x) const {
    F buf[64];
    int st[32], nl = 0, nr = 0;
    for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
      if (l & 1) buf[nl++] = d[l++];
      if (r & 1) st[nr++] = --r;
    }
    while (nr) buf[nl++] = d[st[--nr]];
    while (nl > 1) {
      int k = 0;
      for (int i = 0; i + 1 < nl; i += 2) buf[k++] = op(buf[i], buf[i + 1]);
      if (nl & 1) buf[k++] = buf[nl - 1];
      nl = k;
    }
    return mont::dec(apply(buf[0], mont::enc(x)));
  }
};
