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

  // 覆う節の番号を分岐なしで集める。左は見つけた順に L へ、右は見つけた順に R へ置き、左右とも
  // 「その段で取るか」を番号の最下位 bit から足す数で決める (配列への書き込みは毎段する)。
  // 予測の外れる分岐が無くなる。集めたあとで、L を順に、R を逆順に x へ当てる。
  i64 query_bl_apply(int l, int r, i64 x) const {
    int L[32], R[32], nl = 0, nr = 0;
    for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
      L[nl] = l, nl += l & 1, l += l & 1;
      R[nr] = r - 1, nr += r & 1, r -= r & 1;
    }
    u32 v = mont::enc(x);
    for (int i = 0; i < nl; ++i) v = apply(d[L[i]], v);
    while (nr) v = apply(d[R[--nr]], v);
    return mont::dec(v);
  }

  // 集め方は query_bl_apply と同じ。左は x に順に当て、右はそれと並べて 1 つの写像に合成しておき、
  // 最後に当てる。左右の鎖が並んで進むので、鎖の長さが半分ほどになる。
  i64 query_bl_split(int l, int r, i64 x) const {
    int L[32], R[32], nl = 0, nr = 0;
    for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
      L[nl] = l, nl += l & 1, l += l & 1;
      R[nr] = r - 1, nr += r & 1, r -= r & 1;
    }
    u32 v = mont::enc(x);
    F g{mont::R1, 0};
    const int k = nl < nr ? nl : nr;
    for (int i = 0; i < k; ++i) v = apply(d[L[i]], v), g = op(d[R[i]], g);
    for (int i = k; i < nl; ++i) v = apply(d[L[i]], v);
    for (int i = k; i < nr; ++i) g = op(d[R[i]], g);
    return mont::dec(apply(g, v));
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
