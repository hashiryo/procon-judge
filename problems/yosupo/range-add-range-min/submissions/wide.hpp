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

// 子を 16 個持つ、作用を配らないセグメント木。段 0 は要素を 16 個ずつの塊に並べ、段 h >= 1 の
// 塊は子 (段 h - 1 の塊) 16 個ぶんの s と t を並べて持つ。t はその子の部分木全体に足した量、
// s はその子の最小に t を足した値。加算は最小と可換なので作用を配らずに済む。区間の加算は根から
// 2 本の道を下り、端で一部だけ掛かる子は下りてから s を子の 16 個の最小 + t で取り直し、全部
// 掛かる子は塊の中の連続したレーンの s と t に AVX2 でまとめて足す。区間の最小は、一部だけ掛かる
// 子の結果に t を足す。値は i64 で、16 個を 4 本の __m256i で扱う。要素の無いレーンは +inf。
namespace wide_min {
constexpr i64 INF = numeric_limits<i64>::max() / 4;

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
inline i64 hmin(__m256i x0, __m256i x1, __m256i x2, __m256i x3) {
  const __m256i m = vmin(vmin(x0, x1), vmin(x2, x3));
  __m128i c = _mm256_castsi256_si128(m);
  const __m128i h = _mm256_extracti128_si256(m, 1);
  c = _mm_blendv_epi8(c, h, _mm_cmpgt_epi64(c, h));
  const __m128i s = _mm_unpackhi_epi64(c, c);
  c = _mm_blendv_epi8(c, s, _mm_cmpgt_epi64(c, s));
  return _mm_cvtsi128_si64(c);
}
// レーン [lo, hi) を選ぶマスク (16 レーンを 4 本)。gcc は __m256i の配列をスタックへ退避するので、
// 別々の変数で返し、レーンの番号は定数のベクトルにする。
inline void masks(int lo, int hi, __m256i &k0, __m256i &k1, __m256i &k2, __m256i &k3) {
  const __m256i L = _mm256_set1_epi64x(lo - 1), R = _mm256_set1_epi64x(hi);
  const __m256i i0 = _mm256_setr_epi64x(0, 1, 2, 3), i1 = _mm256_setr_epi64x(4, 5, 6, 7);
  const __m256i i2 = _mm256_setr_epi64x(8, 9, 10, 11), i3 = _mm256_setr_epi64x(12, 13, 14, 15);
  k0 = _mm256_and_si256(_mm256_cmpgt_epi64(i0, L), _mm256_cmpgt_epi64(R, i0));
  k1 = _mm256_and_si256(_mm256_cmpgt_epi64(i1, L), _mm256_cmpgt_epi64(R, i1));
  k2 = _mm256_and_si256(_mm256_cmpgt_epi64(i2, L), _mm256_cmpgt_epi64(R, i2));
  k3 = _mm256_and_si256(_mm256_cmpgt_epi64(i3, L), _mm256_cmpgt_epi64(R, i3));
}

struct alignas(64) Blk {
  i64 s[16], t[16];
};

class Tree {
  int n = 0, H = 0;
  i64 *v0 = nullptr;
  Blk *lv[8] = {};
  HugeArray<i64> mem0;
  HugeArray<Blk> mem1;

  void add_lanes(int h, int b, int lo, int hi, i64 x) {
    __m256i k0, k1, k2, k3;
    masks(lo, hi, k0, k1, k2, k3);
    const __m256i xv = _mm256_set1_epi64x(x);
    const __m256i x0 = _mm256_and_si256(k0, xv), x1 = _mm256_and_si256(k1, xv);
    const __m256i x2 = _mm256_and_si256(k2, xv), x3 = _mm256_and_si256(k3, xv);
    auto upd = [&](i64 *p) {
      st(p, _mm256_add_epi64(ld(p), x0)), st(p + 4, _mm256_add_epi64(ld(p + 4), x1));
      st(p + 8, _mm256_add_epi64(ld(p + 8), x2)), st(p + 12, _mm256_add_epi64(ld(p + 12), x3));
    };
    if (h == 0) return upd(v0 + (size_t(b) << 4));
    upd(lv[h][b].s), upd(lv[h][b].t);
  }
  const i64 *row(int h, int b) const { return h == 0 ? v0 + (size_t(b) << 4) : lv[h][b].s; }
  i64 block_min(int h, int b) const {
    const i64 *p = row(h, b);
    return hmin(ld(p), ld(p + 4), ld(p + 8), ld(p + 12));
  }
  i64 lanes_min(int h, int b, int lo, int hi) const {
    const i64 *p = row(h, b);
    __m256i k0, k1, k2, k3;
    masks(lo, hi, k0, k1, k2, k3);
    const __m256i inf = _mm256_set1_epi64x(INF);
    return hmin(_mm256_blendv_epi8(inf, ld(p), k0), _mm256_blendv_epi8(inf, ld(p + 4), k1),
                _mm256_blendv_epi8(inf, ld(p + 8), k2), _mm256_blendv_epi8(inf, ld(p + 12), k3));
  }
  void add_rec(int h, int b, int lo, int hi, i64 x) {
    const int sh = 4 * h;
    const int jl = (lo >> sh) & 15, jr = ((hi - 1) >> sh) & 15;
    if (h == 0) return add_lanes(0, b, jl, jr + 1, x);
    const int sm = (1 << sh) - 1, base = b << 4;
    Blk &B = lv[h][b];
    int fl = jl, fr = jr + 1;
    if (lo & sm) {
      const int c = base + jl, e = min(hi, (lo | sm) + 1);
      add_rec(h - 1, c, lo, e, x);
      B.s[jl] = block_min(h - 1, c) + B.t[jl];
      if (jl == jr) return;
      fl = jl + 1;
    }
    if (hi & sm) {
      const int c = base + jr, s = max(lo, hi & ~sm);
      add_rec(h - 1, c, s, hi, x);
      B.s[jr] = block_min(h - 1, c) + B.t[jr];
      fr = jr;
    }
    if (fl < fr) add_lanes(h, b, fl, fr, x);
  }
  i64 min_rec(int h, int b, int lo, int hi) const {
    const int sh = 4 * h;
    const int jl = (lo >> sh) & 15, jr = ((hi - 1) >> sh) & 15;
    if (h == 0) return lanes_min(0, b, jl, jr + 1);
    const int sm = (1 << sh) - 1, base = b << 4;
    const Blk &B = lv[h][b];
    i64 res = INF;
    int fl = jl, fr = jr + 1;
    if (lo & sm) {
      const int c = base + jl, e = min(hi, (lo | sm) + 1);
      res = min_rec(h - 1, c, lo, e) + B.t[jl];
      if (jl == jr) return res;
      fl = jl + 1;
    }
    if (hi & sm) {
      const int c = base + jr, s = max(lo, hi & ~sm);
      res = min(res, min_rec(h - 1, c, s, hi) + B.t[jr]);
      fr = jr;
    }
    if (fl < fr) res = min(res, lanes_min(h, b, fl, fr));
    return res;
  }

public:
  explicit Tree(const vector<i64> &a) : n((int)a.size()) {
    int cnt[8], nb = (n + 15) >> 4;
    H = 0;
    size_t total1 = 0;
    for (;;) {
      cnt[H++] = nb;
      if (nb == 1) break;
      nb = (nb + 15) >> 4;
      total1 += nb;
    }
    mem0 = HugeArray<i64>(size_t(cnt[0]) << 4);
    mem1 = HugeArray<Blk>(max<size_t>(total1, 1));
    v0 = mem0.p;
    size_t off = 0;
    for (int h = 1; h < H; ++h) lv[h] = mem1.p + off, off += cnt[h];
    for (int i = 0; i < (cnt[0] << 4); ++i) v0[i] = i < n ? a[i] : INF;
    for (int h = 1; h < H; ++h)
      for (int b = 0; b < cnt[h]; ++b)
        for (int j = 0; j < 16; ++j) {
          const int c = b * 16 + j;
          lv[h][b].s[j] = c < cnt[h - 1] ? block_min(h - 1, c) : INF;
          lv[h][b].t[j] = 0;
        }
  }
  void add(int l, int r, i64 x) { add_rec(H - 1, 0, l, r, x); }
  i64 minimum(int l, int r) const { return min_rec(H - 1, 0, l, r); }
};
}  // namespace wide_min

struct Solver {
  wide_min::Tree t;
  explicit Solver(const vector<i64> &a) : t(a) {}
  void add(int l, int r, i64 x) { t.add(l, r, x); }
  i64 minimum(int l, int r) { return t.minimum(l, r); }
};
