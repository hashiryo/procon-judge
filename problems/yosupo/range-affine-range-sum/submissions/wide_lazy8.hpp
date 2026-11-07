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

// wide_lazy の子を 8 個にした形。塊の 8 レーンが __m256i の 1 本に収まる。段 h >= 1 では、子 8 個
// ぶんの和 s (32 byte) と、まだ配っていない作用 (m, a) (64 byte、キャッシュの 1 行) を別の配列に
// 持つ。区間の和は s だけを読む。作りは wide_lazy と同じで、値は剰余をそのままの値で 2p 未満に
// 持ち、掛け算は Shoup の方法。最後の塊だけはレーンごとの長さを使う。
namespace wide_lazy8 {
constexpr u32 P = 998244353, P2 = 2 * P;
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

struct Mul {
  __m256i m, q;
  explicit Mul(u32 x) {
    x = x >= P ? x - P : x;
    m = _mm256_set1_epi32((int)x);
    q = _mm256_set1_epi32((int)u32((u64(x) << 32) / P));
  }
};
inline __m256i mul(__m256i x, const Mul &f) {
  const __m256i e = _mm256_mul_epu32(x, f.q);
  const __m256i o = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), f.q);
  const __m256i q = _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xaa);
  return _mm256_sub_epi32(_mm256_mullo_epi32(x, f.m), _mm256_mullo_epi32(q, _mm256_set1_epi32((int)P)));
}
inline __m256i add(__m256i a, __m256i b) {
  const __m256i s = _mm256_add_epi32(a, b);
  return _mm256_min_epu32(s, _mm256_sub_epi32(s, _mm256_set1_epi32((int)P2)));
}
inline u32 add1(u32 a, u32 b) {
  const u32 s = a + b;
  return s >= P2 ? s - P2 : s;
}
inline u32 hsum(__m256i s) {
  __m128i t = _mm_add_epi32(_mm256_castsi256_si128(s), _mm256_extracti128_si256(s, 1));
  t = _mm_min_epu32(t, _mm_sub_epi32(t, _mm_set1_epi32((int)P2)));
  t = _mm_add_epi32(t, _mm_shuffle_epi32(t, 0x4e));
  t = _mm_min_epu32(t, _mm_sub_epi32(t, _mm_set1_epi32((int)P2)));
  t = _mm_add_epi32(t, _mm_shuffle_epi32(t, 0xb1));
  t = _mm_min_epu32(t, _mm_sub_epi32(t, _mm_set1_epi32((int)P2)));
  return (u32)_mm_cvtsi128_si32(t);
}
inline __m256i mask(int lo, int hi) {
  const __m256i i0 = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
  return _mm256_and_si256(_mm256_cmpgt_epi32(i0, _mm256_set1_epi32(lo - 1)), _mm256_cmpgt_epi32(_mm256_set1_epi32(hi), i0));
}
inline __m256i ld(const u32 *p) { return _mm256_load_si256((const __m256i *)p); }
inline void st(u32 *p, __m256i x) { _mm256_store_si256((__m256i *)p, x); }

struct alignas(64) Tag {
  u32 m[B], a[B];
};

class Tree {
  int n = 0, H = 0;
  u32 *S[12] = {};  // S[0] は要素、S[h] (h >= 1) は子の和
  Tag *T[12] = {};
  int last[12] = {};
  u32 pw[12] = {};
  alignas(32) u32 tl[12][B] = {};
  HugeArray<u32> memS;
  HugeArray<Tag> memT;

  template <bool ALL> void apply_lanes(int h, int b, int lo, int hi, u32 m, u32 a) {
    const Mul f(m);
    __m256i ad;
    if (b == last[h]) ad = mul(ld(tl[h]), Mul(a));
    else ad = _mm256_set1_epi32((int)u32(u64(a) * pw[h] % P));
    __m256i k = _mm256_setzero_si256();
    if constexpr (!ALL) k = mask(lo, hi);
    auto upd = [&](u32 *p, __m256i x) {
      if constexpr (ALL) st(p, x);
      else st(p, _mm256_blendv_epi8(ld(p), x, k));
    };
    u32 *s = S[h] + (size_t(b) << LB);
    upd(s, add(mul(ld(s), f), ad));
    if (h == 0) return;
    Tag &G = T[h][b];
    upd(G.m, mul(ld(G.m), f));
    upd(G.a, add(mul(ld(G.a), f), _mm256_set1_epi32((int)a)));
  }
  void push(int h, int c) {
    Tag &G = T[h][c >> LB];
    const int j = c & (B - 1);
    const u32 m = G.m[j], a = G.a[j];
    if (m == 1 && a == 0) return;
    G.m[j] = 1, G.a[j] = 0;
    apply_lanes<true>(h - 1, c, 0, B, m, a);
  }
  u32 block_sum(int h, int b) const { return hsum(ld(S[h] + (size_t(b) << LB))); }
  u32 lanes_sum(int h, int b, int lo, int hi) const {
    return hsum(_mm256_and_si256(ld(S[h] + (size_t(b) << LB)), mask(lo, hi)));
  }

  void apply_rec(int h, int b, int lo, int hi, u32 m, u32 a) {
    const int sh = LB * h;
    const int jl = (lo >> sh) & (B - 1), jr = ((hi - 1) >> sh) & (B - 1);
    if (h == 0) return apply_lanes<false>(0, b, jl, jr + 1, m, a);
    const int sm = (1 << sh) - 1, base = b << LB;
    int fl = jl, fr = jr + 1;
    if (lo & sm) {
      const int c = base + jl, e = min(hi, (lo | sm) + 1);
      push(h, c);
      apply_rec(h - 1, c, lo, e, m, a);
      S[h][c] = block_sum(h - 1, c);
      if (jl == jr) return;
      fl = jl + 1;
    }
    if (hi & sm) {
      const int c = base + jr, s = max(lo, hi & ~sm);
      push(h, c);
      apply_rec(h - 1, c, s, hi, m, a);
      S[h][c] = block_sum(h - 1, c);
      fr = jr;
    }
    if (fl < fr) apply_lanes<false>(h, b, fl, fr, m, a);
  }
  u32 sum_rec(int h, int b, int lo, int hi) const {
    const int sh = LB * h;
    const int jl = (lo >> sh) & (B - 1), jr = ((hi - 1) >> sh) & (B - 1);
    if (h == 0) return lanes_sum(0, b, jl, jr + 1);
    const int sm = (1 << sh) - 1, base = b << LB;
    const Tag &G = T[h][b];
    u32 res = 0;
    int fl = jl, fr = jr + 1;
    if (lo & sm) {
      const int c = base + jl, e = min(hi, (lo | sm) + 1);
      const u32 x = sum_rec(h - 1, c, lo, e);
      res = u32((u64(G.m[jl]) * x + u64(G.a[jl]) * u32(e - lo)) % P);
      if (jl == jr) return res;
      fl = jl + 1;
    }
    if (hi & sm) {
      const int c = base + jr, s = max(lo, hi & ~sm);
      const u32 x = sum_rec(h - 1, c, s, hi);
      res = add1(res, u32((u64(G.m[jr]) * x + u64(G.a[jr]) * u32(hi - s)) % P));
      fr = jr;
    }
    if (fl < fr) res = add1(res, lanes_sum(h, b, fl, fr));
    return res;
  }

public:
  explicit Tree(const vector<i64> &a) : n((int)a.size()) {
    int cnt[12], nb = (n + B - 1) >> LB;
    H = 0;
    size_t totS = 0, totT = 0;
    for (;;) {
      cnt[H] = nb;
      totS += size_t(nb) << LB;
      if (H > 0) totT += nb;
      ++H;
      if (nb == 1) break;
      nb = (nb + B - 1) >> LB;
    }
    memS = HugeArray<u32>(totS);
    memT = HugeArray<Tag>(max<size_t>(totT, 1));
    size_t oS = 0, oT = 0;
    for (int h = 0; h < H; ++h) {
      S[h] = memS.p + oS, oS += size_t(cnt[h]) << LB;
      if (h > 0) T[h] = memT.p + oT, oT += cnt[h];
      last[h] = cnt[h] - 1;
      pw[h] = u32((u64(1) << (LB * h)) % P);
      for (int j = 0; j < B; ++j) {
        const i64 start = (i64(last[h]) * B + j) << (LB * h);
        tl[h][j] = u32(std::clamp<i64>(n - start, 0, i64(1) << (LB * h)));
      }
    }
    for (int i = 0; i < (cnt[0] << LB); ++i) S[0][i] = i < n ? u32(a[i] % P) : 0;
    for (int h = 1; h < H; ++h)
      for (int c = 0; c < (cnt[h] << LB); ++c) {
        S[h][c] = c < cnt[h - 1] ? block_sum(h - 1, c) : 0;
        T[h][c >> LB].m[c & (B - 1)] = 1, T[h][c >> LB].a[c & (B - 1)] = 0;
      }
  }
  void affine(int l, int r, u32 m, u32 a) { apply_rec(H - 1, 0, l, r, m, a); }
  u32 sum(int l, int r) const {
    const u32 x = sum_rec(H - 1, 0, l, r);
    return x >= P ? x - P : x;
  }
};
}  // namespace wide_lazy8

struct Solver {
  wide_lazy8::Tree t;
  explicit Solver(const vector<i64> &a) : t(a) {}
  void affine(int l, int r, i64 b, i64 c) { t.affine(l, r, (u32)b, (u32)c); }
  i64 sum(int l, int r) { return t.sum(l, r); }
};
