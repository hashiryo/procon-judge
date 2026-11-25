#pragma once
// Source: Library Checker "Convolution" (convolution_mod) の提出 406408 (Aiyiyi)、
//   https://judge.yosupo.jp/submission/406408
//   2026-11-25 の API の一覧で 0.017 s (AVX2、AVX-512 なし)。flip のカーネル
//   (radix-4 再帰、Montgomery、256 の区画、direct8 の葉)。元のソースは
//   work/ntt/uop_explore/make_yosupo.py の生成物。I/O は QgQ さんの提出 393435 から。
//   ライセンスの表示は無い。出典を残し、変更は最小限にする。
// self/ntt/convolution への移植: 全体の積を求めて [lo, hi) を切り出す (範囲専用の最適化はしない)。
//   配列は self の最大 (2^20 + 2^20 - 1 -> 2^21) に合わせ、元の main の 16 語の詰め物を残す。
//   カーネルは x86 の asm ("+x" の障壁) と AVX2 の intrinsics を含むので x64 だけで組める。
//   SIMDe では組めないため、arm は理由の分かるコンパイルエラーにする。
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

#if !defined(__x86_64__)
#error "yosupo_406408_aiyiyi は x86-64 のインライン asm と AVX2 を含む。arm (SIMDe) では組めない。"
#endif

#if defined(__clang__)
#pragma clang attribute push(__attribute__((target("avx2"))), apply_to = function)
#define PJ_CLANG_TARGET_PUSHED 1
#else
#pragma GCC optimize("O3,unroll-loops")
#pragma GCC target("avx2,bmi")
#endif
#include <immintrin.h>

namespace qflip {
using U = uint32_t; using W = uint64_t; using V = __m256i;
constexpr U P = 998244353, P2 = 2 * P, NI = 998244351, R2 = 932051910;  // NI = -P^-1 mod 2^32, R2 = 2^64 mod P
static_assert(W(4) * P < (W(1) << 32));
static_assert(U(P * NI) == U(-1));
#define QF_AI __attribute__((always_inline)) inline

constexpr U muls(U a, U b) { W x = W(a) * b; U c = U(x) * NI; U z = U((x + W(c) * P) >> 32); return z >= P ? z - P : z; }
constexpr U power(U a, U e) { U r = 1; for (; e; e >>= 1, a = U(W(a) * a % P)) if (e & 1) r = U(W(r) * a % P); return r; }
constexpr U mont(U a) { return muls(a, R2); }
constexpr U ONE = mont(1);

QF_AI V splat(U x) { return _mm256_set1_epi32(int(x)); }
QF_AI V plus(V x, V y) { return _mm256_add_epi32(x, y); }
QF_AI V minus(V x, V y) { return _mm256_sub_epi32(x, y); }
QF_AI V shrink(V x, U p) { return _mm256_min_epu32(x, minus(x, splat(p))); }
QF_AI V low(V x) { return shrink(x, P2); }
QF_AI V canonical(V x) { return shrink(low(x), P); }
QF_AI V diff(V x, V y) { return minus(plus(x, splat(P2)), y); }
QF_AI V odd(V x) { return _mm256_srli_epi64(x, 32); }
QF_AI V sigma(V x) { return _mm256_shuffle_epi32(x, 0xD8); }  // lanes 1<->2 in each half
// Empty asm: materializes x so GCC cannot re-associate x into neighbouring
// additions (it turned A +/- (xw - qP) into 5 ops instead of 4 for Shoup).
QF_AI V opaque(V x) { asm("" : "+x"(x)); return x; }
template<bool Shuf> QF_AI V odd_of(V x) { if constexpr (Shuf) return _mm256_shuffle_epi32(x, 0xF5); else return odd(x); }

// e,o: 64-bit sums whose high words are the results of lanes (0,2,4,6)/(1,3,5,7).
template<bool Flip> QF_AI V combine(V e, V o) {
    if constexpr (Flip) return _mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(e), _mm256_castsi256_ps(o), 0xDD));
    else return _mm256_blend_epi32(odd(e), o, 0xAA);
}
// Montgomery REDC of two 64-bit accumulator vectors (sum < 2^64 - 2^32*P).
template<bool Flip> QF_AI V reduce(V e, V o) {
    const V m = splat(NI), p = splat(P);
    e = _mm256_add_epi64(e, _mm256_mul_epu32(_mm256_mul_epu32(e, m), p));
    o = _mm256_add_epi64(o, _mm256_mul_epu32(_mm256_mul_epu32(o, m), p));
    return combine<Flip>(e, o);
}

struct Fixed {
    // Montgomery (Mul 0/1): w < P in Montgomery form, wi = w*NI mod 2^32.
    // Shoup (Mul 2): w < P in normal form, wi = floor(w*2^32/P) = mont(w)*NI mod 2^32.
    V w, wi;
    QF_AI Fixed() : w(), wi() {}
    QF_AI Fixed(V w_, V wi_) : w(w_), wi(wi_) {}
    QF_AI static Fixed scalar(U x) { return Fixed(splat(x), splat(x * NI)); }
    QF_AI static Fixed vec(V x) { return Fixed(x, _mm256_mullo_epi32(x, splat(NI))); }
    // x < 2^32 -> x*(twiddle value) mod P, in [0,2P). Mul: 0 Montgomery with
    // vpmuludq quotients, 1 Montgomery with one vpmulld quotient, 2 Shoup.
    template<int Mul, bool Flip, bool Shuf = false, bool Opq = false> QF_AI V mul(V x) const {
        return mul2<Mul, Flip, Opq>(x, odd_of<Shuf>(x));
    }
    // xo: any vector whose even lanes hold the odd lanes of x (e.g. an unaligned
    // load of the same data one element later).
    template<int Mul, bool Flip, bool Opq = false> QF_AI V mul2(V x, V xo) const {
        const V p = splat(P);
        V e, o;
        if constexpr (Mul == 2) {
            static_assert(!Flip, "Shoup results are already packed");
            V q = _mm256_blend_epi32(odd(_mm256_mul_epu32(x, wi)), _mm256_mul_epu32(xo, wi), 0xAA);
            V r = _mm256_sub_epi32(_mm256_mullo_epi32(x, w), _mm256_mullo_epi32(q, p));
            return Opq ? opaque(r) : r;
        } else if constexpr (Mul == 1) {
            V q = _mm256_mullo_epi32(x, wi);
            e = _mm256_add_epi64(_mm256_mul_epu32(x, w), _mm256_mul_epu32(q, p));
            o = _mm256_add_epi64(_mm256_mul_epu32(xo, w), _mm256_mul_epu32(odd(q), p));
        } else {
            e = _mm256_add_epi64(_mm256_mul_epu32(x, w), _mm256_mul_epu32(_mm256_mul_epu32(x, wi), p));
            o = _mm256_add_epi64(_mm256_mul_epu32(xo, w), _mm256_mul_epu32(_mm256_mul_epu32(xo, wi), p));
        }
        return combine<Flip>(e, o);
    }
};

// q[j] = 3^((P-1)/2^(j+2)) (Montgomery form); r[k] = prod q[b] over set bits b of k.
struct Constants {
    U q[22]{}, iq[22]{}, even_step[21]{};
    alignas(32) U perm_idx[2][8]{};
    constexpr Constants() {
        q[21] = mont(power(3, (P - 1) >> 23)); iq[21] = mont(power(power(3, (P - 1) >> 23), P - 2));
        for (int j = 20; j >= 0; --j) { q[j] = muls(q[j + 1], q[j + 1]); iq[j] = muls(iq[j + 1], iq[j + 1]); }
        U c = ONE;
        for (int j = 0; j < 21; ++j) { even_step[j] = muls(q[j + 1], c); c = muls(c, iq[j + 1]); }
        for (int l = 0; l < 8; ++l) { perm_idx[0][l] = U(l); perm_idx[1][l] = U((l & 4) | ((l & 1) << 1) | ((l >> 1) & 1)); }
    }
};
inline constexpr Constants constants{};

// Leaf: 0 plain (unroll 2); 1 odd-lane reuse, peeled first step + unroll 1; 2 reuse, full unroll;
// 3 register reuse of the previous window load, one leaf at a time; 4/5 the same
// with 2/4 leaves interleaved; 6/7/8 windows built in registers (1/2/4 leaves).
// Shuf: odd-lane extraction with vpshufd (shuffle port) instead of vpsrlq.
// Mul: 0 Montgomery, 1 Montgomery+vpmulld quotient, 2 Shoup (needs Pair, no Flip).
// Leaf accumulators are always reduced with Montgomery.
// With Shoup, the leaf's w*a multiply and the final scale also use Shoup.
// Aux=false keeps Montgomery (vpmulld) for those two multiplies even with Shoup.
// Opq: opaque() barrier on multiply results. LdOdd: odd lanes of the forward
// butterfly's loaded c,d inputs come from unaligned loads (needs 4 readable
// bytes past the end of the array).
template<int Mul_, bool Flip_, bool Pair_, int Leaf_, bool Shuf_ = false, int Tile_ = 256, bool Aux_ = true,
         bool Opq_ = false, bool LdOdd_ = false, int Il_ = 1, bool Blk_ = false, bool Pipe_ = false>
struct Cfg {
    // Pipe: bottom stage builds the next batch's leaf windows before this batch's
    // multiply-accumulate, so the unaligned window loads read committed stores.
    static constexpr bool Pipe = Pipe_;
    // Blk (Shoup only): table in blocks of 8 values then their 8 quotients, built
    // with 8-lane Shoup products instead of 4-pair Montgomery products.
    static constexpr bool Opq = Opq_, LdOdd = LdOdd_, Blk = Blk_;
    static_assert(!Blk_ || Mul_ == 2);
    static constexpr int Il = Il_;   // butterflies per loop iteration (2: interleaved pair)
    static constexpr int Mul = Mul_, MontMul = Mul_ == 0 ? 0 : 1, Leaf = Leaf_, Tile = Tile_;
    static constexpr bool ShoupAux = Mul_ == 2 && Aux_;
    static constexpr int LeafMul = ShoupAux ? 2 : MontMul;
    static constexpr bool Flip = Flip_, Pair = Pair_, Shuf = Shuf_;
    static_assert(Mul != 2 || (Pair && !Flip));
};

struct Twiddle { Fixed x, y, z; };

// Forward Cooley-Tukey radix-4 on f[j + t*h], t=0..3, j<h (inputs/outputs < 4P):
//   A=a+x*c, C=a-x*c, B=b+x*d, D=b-x*d; out = A+y*B, A-y*B, C+z*D, C-z*D.
// Flip layout: perms a:p b:~p c:~p d:p -> all outputs p.
template<class C, bool Identity>
QF_AI void fwd4(V* f, int h, const Twiddle& t) {
    constexpr int M = C::Mul; constexpr bool F = C::Flip, S = C::Shuf, O = C::Opq;
    auto body = [&](int j) __attribute__((always_inline)) {
        V a = low(f[j]), b = low(f[j + h]), c = f[j + 2 * h], d = f[j + 3 * h];
        if constexpr (Identity) {
            c = low(c); d = low(d);
            if constexpr (F) { c = sigma(c); d = sigma(d); }
        } else if constexpr (C::LdOdd) {
            const U* fc = (const U*)(f + j + 2 * h);
            c = t.x.mul2<M, F, O>(c, _mm256_loadu_si256((const V*)(fc + 1)));
            d = t.x.mul2<M, F, O>(d, _mm256_loadu_si256((const V*)(fc + 8 * h + 1)));
        } else { c = t.x.mul<M, F, S, O>(c); d = t.x.mul<M, F, S, O>(d); }
        V ac = low(plus(a, c)), amc = low(diff(a, c));
        V bd = plus(b, d), bmd = diff(b, d);
        if constexpr (Identity) { bd = low(bd); if constexpr (F) bd = sigma(bd); }
        else bd = t.y.mul<M, F, S, O>(bd);
        bmd = t.z.mul<M, F, S, O>(bmd);
        f[j] = plus(ac, bd); f[j + h] = diff(ac, bd); f[j + 2 * h] = plus(amc, bmd); f[j + 3 * h] = diff(amc, bmd);
    };
    if (C::Il == 2 && h >= 2) for (int j = 0; j < h; j += 2) { body(j); body(j + 1); }
    else for (int j = 0; j < h; ++j) body(j);
}
// Inverse Gentleman-Sande radix-4 (inputs/outputs < 2P).
// Flip layout: inputs all q -> outputs q,~q,~q,q.
template<class C, bool Identity>
QF_AI void inv4(V* f, int h, const Twiddle& t) {
    constexpr int M = C::Mul; constexpr bool F = C::Flip, S = C::Shuf, O = C::Opq;
    auto body = [&](int j) __attribute__((always_inline)) {
        V a = f[j], b = f[j + h], c = f[j + 2 * h], d = f[j + 3 * h];
        V ab = low(plus(a, b)), cd = low(plus(c, d)), amb = diff(a, b), cmd = diff(c, d);
        if constexpr (Identity) { amb = low(amb); if constexpr (F) amb = sigma(amb); }
        else amb = t.y.mul<M, F, S, O>(amb);
        cmd = t.z.mul<M, F, S, O>(cmd);
        V o0 = low(plus(ab, cd)), o1 = low(plus(amb, cmd)), o2 = diff(ab, cd), o3 = diff(amb, cmd);
        if constexpr (Identity) {
            o2 = low(o2); o3 = low(o3);
            if constexpr (F) { o2 = sigma(o2); o3 = sigma(o3); }
        } else { o2 = t.x.mul<M, F, S, O>(o2); o3 = t.x.mul<M, F, S, O>(o3); }
        f[j] = o0; f[j + h] = o1; f[j + 2 * h] = o2; f[j + 3 * h] = o3;
    };
    if (C::Il == 2 && h >= 2) for (int j = 0; j < h; j += 2) { body(j); body(j + 1); }
    else for (int j = 0; j < h; ++j) body(j);
}

// Direct product of 4 leaves: a[t] = a[t]*b[t] mod (x^8 - w[t]) (times R^-1 scale
// handled by the final constant). Inputs < 4P, natural lane order; output < 2P with
// permutation sigma when Flip. Sum bound 8(P-1)^2 + (2^32-1)P < 2^64.
struct LeafBuf { alignas(32) U window[4][16]; alignas(32) U coeff[4][8]; };
// Stores the windows [w*a, a] and canonical b coefficients of four leaves.
template<class C>
QF_AI void leaf_build(const V* a, const V* b, const U* weights, const U* weights_ni, LeafBuf& L) {
    for (int t = 0; t < 4; ++t) {
        V x = canonical(a[t]);
        const Fixed wt = weights_ni ? Fixed(splat(weights[t]), splat(weights_ni[t])) : Fixed::scalar(weights[t]);
        _mm256_store_si256((V*)L.window[t], shrink(wt.mul<C::LeafMul, false, C::Shuf, C::Opq>(x), P));
        _mm256_store_si256((V*)(L.window[t] + 8), x);
        _mm256_store_si256((V*)L.coeff[t], canonical(b[t]));
    }
}
// Multiply-accumulate from a built buffer; a[t] = product, < 2P.
template<class C>
QF_AI void leaf_mac(V* a, const LeafBuf& L) {
    const auto& window = L.window; const auto& coeff = L.coeff;
    if constexpr (C::Leaf == 4 || C::Leaf == 5) {
        // Register reuse as in Leaf 3, with G leaves interleaved for parallelism.
        constexpr int G = C::Leaf == 4 ? 2 : 4;
        for (int t0 = 0; t0 < 4; t0 += G) {
            V e[G], o[G], prev[G];
            for (int g = 0; g < G; ++g) {
                prev[g] = _mm256_loadu_si256((const V*)(window[t0 + g] + 8));
                V y = splat(coeff[t0 + g][0]);
                e[g] = _mm256_mul_epu32(prev[g], y); o[g] = _mm256_mul_epu32(odd(prev[g]), y);
            }
#pragma GCC unroll 7
            for (int i = 1; i < 8; ++i)
                for (int g = 0; g < G; ++g) {
                    V x = _mm256_loadu_si256((const V*)(window[t0 + g] + 8 - i)), y = splat(coeff[t0 + g][i]);
                    e[g] = _mm256_add_epi64(e[g], _mm256_mul_epu32(x, y));
                    o[g] = _mm256_add_epi64(o[g], _mm256_mul_epu32(prev[g], y));
                    prev[g] = x;
                }
            for (int g = 0; g < G; ++g) a[t0 + g] = low(reduce<C::Flip>(e[g], o[g]));
        }
        return;
    }
    if constexpr (C::Leaf == 3) {
        // One leaf at a time; the odd lanes of window load i are the even lanes of
        // window load i-1, which is still in a register (no shift, no extra load).
        for (int t = 0; t < 4; ++t) {
            V prev = _mm256_loadu_si256((const V*)(window[t] + 8)), y = splat(coeff[t][0]);
            V e = _mm256_mul_epu32(prev, y), o = _mm256_mul_epu32(odd(prev), y);
#pragma GCC unroll 7
            for (int i = 1; i < 8; ++i) {
                V x = _mm256_loadu_si256((const V*)(window[t] + 8 - i));
                y = splat(coeff[t][i]);
                e = _mm256_add_epi64(e, _mm256_mul_epu32(x, y));
                o = _mm256_add_epi64(o, _mm256_mul_epu32(prev, y));
                prev = x;
            }
            a[t] = low(reduce<C::Flip>(e, o));
        }
        return;
    }
    V e[4], o[4];
    for (int t = 0; t < 4; ++t) e[t] = o[t] = _mm256_setzero_si256();
    auto step = [&](int i, auto reuse) __attribute__((always_inline)) {
        for (int t = 0; t < 4; ++t) {
            V x = _mm256_loadu_si256((const V*)(window[t] + 8 - i)), y = splat(coeff[t][i]);
            V xo;
            // Even lanes of the load at offset 9-i are the odd lanes of x (i>0).
            if constexpr (decltype(reuse)::value) xo = _mm256_loadu_si256((const V*)(window[t] + 9 - i));
            else xo = odd_of<C::Shuf>(x);
            e[t] = _mm256_add_epi64(e[t], _mm256_mul_epu32(x, y));
            o[t] = _mm256_add_epi64(o[t], _mm256_mul_epu32(xo, y));
        }
    };
    if constexpr (C::Leaf == 1) {
        step(0, std::false_type{});
#pragma GCC unroll 1
        for (int i = 1; i < 8; ++i) step(i, std::true_type{});
    } else if constexpr (C::Leaf == 2) {
        step(0, std::false_type{});
#pragma GCC unroll 8
        for (int i = 1; i < 8; ++i) step(i, std::true_type{});
    } else {
#pragma GCC unroll 2
        for (int i = 0; i < 8; ++i) step(i, std::false_type{});
    }
    for (int t = 0; t < 4; ++t) a[t] = low(reduce<C::Flip>(e[t], o[t]));
}
// Leaf 6/7/8: windows built in registers. With Lw = w*a and H = a (canonical),
// T = [Lw_hi | H_lo] is window 4; windows 1-3 are alignr(H, T, 4(4-i)) and 5-7 are
// alignr(T, Lw, 4(8-i)) (in-lane byte shifts). Window i-1 supplies the odd lanes of
// window i. No window stores or unaligned loads. G leaves are interleaved.
template<class C>
QF_AI void leaf4_reg(V* a, V* b, const U* weights, const U* weights_ni) {
    constexpr int G = C::Leaf == 6 ? 1 : C::Leaf == 7 ? 2 : 4;
    alignas(32) U coeff[4][8];
    for (int t = 0; t < 4; ++t) _mm256_store_si256((V*)coeff[t], canonical(b[t]));
    for (int t0 = 0; t0 < 4; t0 += G) {
        V H[G], Lw[G], T[G], e[G], o[G], prev[G];
        for (int g = 0; g < G; ++g) {
            const int t = t0 + g;
            H[g] = canonical(a[t]);
            const Fixed wt = weights_ni ? Fixed(splat(weights[t]), splat(weights_ni[t])) : Fixed::scalar(weights[t]);
            Lw[g] = shrink(wt.mul<C::LeafMul, false, C::Shuf, C::Opq>(H[g]), P);
            T[g] = _mm256_permute2x128_si256(Lw[g], H[g], 0x21);
            const V y = splat(coeff[t][0]);
            e[g] = _mm256_mul_epu32(H[g], y); o[g] = _mm256_mul_epu32(odd(H[g]), y); prev[g] = H[g];
        }
        auto step = [&](auto ic) __attribute__((always_inline)) {
            constexpr int i = decltype(ic)::value;
            for (int g = 0; g < G; ++g) {
                V w;
                if constexpr (i == 4) w = T[g];
                else if constexpr (i < 4) w = _mm256_alignr_epi8(H[g], T[g], 4 * (4 - i));
                else w = _mm256_alignr_epi8(T[g], Lw[g], 4 * (8 - i));
                const V y = splat(coeff[t0 + g][i]);
                e[g] = _mm256_add_epi64(e[g], _mm256_mul_epu32(w, y));
                o[g] = _mm256_add_epi64(o[g], _mm256_mul_epu32(prev[g], y));
                prev[g] = w;
            }
        };
        step(std::integral_constant<int, 1>{}); step(std::integral_constant<int, 2>{});
        step(std::integral_constant<int, 3>{}); step(std::integral_constant<int, 4>{});
        step(std::integral_constant<int, 5>{}); step(std::integral_constant<int, 6>{});
        step(std::integral_constant<int, 7>{});
        for (int g = 0; g < G; ++g) a[t0 + g] = low(reduce<C::Flip>(e[g], o[g]));
    }
}
template<class C>
QF_AI void leaf4(V* a, V* b, const U* weights, const U* weights_ni) {
    if constexpr (C::Leaf >= 6) { leaf4_reg<C>(a, b, weights, weights_ni); return; }
    LeafBuf L;
    leaf_build<C>(a, b, weights, weights_ni, L);
    leaf_mac<C>(a, L);
}

template<class C>
struct Kernel {
    static constexpr int Tile = C::Tile;
    const U *rt, *irt;
    U leaf_cursor = ONE;
    Kernel(const U* r, const U* ir) : rt(r), irt(ir) {}

    // Plain: r[k]. Pair: r[2k]=w_k, r[2k+1]=w_k*NI (Montgomery) or (normal w_k,
    // floor(w_k*2^32/P)) for Shoup. Doubling from the valid prefix.
    static QF_AI int blk(int k) { return ((k >> 3) << 4) | (k & 7); }
    static void tables(int count, U* r, U* ir, int& size, bool fresh) {
        if constexpr (C::Blk) {
            if (fresh || size == 0) { r[0] = ir[0] = 1; r[8] = ir[8] = U((W(1) << 32) / P); size = 1; }
            constexpr U RMODP = U((W(1) << 32) % P);   // normal value whose Montgomery form is R2
            for (int h = size; h < count; h *= 2) {
                int s = __builtin_ctz(unsigned(h));
                for (int dir = 0; dir < 2; ++dir) {
                    U* t = dir ? ir : r; U qs = dir ? constants.iq[s] : constants.q[s];
                    if (h >= 8) {
                        const Fixed fq(splat(muls(qs, 1)), splat(qs * NI)), fc(splat(RMODP), splat(R2 * NI));
                        for (int j = 0; j < h; j += 8) {
                            V z = shrink(fq.mul<2, false, false, true>(_mm256_load_si256((const V*)(t + 2 * j))), P);
                            V zm = shrink(fc.mul<2, false, false, true>(z), P);   // mont(z)
                            _mm256_store_si256((V*)(t + 2 * (h + j)), z);
                            _mm256_store_si256((V*)(t + 2 * (h + j) + 8), _mm256_mullo_epi32(zm, splat(NI)));
                        }
                    } else for (int j = 0; j < h; ++j) {
                        U v = muls(t[blk(j)], qs); t[blk(h + j)] = v; t[blk(h + j) + 8] = muls(v, R2) * NI;
                    }
                }
            }
            size = std::max(size, count);
            return;
        }
        if (fresh || size == 0) {
            if constexpr (C::Mul == 2) { r[0] = ir[0] = 1; r[1] = ir[1] = U((W(1) << 32) / P); }
            else if constexpr (C::Pair) { r[0] = ir[0] = ONE; r[1] = ir[1] = ONE * NI; }
            else r[0] = ir[0] = ONE;
            size = 1;
        }
        for (int h = size; h < count; h *= 2) {
            int s = __builtin_ctz(unsigned(h));
            for (int dir = 0; dir < 2; ++dir) {
                U* t = dir ? ir : r; U qs = dir ? constants.iq[s] : constants.q[s];
                if constexpr (C::Pair) {
                    if (h >= 4) {
                        const V w = splat(qs), wi = splat(qs * NI), p = splat(P), ni = splat(NI);
                        for (int j = 0; j < h; j += 4) {
                            V x = _mm256_load_si256((const V*)(t + 2 * j));   // [w0 wi0 w1 wi1 ...]
                            V m = _mm256_mul_epu32(x, wi);
                            V z = _mm256_srli_epi64(_mm256_add_epi64(_mm256_mul_epu32(x, w), _mm256_mul_epu32(m, p)), 32);
                            z = shrink(z, P);                                      // high words stay 0
                            V zm = z;
                            if constexpr (C::Mul == 2) {   // Shoup quotient = mont(z)*NI
                                const V r2 = splat(R2), r2i = splat(R2 * NI);
                                zm = _mm256_srli_epi64(_mm256_add_epi64(_mm256_mul_epu32(z, r2), _mm256_mul_epu32(_mm256_mul_epu32(z, r2i), p)), 32);
                                zm = shrink(zm, P);
                            }
                            V zi = _mm256_slli_epi64(_mm256_mul_epu32(zm, ni), 32);
                            _mm256_store_si256((V*)(t + 2 * (h + j)), _mm256_or_si256(z, zi));
                        }
                    } else for (int j = 0; j < h; ++j) {
                        U v = muls(t[2 * j], qs); t[2 * (h + j)] = v;
                        t[2 * (h + j) + 1] = (C::Mul == 2 ? muls(v, R2) : v) * NI;
                    }
                } else {
                    if (h >= 8) {
                        Fixed f = Fixed::scalar(qs);
                        for (int j = 0; j < h; j += 8)
                            _mm256_store_si256((V*)(t + h + j), shrink(f.mul<1, false>(_mm256_load_si256((const V*)(t + j))), P));
                    } else for (int j = 0; j < h; ++j) t[h + j] = muls(t[j], qs);
                }
            }
        }
        size = std::max(size, count);
    }
    template<bool Inv> QF_AI Fixed fixed_at(int k) const {
        const U* t = Inv ? irt : rt;
        if constexpr (C::Blk) { const int i = blk(k); return Fixed(splat(t[i]), splat(t[i + 8])); }
        else if constexpr (C::Pair) return Fixed(splat(t[2 * k]), splat(t[2 * k + 1]));
        else return C::Mul == 1 ? Fixed::vec(splat(t[k])) : Fixed::scalar(t[k]);
    }
    template<bool Inv> QF_AI Twiddle twiddle(int k) const { return Twiddle{fixed_at<Inv>(k), fixed_at<Inv>(2 * k), fixed_at<Inv>(2 * k + 1)}; }

    template<bool Inv, int H = 0>
    QF_AI void group(V* a, V* b, int h_, int k) {
        const int h = H ? H : h_;
        Twiddle t = twiddle<Inv>(k);
        if (k == 0) {
            if constexpr (Inv) inv4<C, true>(a, h, t); else { fwd4<C, true>(a, h, t); fwd4<C, true>(b, h, t); }
        } else {
            if constexpr (Inv) inv4<C, false>(a, h, t); else { fwd4<C, false>(a, h, t); fwd4<C, false>(b, h, t); }
        }
    }
    // Weights of the four leaves of group k (in order) and their quotients.
    QF_AI void leaf_weights(int k, U* w, U* wi) {
        w[0] = leaf_cursor; w[2] = muls(w[0], constants.q[0]);
        leaf_cursor = muls(leaf_cursor, constants.even_step[__builtin_ctz(~unsigned(k))]);
        __m128i wv = _mm_setr_epi32(int(w[0]), int(P - w[0]), int(w[2]), int(P - w[2]));
        _mm_store_si128((__m128i*)wi, _mm_mullo_epi32(wv, _mm_set1_epi32(int(NI))));
        if constexpr (C::ShoupAux) {
            U n0 = muls(w[0], 1), n2 = muls(w[2], 1);
            wv = _mm_setr_epi32(int(n0), int(P - n0), int(n2), int(P - n2));
        }
        _mm_store_si128((__m128i*)w, wv);
    }
    QF_AI void leaves(V* a, V* b, int nv, int first) {
        if constexpr (C::Pipe && C::Leaf < 6) {
            static_assert(C::Pair);
            LeafBuf L[2];
            auto prep = [&](int j, LeafBuf& buf) __attribute__((always_inline)) {
                const int k = (first + j) / 4;
                group<false, 1>(a + j, b + j, 1, k);
                alignas(16) U w[4], wi[4];
                leaf_weights(k, w, wi);
                leaf_build<C>(a + j, b + j, w, wi, buf);
            };
            prep(0, L[0]);
            for (int j = 0; j < nv; j += 4) {
                if (j + 4 < nv) prep(j + 4, L[((j >> 2) + 1) & 1]);
                leaf_mac<C>(a + j, L[(j >> 2) & 1]);
                group<true, 1>(a + j, nullptr, 1, (first + j) / 4);
            }
            return;
        }
        for (int j = 0; j < nv; j += 4) {
            int k = (first + j) / 4;
            group<false, 1>(a + j, b + j, 1, k);
            alignas(16) U w[4], wi[4];
            w[0] = leaf_cursor; w[2] = muls(w[0], constants.q[0]);
            leaf_cursor = muls(leaf_cursor, constants.even_step[__builtin_ctz(~unsigned(k))]);
            if constexpr (C::Pair) {   // weights and their NI products as one vector each
                __m128i wv = _mm_setr_epi32(int(w[0]), int(P - w[0]), int(w[2]), int(P - w[2]));
                _mm_store_si128((__m128i*)wi, _mm_mullo_epi32(wv, _mm_set1_epi32(int(NI))));
                if constexpr (C::ShoupAux) {   // Shoup: normal-form weights, same quotients
                    U n0 = muls(w[0], 1), n2 = muls(w[2], 1);
                    wv = _mm_setr_epi32(int(n0), int(P - n0), int(n2), int(P - n2));
                }
                _mm_store_si128((__m128i*)w, wv);
                leaf4<C>(a + j, b + j, w, wi);
            } else {
                w[1] = P - w[0]; w[3] = P - w[2];
                leaf4<C>(a + j, b + j, w, nullptr);
            }
            group<true, 1>(a + j, nullptr, 1, k);
        }
    }
    template<int NV, int H> QF_AI void tile_forward(V* a, V* b, int k) {
        if constexpr (H >= 4) {
#pragma GCC unroll 1
            for (int j = 0; j < NV; j += 4 * H) group<false, H>(a + j, b + j, H, k * (NV / (4 * H)) + j / (4 * H));
            tile_forward<NV, H / 4>(a, b, k);
        }
    }
    template<int NV, int H> QF_AI void tile_inverse(V* a, int k) {
        if constexpr (H < NV) {
#pragma GCC unroll 1
            for (int j = 0; j < NV; j += 4 * H) group<true, H>(a + j, nullptr, H, k * (NV / (4 * H)) + j / (4 * H));
            tile_inverse<NV, H * 4>(a, k);
        }
    }
    template<int NV> __attribute__((noinline)) void fixed_tile(V* a, V* b, int k) {
        tile_forward<NV, NV / 4>(a, b, k);
        leaves(a, b, NV, k * NV);
        tile_inverse<NV, 4>(a, k);
    }
    void visit(V* a, V* b, int nv, int k) {
        if (nv <= Tile) {
            if (nv == 4) fixed_tile<4>(a, b, k);
            else if (nv == 16) fixed_tile<16>(a, b, k);
            else if (nv == 64) fixed_tile<64>(a, b, k);
            else if constexpr (Tile >= 256) {
                if (nv == 256) fixed_tile<256>(a, b, k);
                else if constexpr (Tile >= 1024) { if (nv == 1024) fixed_tile<1024>(a, b, k); }
            }
        } else {
            int h = nv / 4;
            group<false>(a, b, h, k);
            for (int t = 0; t < 4; ++t) visit(a + t * h, b + t * h, h, 4 * k + t);
            group<true>(a, nullptr, h, k);
        }
    }
    // Permutation fixing vector i: sigma when parity(i) is odd (Flip layout only).
    static QF_AI V seed(V x, unsigned i) {
        if constexpr (C::Flip) return _mm256_permutevar8x32_epi32(x, _mm256_load_si256((const V*)constants.perm_idx[__builtin_parity(i)]));
        else return x;
    }
    // nza/nzb: optional counts of leading entries that may be nonzero. When an
    // input's upper half is zero (ordinary convolution padding), the forward top
    // radix-2 layer is a copy: x+0 = x and x-0 = x for canonical x.
    static void run(int n, U* aa, U* bb, U* r, U* ir, int& size, bool fresh, int nza = 1 << 30, int nzb = 1 << 30) {
        assert(n >= 64 && n <= (1 << 22) && (n & (n - 1)) == 0);
        tables(n / 16, r, ir, size, fresh);
        Kernel job(r, ir);
        V *a = (V*)aa, *b = (V*)bb;
        const int nv = n / 8;
        // Result so far carries nv*R^-1; scale by nv^-1*R (Shoup) or its Montgomery form.
        const U s_norm = mont(power(U(nv), P - 2)), s_mont = mont(s_norm);
        const Fixed scale = C::ShoupAux ? Fixed(splat(s_norm), splat(s_mont * NI)) : Fixed::scalar(s_mont);
        constexpr int SM = C::ShoupAux ? 2 : C::MontMul;
        constexpr bool F = C::Flip;
        if (__builtin_ctz(unsigned(nv)) & 1) {
            const int h = nv / 2;
            auto top = [&](V* f, bool zero_upper) {
                if (zero_upper) {
                    if constexpr (C::Flip) for (int i = 0; i < h; ++i) { V x = seed(f[i], i); f[i] = x; f[i + h] = x; }
                    else for (int i = 0; i < h; ++i) f[i + h] = f[i];   // lower half already final
                }
                else for (int i = 0; i < h; ++i) {
                    V x = f[i], y = f[i + h];
                    f[i] = seed(low(plus(x, y)), i); f[i + h] = seed(low(diff(x, y)), i);
                }
            };
            top(a, nza <= n / 2); top(b, nzb <= n / 2);
            job.visit(a, b, h, 0); job.visit(a + h, b + h, h, 1);
            // Values < 2P; sums/differences < 4P are valid multiply inputs.
            for (int i = 0; i < h; ++i) {
                V x = a[i], y = a[i + h];
                a[i] = seed(shrink(scale.mul<SM, F, C::Shuf, C::Opq>(plus(x, y)), P), i);
                a[i + h] = seed(shrink(scale.mul<SM, F, C::Shuf, C::Opq>(diff(x, y)), P), i);
            }
        } else {
            if constexpr (F) for (int i = 0; i < nv; ++i) { a[i] = seed(a[i], i); b[i] = seed(b[i], i); }
            job.visit(a, b, nv, 0);
            for (int i = 0; i < nv; ++i) a[i] = seed(shrink(scale.mul<SM, F, C::Shuf, C::Opq>(a[i]), P), i);
        }
    }
};
}  // namespace qflip

namespace yosupo_406408_self {
using u32= uint32_t;
// self/ntt/convolution の最大は n = m = 2^20 で count = 2^21 - 1、length = 2^21。
constexpr int max_transform= 1 << 21;
constexpr int pad= 16;
alignas(32) inline u32 fa[max_transform + pad], fb[max_transform + pad];
alignas(32) inline u32 roots[max_transform / 8 + pad], inverse_roots[max_transform / 8 + pad];
}  // namespace yosupo_406408_self

struct Kernel {
 static void convolve_range(const u32* a, int n, const u32* b, int m, int lo, int hi, u32* c) {
  using namespace yosupo_406408_self;
  const int count= n + m - 1;
  int length= 64;
  while (length < count) length<<= 1;
  std::copy(a, a + n, fa);
  std::fill(fa + n, fa + length + pad, 0u);
  std::copy(b, b + m, fb);
  std::fill(fb + m, fb + length + pad, 0u);
  int root_size= 0;
  ::qflip::Kernel<::qflip::Cfg<2, false, true, 0, false, 256, true, true, true, 1, true, true>>::run(
      length, fa, fb, roots, inverse_roots, root_size, true, n, m);
  for (int k= lo; k < hi; ++k) c[k - lo]= fa[k];
 }
};

#ifdef PJ_CLANG_TARGET_PUSHED
#undef PJ_CLANG_TARGET_PUSHED
#pragma clang attribute pop
#endif
