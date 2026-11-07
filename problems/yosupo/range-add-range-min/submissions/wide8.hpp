#pragma once
#include "pj.hpp"
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#ifdef __linux__
#include <sys/mman.h>
#endif

// wide2 の子を 8 個にした形。塊の 8 個の i64 は __m256i の 2 本 (キャッシュの 1 行) に収まる。段 h >= 1
// では、子の部分木全体に足した量 t と、子の最小に t を足した s を別の配列に持ち、区間の最小は s だけを
// 読む。区間の加算は下から全部掛かるレーンの s と t に足し、下から両端の道の s を取り直す (作用は配らない)。
// 区間の最小は書き込みのない再帰で、一部だけ掛かる子の結果に t を足す。要素の無いレーンは +inf。
namespace wide_min8 {
constexpr i64 INF = numeric_limits<i64>::max() / 4;
constexpr int LB = 3, B = 8;

template <class T> struct HugeArray {
  unique_ptr<char[]> raw;
  T *p = nullptr;
  HugeArray() = default;
  explicit HugeArray(size_t n) {
    const size_t want = max<size_t>(n, 1) * sizeof(T);
    const size_t A = want >= (size_t(1) << 20) ? (size_t(1) << 21) : 64;
    const size_t bytes = (want + A - 1) & ~(A - 1);
    raw.reset(new char[bytes + A]);
    p = reinterpret_cast<T *>((reinterpret_cast<uintptr_t>(raw.get()) + A - 1) & ~uintptr_t(A - 1));
#ifdef __linux__
    if (A > 64) madvise(p, bytes, MADV_HUGEPAGE);
#endif
  }
};

inline __m256i ld(const i64 *p) { return _mm256_load_si256((const __m256i *)p); }
inline void st(i64 *p, __m256i x) { _mm256_store_si256((__m256i *)p, x); }
inline __m256i vmin(__m256i a, __m256i b) { return _mm256_blendv_epi8(a, b, _mm256_cmpgt_epi64(a, b)); }
inline i64 hmin(__m256i x0, __m256i x1) {
  const __m256i m = vmin(x0, x1);
  __m128i c = _mm256_castsi256_si128(m);
  const __m128i h = _mm256_extracti128_si256(m, 1);
  c = _mm_blendv_epi8(c, h, _mm_cmpgt_epi64(c, h));
  const __m128i s = _mm_unpackhi_epi64(c, c);
  c = _mm_blendv_epi8(c, s, _mm_cmpgt_epi64(c, s));
  return _mm_cvtsi128_si64(c);
}
inline void masks(int lo, int hi, __m256i &k0, __m256i &k1) {
  const __m256i L = _mm256_set1_epi64x(lo - 1), R = _mm256_set1_epi64x(hi);
  const __m256i i0 = _mm256_setr_epi64x(0, 1, 2, 3), i1 = _mm256_setr_epi64x(4, 5, 6, 7);
  k0 = _mm256_and_si256(_mm256_cmpgt_epi64(i0, L), _mm256_cmpgt_epi64(R, i0));
  k1 = _mm256_and_si256(_mm256_cmpgt_epi64(i1, L), _mm256_cmpgt_epi64(R, i1));
}

class Tree {
  int n = 0, H = 0;
  i64 *S[24] = {}, *T[24] = {};
  HugeArray<i64> memS, memT;

  void add_lanes(int h, int b, int lo, int hi, i64 x) {
    __m256i k0, k1;
    masks(lo, hi, k0, k1);
    const __m256i xv = _mm256_set1_epi64x(x), x0 = _mm256_and_si256(k0, xv), x1 = _mm256_and_si256(k1, xv);
    i64 *s = S[h] + (size_t(b) << LB);
    st(s, _mm256_add_epi64(ld(s), x0)), st(s + 4, _mm256_add_epi64(ld(s + 4), x1));
    if (h == 0) return;
    i64 *t = T[h] + (size_t(b) << LB);
    st(t, _mm256_add_epi64(ld(t), x0)), st(t + 4, _mm256_add_epi64(ld(t + 4), x1));
  }
  i64 block_min(int h, int b) const {
    const i64 *s = S[h] + (size_t(b) << LB);
    return hmin(ld(s), ld(s + 4));
  }
  i64 lanes_min(int h, int b, int lo, int hi) const {
    const i64 *s = S[h] + (size_t(b) << LB);
    __m256i k0, k1;
    masks(lo, hi, k0, k1);
    const __m256i inf = _mm256_set1_epi64x(INF);
    return hmin(_mm256_blendv_epi8(inf, ld(s), k0), _mm256_blendv_epi8(inf, ld(s + 4), k1));
  }
  i64 min_rec(int h, int b, int lo, int hi) const {
    const int sh = LB * h;
    const int jl = (lo >> sh) & (B - 1), jr = ((hi - 1) >> sh) & (B - 1);
    if (h == 0) return lanes_min(0, b, jl, jr + 1);
    const int sm = (1 << sh) - 1, base = b << LB;
    const i64 *t = T[h];
    i64 res = INF;
    int fl = jl, fr = jr + 1;
    if (lo & sm) {
      const int c = base + jl, e = min(hi, (lo | sm) + 1);
      res = min_rec(h - 1, c, lo, e) + t[c];
      if (jl == jr) return res;
      fl = jl + 1;
    }
    if (hi & sm) {
      const int c = base + jr, s = max(lo, hi & ~sm);
      res = min(res, min_rec(h - 1, c, s, hi) + t[c]);
      fr = jr;
    }
    if (fl < fr) res = min(res, lanes_min(h, b, fl, fr));
    return res;
  }

public:
  explicit Tree(const vector<i64> &a) : n((int)a.size()) {
    int cnt[24], nb = (n + B - 1) >> LB;
    H = 0;
    size_t tot = 0;
    for (;;) {
      cnt[H++] = nb;
      tot += size_t(nb) << LB;
      if (nb == 1) break;
      nb = (nb + B - 1) >> LB;
    }
    memS = HugeArray<i64>(tot);
    memT = HugeArray<i64>(tot);
    size_t o = 0;
    for (int h = 0; h < H; ++h) S[h] = memS.p + o, T[h] = memT.p + o, o += size_t(cnt[h]) << LB;
    for (int i = 0; i < (cnt[0] << LB); ++i) S[0][i] = i < n ? a[i] : INF, T[0][i] = 0;
    for (int h = 1; h < H; ++h)
      for (int c = 0; c < (cnt[h] << LB); ++c) S[h][c] = c < cnt[h - 1] ? block_min(h - 1, c) : INF, T[h][c] = 0;
  }
  void add(int l, int r, i64 x) {
    for (int h = 0, p = l, q = r; p < q; ++h) {
      const int bp = p >> LB, bq = q >> LB;
      if (bp == bq || h == H - 1) {
        add_lanes(h, bp, p & (B - 1), q - (bp << LB), x);
        break;
      }
      if (p & (B - 1)) add_lanes(h, bp, p & (B - 1), B, x), p = bp + 1;
      else p = bp;
      if (q & (B - 1)) add_lanes(h, bq, 0, q & (B - 1), x), q = bq;
      else q = bq;
    }
    const int y = r - 1;
    for (int h = 1; h < H; ++h) {
      const int sh = LB * h, sm = (1 << sh) - 1;
      if (l & sm) {
        const int c = l >> sh;
        S[h][c] = block_min(h - 1, c) + T[h][c];
      }
      if (r & sm) {
        const int c = y >> sh;
        S[h][c] = block_min(h - 1, c) + T[h][c];
      }
    }
  }
  i64 minimum(int l, int r) const { return min_rec(H - 1, 0, l, r); }
};
}  // namespace wide_min8

struct Solver {
  wide_min8::Tree t;
  explicit Solver(const vector<i64> &a) : t(a) {}
  void add(int l, int r, i64 x) { t.add(l, r, x); }
  i64 minimum(int l, int r) { return t.minimum(l, r); }
};
