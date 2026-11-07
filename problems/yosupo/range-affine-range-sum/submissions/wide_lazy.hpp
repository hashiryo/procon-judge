#pragma once
#include "pj.hpp"
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <cstdint>
#include <memory>
#ifdef __linux__
#include <sys/mman.h>
#endif

// 子を 16 個持つ遅延セグ木 (Library Checker の首位 nandhagk の 402323 と同じ考え方で、先読みは
// しない)。段 0 は要素を 16 個ずつの塊に並べ、段 h >= 1 の塊は子 (段 h - 1 の塊) 16 個ぶんの
// 和 s と、その子へまだ配っていない作用 (m, a) を並べて持つ。s はその子の作用まで当てた値で、
// 祖先の作用は当てていない。区間の作用は根から 2 本の道を下り、端で一部だけ掛かる子は作用を
// 配ってから下り、戻りに和を 16 個の合計で取り直す。全部掛かる子は、塊の中の連続したレーンへ
// AVX2 でまとめて当てる。区間の和は書き込みをせず、一部だけ掛かる子の結果にその子の作用を
// 当てて足す。値は剰余をそのままの値で 2p 未満に持ち、掛け算は Shoup の方法 (掛ける数 m に
// floor(m 2^32 / p) を前計算) で 8 レーンずつ。区間の長さは段ごとに決まり、最後の塊だけ
// レーンごとの長さの表を使う (要素の無いレーンは長さ 0 で、値は 0 のまま)。
namespace wide_lazy {
constexpr u32 P = 998244353, P2 = 2 * P;

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

// 掛ける数 m (p 未満に直す) と、Shoup の商の近似 floor(m 2^32 / p) を 8 レーンに配ったもの。
struct Mul {
  __m256i m, q;
  explicit Mul(u32 x) {
    x = x >= P ? x - P : x;
    m = _mm256_set1_epi32((int)x);
    q = _mm256_set1_epi32((int)u32((u64(x) << 32) / P));
  }
};
// x m mod p を 2p 未満で。x は u32 なら何でもよい。
inline __m256i mul(__m256i x, const Mul &f) {
  const __m256i e = _mm256_mul_epu32(x, f.q);
  const __m256i o = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), f.q);
  const __m256i q = _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xaa);
  return _mm256_sub_epi32(_mm256_mullo_epi32(x, f.m), _mm256_mullo_epi32(q, _mm256_set1_epi32((int)P)));
}
// a + b を 2p 未満で。a + b < 4p。
inline __m256i add(__m256i a, __m256i b) {
  const __m256i s = _mm256_add_epi32(a, b);
  return _mm256_min_epu32(s, _mm256_sub_epi32(s, _mm256_set1_epi32((int)P2)));
}
inline u32 add1(u32 a, u32 b) {
  const u32 s = a + b;
  return s >= P2 ? s - P2 : s;
}
// 16 レーン (2p 未満) の合計を 2p 未満で。
inline u32 hsum(__m256i x0, __m256i x1) {
  __m256i s = add(x0, x1);
  __m128i t = _mm_add_epi32(_mm256_castsi256_si128(s), _mm256_extracti128_si256(s, 1));
  t = _mm_min_epu32(t, _mm_sub_epi32(t, _mm_set1_epi32((int)P2)));
  t = _mm_add_epi32(t, _mm_shuffle_epi32(t, 0x4e));
  t = _mm_min_epu32(t, _mm_sub_epi32(t, _mm_set1_epi32((int)P2)));
  t = _mm_add_epi32(t, _mm_shuffle_epi32(t, 0xb1));
  t = _mm_min_epu32(t, _mm_sub_epi32(t, _mm_set1_epi32((int)P2)));
  return (u32)_mm_cvtsi128_si32(t);
}
// レーン [lo, hi) を選ぶマスク (16 レーンを 2 本)。
inline void masks(int lo, int hi, __m256i &k0, __m256i &k1) {
  const __m256i i0 = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
  const __m256i i1 = _mm256_setr_epi32(8, 9, 10, 11, 12, 13, 14, 15);
  const __m256i L = _mm256_set1_epi32(lo - 1), R = _mm256_set1_epi32(hi);
  k0 = _mm256_and_si256(_mm256_cmpgt_epi32(i0, L), _mm256_cmpgt_epi32(R, i0));
  k1 = _mm256_and_si256(_mm256_cmpgt_epi32(i1, L), _mm256_cmpgt_epi32(R, i1));
}
inline __m256i ld(const u32 *p) { return _mm256_load_si256((const __m256i *)p); }
inline void st(u32 *p, __m256i x) { _mm256_store_si256((__m256i *)p, x); }

struct alignas(64) Blk {
  u32 s[16], m[16], a[16];
};

class Tree {
  int n = 0, H = 0;
  u32 *v0 = nullptr;
  Blk *lv[8] = {};
  int last[8] = {};
  u32 pw[8] = {};
  alignas(32) u32 tl[8][16] = {};
  HugeArray<u32> mem0;
  HugeArray<Blk> mem1;

  // 段 h の塊 b のレーン [lo, hi) に作用 x -> m x + a を当てる。ALL ならマスクを取らない。
  template <bool ALL> void apply_lanes(int h, int b, int lo, int hi, u32 m, u32 a) {
    const Mul f(m);
    __m256i add0, add1;
    if (b == last[h]) {
      const Mul g(a);
      add0 = mul(ld(tl[h]), g), add1 = mul(ld(tl[h] + 8), g);
    } else {
      add0 = add1 = _mm256_set1_epi32((int)u32(u64(a) * pw[h] % P));
    }
    __m256i k0 = _mm256_setzero_si256(), k1 = k0;
    if constexpr (!ALL) masks(lo, hi, k0, k1);
    auto upd = [&](u32 *p, __m256i x0, __m256i x1) {
      if constexpr (ALL) {
        st(p, x0), st(p + 8, x1);
      } else {
        st(p, _mm256_blendv_epi8(ld(p), x0, k0));
        st(p + 8, _mm256_blendv_epi8(ld(p + 8), x1, k1));
      }
    };
    if (h == 0) {
      u32 *p = v0 + (size_t(b) << 4);
      upd(p, add(mul(ld(p), f), add0), add(mul(ld(p + 8), f), add1));
      return;
    }
    Blk &B = lv[h][b];
    upd(B.s, add(mul(ld(B.s), f), add0), add(mul(ld(B.s + 8), f), add1));
    upd(B.m, mul(ld(B.m), f), mul(ld(B.m + 8), f));
    const __m256i av = _mm256_set1_epi32((int)a);
    upd(B.a, add(mul(ld(B.a), f), av), add(mul(ld(B.a + 8), f), av));
  }
  // 段 h の子 c (段 h - 1 の塊 c) へ作用を配る。
  void push(int h, int c) {
    Blk &B = lv[h][c >> 4];
    const int j = c & 15;
    const u32 m = B.m[j], a = B.a[j];
    if (m == 1 && a == 0) return;
    B.m[j] = 1, B.a[j] = 0;
    apply_lanes<true>(h - 1, c, 0, 16, m, a);
  }
  u32 block_sum(int h, int b) const {
    const u32 *p = h == 0 ? v0 + (size_t(b) << 4) : lv[h][b].s;
    return hsum(ld(p), ld(p + 8));
  }
  u32 lanes_sum(int h, int b, int lo, int hi) const {
    const u32 *p = h == 0 ? v0 + (size_t(b) << 4) : lv[h][b].s;
    __m256i k0, k1;
    masks(lo, hi, k0, k1);
    return hsum(_mm256_and_si256(ld(p), k0), _mm256_and_si256(ld(p + 8), k1));
  }

  void apply_rec(int h, int b, int lo, int hi, u32 m, u32 a) {
    const int sh = 4 * h;
    const int jl = (lo >> sh) & 15, jr = ((hi - 1) >> sh) & 15;
    if (h == 0) return apply_lanes<false>(0, b, jl, jr + 1, m, a);
    const int sm = (1 << sh) - 1, base = b << 4;
    int fl = jl, fr = jr + 1;
    if (lo & sm) {
      const int c = base + jl, e = min(hi, (lo | sm) + 1);
      push(h, c);
      apply_rec(h - 1, c, lo, e, m, a);
      lv[h][b].s[jl] = block_sum(h - 1, c);
      if (jl == jr) return;
      fl = jl + 1;
    }
    if (hi & sm) {
      const int c = base + jr, s = max(lo, hi & ~sm);
      push(h, c);
      apply_rec(h - 1, c, s, hi, m, a);
      lv[h][b].s[jr] = block_sum(h - 1, c);
      fr = jr;
    }
    if (fl < fr) apply_lanes<false>(h, b, fl, fr, m, a);
  }
  // 塊 b の中の [lo, hi) の和。祖先の作用は当てていない値。
  u32 sum_rec(int h, int b, int lo, int hi) const {
    const int sh = 4 * h;
    const int jl = (lo >> sh) & 15, jr = ((hi - 1) >> sh) & 15;
    if (h == 0) return lanes_sum(0, b, jl, jr + 1);
    const int sm = (1 << sh) - 1, base = b << 4;
    const Blk &B = lv[h][b];
    u32 res = 0;
    int fl = jl, fr = jr + 1;
    if (lo & sm) {
      const int c = base + jl, e = min(hi, (lo | sm) + 1);
      const u32 x = sum_rec(h - 1, c, lo, e);
      res = u32((u64(B.m[jl]) * x + u64(B.a[jl]) * u32(e - lo)) % P);
      if (jl == jr) return res;
      fl = jl + 1;
    }
    if (hi & sm) {
      const int c = base + jr, s = max(lo, hi & ~sm);
      const u32 x = sum_rec(h - 1, c, s, hi);
      res = add1(res, u32((u64(B.m[jr]) * x + u64(B.a[jr]) * u32(hi - s)) % P));
      fr = jr;
    }
    if (fl < fr) res = add1(res, lanes_sum(h, b, fl, fr));
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
      if (H > 1) total1 += nb;
      nb = (nb + 15) >> 4;
    }
    if (H > 1) total1 += cnt[H - 1];
    mem0 = HugeArray<u32>(size_t(cnt[0]) << 4);
    mem1 = HugeArray<Blk>(max<size_t>(total1, 1));
    v0 = mem0.p;
    size_t off = 0;
    for (int h = 1; h < H; ++h) lv[h] = mem1.p + off, off += cnt[h];
    for (int h = 0; h < H; ++h) {
      last[h] = cnt[h] - 1;
      pw[h] = u32((u64(1) << (4 * h)) % P);
      for (int j = 0; j < 16; ++j) {
        const i64 start = (i64(last[h]) * 16 + j) << (4 * h);
        tl[h][j] = u32(std::clamp<i64>(n - start, 0, i64(1) << (4 * h)));
      }
    }
    for (int i = 0; i < (cnt[0] << 4); ++i) v0[i] = i < n ? u32(a[i] % P) : 0;
    for (int h = 1; h < H; ++h) {
      for (int b = 0; b < cnt[h]; ++b) {
        Blk &B = lv[h][b];
        for (int j = 0; j < 16; ++j) {
          const int c = b * 16 + j;
          B.s[j] = c < cnt[h - 1] ? block_sum(h - 1, c) : 0;
          B.m[j] = 1, B.a[j] = 0;
        }
      }
    }
  }
  void affine(int l, int r, u32 m, u32 a) { apply_rec(H - 1, 0, l, r, m, a); }
  u32 sum(int l, int r) const {
    const u32 x = sum_rec(H - 1, 0, l, r);
    return x >= P ? x - P : x;
  }
};
}  // namespace wide_lazy

struct Solver {
  wide_lazy::Tree t;
  explicit Solver(const vector<i64> &a) : t(a) {}
  void affine(int l, int r, i64 b, i64 c) { t.affine(l, r, (u32)b, (u32)c); }
  i64 sum(int l, int r) { return t.sum(l, r); }
};
