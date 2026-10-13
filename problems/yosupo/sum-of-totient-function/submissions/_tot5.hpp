#pragma once
// トーシェントの和の 5 回目。4 回目の再帰で、双曲線の分け目を √v から u = max(⌊R/q⌋, CU/100 √v) に替える。任意の u で
//   M(v) = 1 - Σ_{j=2}^{u} M(⌊v/j⌋) - Σ_{t≤t1} μ(t) ⌊v/t⌋ + u M(t1)  (t1 = ⌊v/(u+1)⌋)
// が成り立つ。前の和の j > R/q の項は 1 個ずつ割って表を引くので重く、後ろの和は AVX2 で 4 本ずつ進むので軽い。u を
// √v より小さくして重い項を減らす。u ≥ ⌊R/q⌋ なら t1 ≤ R + 2 なので、R + 3 までの μ と M で足りる。
//
// 以下は 4 回目の説明。トーシェントの和の 4 回目。3 回目の篩の最後の走査を AVX2 にする。32 バイトずつ μ を作り (符号、log と閾値の比べ、
// 平方因子の印)、床の値を含まない 32 個は sad で和を取って M に足す。床の値を含む 32 個だけ 1 個ずつ足して書き留める。
//
// 以下は 3 回目の説明。トーシェントの和の 3 回目。2 回目から次を替える。
// - 再帰の Σ_{t≤s1} μ(t) ⌊v/t⌋ を AVX2 の double 4 本で求める。⌊v/t⌋ は floor(v (1/t) + 2^{-17})、μ(t) との積を FMA で
//   double のまま足す (和は 1.2 × 10^11 未満なので 53 bit に収まり、丸めが起きない)。
// - 篩の符号と log を 1 バイトにまとめる (最上位が符号、下 7 bit が log2 p の 4 倍。素数の倍数ごとに 0x80 + l を足す。log の
//   和は 4 log2 L < 128 なので桁が上がらない)。p^2 の倍数は別のバイトの列に印を付ける。
//
// 以下は 2 回目の説明。トーシェントの和の 2 回目。1 回目から次を替える。
// - 再帰の Σ_{t≤s1} μ(t) ⌊v/t⌋ を、μ(t) ≠ 0 の t だけを並べた列 (t、μ(t)、1/t) で分岐なしに足す (μ(t) = 0 かの分岐が外れていた)。
// - 篩の 2、3、5、7 の倍数の符号と log、4、9、25、49 の倍数の 0 は、周期 44100 の型を写す。
// - 篩の最後の確かめ (√L より大きい素因数があるか) と M の足し上げを 1 回の走査にまとめ、反転はマスクで書く。
// LF は L を N^{2/3} の何倍にとるか。
//
// 以下は 1 回目の説明。トーシェントの和の 1 回目。Φ(N) = Σ_{i≤N} φ(i) = (1 + Σ_{d≤N} μ(d) ⌊N/d⌋^2) / 2 を、Mertens 関数 M(x) = Σ_{d≤x} μ(d) から
// 求める。R = ⌊√N⌋ として、d ≤ R は μ(d) を直に足し、d > R は q = ⌊N/d⌋ ごとにまとめて q^2 (M(⌊N/q⌋) - M(⌊N/(q+1)⌋)) を
// 足す。要るのは、R 以下の μ と M と、q ≤ R の W[q] = M(⌊N/q⌋)。
//
// W は 2 通りで作る。⌊N/q⌋ ≤ L となる q (K < q ≤ R、K = ⌊N/L⌋) は、L までの μ を区画ごとに篩い、M を足しながら床の値
// ⌊N/q⌋ を通るところで書き留める。q ≤ K は v = ⌊N/q⌋ について q の大きい順に
//   M(v) = 1 - Σ_{j=2}^{s} M(⌊v/j⌋) - Σ_{t≤s1} μ(t) ⌊v/t⌋ + s M(s1)  (s = ⌊√v⌋、s1 = ⌊v/(s+1)⌋)
// で求める。⌊v/j⌋ = ⌊N/(qj)⌋ なので、qj ≤ R なら W[qj]、そうでなければ R 以下の M の表を引く。M の値は小さい整数なので
// 剰余を取らない。⌊v/t⌋ は、1/t の double との積に 2^{-17} を足して切り捨てる (v < 2^34、t < 2^17 では割り算と一致する)。
//
// μ の篩は 2^16 個ずつの区画で、√L 以下の素数 p の倍数で符号を反転し、log2 p の 8 倍を u8 で足し、p^2 の倍数を 0 にする。
// 最後に、足した log が log2 x より √L の log の半分以上小さければ √L より大きい素因数が 1 つあるので反転する。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#include <simde/x86/fma.h>
#else
#include <immintrin.h>
#endif
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <vector>
#include "../common.hpp"

namespace tot5 {
using u8 = unsigned char;
using i8 = signed char;

constexpr u32 MOD = 998244353;

inline u64 isqrt(u64 n) {
    u64 r = (u64)std::sqrt((double)n);
    while (r * r > n) --r;
    while ((r + 1) * (r + 1) <= n) ++r;
    return r;
}

// 2、3、5、7 の倍数の符号と log、4、9、25、49 の倍数の 0 の型 (周期 44100)
constexpr u32 PER = 44100;
struct PreSieve {
    std::array<u8, PER> sv, zf;  // sv: 最上位が符号、下 7 bit が log の和。zf: 平方因子があれば 1
};
inline u8 lg4(u32 p) { return (u8)std::lround(std::log2((double)p) * 4); }
inline const PreSieve& presieve() {
    static const PreSieve t = [] {  // 倍数を飛びながら作る (x ごとに割り算と log をすると 1 ms ほどかかる)
        PreSieve r{};
        for (u32 p : {2u, 3u, 5u, 7u}) {
            const u8 a = (u8)(0x80 + lg4(p));
            for (u32 x = 0; x < PER; x += p) r.sv[x] = (u8)(r.sv[x] + a);
            for (u32 x = 0; x < PER; x += p * p) r.zf[x] = 1;
        }
        return r;
    }();
    return t;
}

// Φ(N) mod 998244353
template <int LF10 = 8, int CU = 40>  // L = LF10 / 10 N^{2/3}、分け目は CU / 100 √v
inline u32 totient_sum(u64 N) {
    if (N == 0) return 0;
    const u64 R = isqrt(N);
    // L は N^{2/3} の数倍。区画の大きさの倍数に揃える
    constexpr u32 S = 1u << 16;
    u64 L = (u64)(LF10 / 10.0 * std::cbrt((double)N) * std::cbrt((double)N));
    L = std::max<u64>(L, R + 1);
    L = (L + S - 1) / S * S;
    const u64 K = N / L;  // q ≤ K は ⌊N/q⌋ > L ... (⌊N/q⌋ ≥ L のこともある)
    // R 以下の μ と M (μ は小さい篩で)
    const u64 RS = R + 3;  // μ と M を持つ上限
    std::vector<i8> mus(RS + 1, 1);
    std::vector<int> Ms(RS + 1, 0);
    {
        std::vector<u32> pr;
        std::vector<u8> comp(RS + 1, 0);
        mus[0] = 0;
        for (u64 i = 2; i <= RS; ++i) {
            if (!comp[i]) pr.push_back((u32)i), mus[i] = -1;
            for (u32 p : pr) {
                if ((u64)p * i > RS) break;
                comp[p * i] = 1;
                if (i % p == 0) {
                    mus[p * i] = 0;
                    break;
                }
                mus[p * i] = (i8)-mus[i];
            }
        }
        for (u64 i = 1; i <= RS; ++i) Ms[i] = Ms[i - 1] + mus[i];
    }
    // W[q] = M(⌊N/q⌋)、q = 1..R+1
    std::vector<i64> W(R + 2, 0);
    // 篩: [1, L] の μ を区画ごとに。床の値 ⌊N/q⌋ (q は R から K+1 へ、値は小さい順) を通るところで W を書く
    {
        const u32 sq = (u32)isqrt(L);
        std::vector<u32> pr;
        std::vector<u8> lgp;
        {
            std::vector<u8> c(sq + 1, 0);
            for (u32 i = 2; i <= sq; ++i)
                if (!c[i]) {
                    if (i > 7) pr.push_back(i), lgp.push_back((u8)(0x80 + lg4(i)));
                    for (u32 j = i * i; j <= sq; j += i) c[j] = 1;
                }
        }
        const PreSieve& ps = presieve();
        std::vector<u8> sv(S), zf(S);
        i64 M = 0;
        u64 q = R;  // 次に書き留める W の添字
        while (q > K && N / q <= R) --q;  // ⌊N/q⌋ ≤ R の分は Ms から
        for (u64 qq = R; qq > q; --qq) W[qq] = Ms[N / qq];
        for (u64 lo = 1; lo <= L; lo += S) {
            const u64 hi = lo + S;  // [lo, hi)
            // 型を写す (位相は lo mod PER)
            for (u32 i = 0, ph = (u32)(lo % PER); i < S;) {
                const u32 c = std::min(S - i, PER - ph);
                std::memcpy(&sv[i], &ps.sv[ph], c), std::memcpy(&zf[i], &ps.zf[ph], c);
                i += c, ph = 0;
            }
            for (size_t k = 0; k < pr.size(); ++k) {
                const u64 p = pr[k];
                if (p >= hi) break;
                const u8 l = lgp[k];
                for (u64 m = (lo + p - 1) / p * p; m < hi; m += p) sv[m - lo] = (u8)(sv[m - lo] + l);
                const u64 p2 = p * p;
                if (p2 < hi)
                    for (u64 m = (lo + p2 - 1) / p2 * p2; m < hi; m += p2) zf[m - lo] = 1;
            }
            // 素数 p ≤ √L も上の輪で -1 になる (log が log p なので反転しない)
            const int th = (int)std::lround((std::log2((double)lo) - 0.5 * std::log2((double)L) * 0.5) * 4);
            u64 nx = q > K ? N / q : ~0ull;
            if (lo < S) {  // lo が小さい区画は x ごとに閾値を求める
                for (u32 i = 0; i < S; ++i) {
                    const u64 x = lo + i;
                    const int t = x == 1 ? -1000 : (int)std::lround((std::log2((double)x) - 0.5 * std::log2((double)L) * 0.5) * 4);
                    const int sg = sv[i] >> 7, f = (int)(sv[i] & 0x7F) < t;
                    M += zf[i] ? 0 : 1 - 2 * (sg ^ f);
                    if (x == nx) {
                        W[q] = M, --q;
                        nx = q > K ? N / q : ~0ull;
                    }
                }
                continue;
            }
            // 最後の確かめと M の足し上げ、床の値で W を書き留める (32 個ずつ)
            {
                const __m256i zero = _mm256_setzero_si256(), m7f = _mm256_set1_epi8(0x7F), one = _mm256_set1_epi8(1), thv = _mm256_set1_epi8((char)th);
                alignas(32) i8 mu8[32];
                for (u32 i = 0; i < S; i += 32) {
                    const __m256i v = _mm256_loadu_si256((const __m256i*)&sv[i]), z = _mm256_loadu_si256((const __m256i*)&zf[i]);
                    const __m256i sm = _mm256_cmpgt_epi8(zero, v), fm = _mm256_cmpgt_epi8(thv, _mm256_and_si256(v, m7f));
                    const __m256i mu = _mm256_andnot_si256(_mm256_cmpeq_epi8(z, one), _mm256_or_si256(_mm256_xor_si256(sm, fm), one));
                    if (lo + i + 31 < nx) {  // 床の値を含まない: (μ + 1) の和から 32 を引く
                        const __m256i sad = _mm256_sad_epu8(_mm256_add_epi8(mu, one), zero);
                        const __m128i h = _mm_add_epi64(_mm256_castsi256_si128(sad), _mm256_extracti128_si256(sad, 1));
                        M += (i64)(_mm_cvtsi128_si64(h) + _mm_extract_epi64(h, 1)) - 32;
                        continue;
                    }
                    _mm256_store_si256((__m256i*)mu8, mu);
                    for (u32 d = 0; d < 32; ++d) {
                        M += mu8[d];
                        if (lo + i + d == nx) {
                            W[q] = M, --q;
                            nx = q > K ? N / q : ~0ull;
                        }
                    }
                }
            }
        }
    }
    // 大きい v = ⌊N/q⌋ (q ≤ K) は q の大きい順に
    {
        std::vector<double> rt(RS + 1);
        for (u64 t = 1; t <= RS; ++t) rt[t] = 1.0 / (double)t;
        // μ(t) ≠ 0 の t の列 (μ(t)、1/t を double で) と、x 以下の個数 cnt[x]
        std::vector<double> sfm, sfr;
        std::vector<u32> cnt(RS + 1, 0);
        for (u64 t = 1; t <= RS; ++t) {
            if (mus[t]) sfm.push_back(mus[t]), sfr.push_back(rt[t]);
            cnt[t] = (u32)sfm.size();
        }
        for (int pad = 0; pad < 4; ++pad) sfm.push_back(0), sfr.push_back(0);
        for (u64 qi = K; qi >= 1; --qi) {
            const u64 v = N / qi, s = isqrt(v);
            const u64 u = std::min(s, std::max<u64>({R / qi, (u64)(CU / 100.0 * (double)s), 1})), s1 = v / (u + 1);
            const double vd = (double)v;
            i64 r = 1 + (i64)u * Ms[s1];
            // Σ_{j=2}^{u} M(⌊v/j⌋)
            u64 j = 2;
            for (; j <= u && qi * j <= R; ++j) r -= W[qi * j];
            for (; j <= u; ++j) r -= Ms[(u64)(vd * rt[j] + 0x1p-17)];
            // Σ_{t≤s1} μ(t) ⌊v/t⌋ を 4 本ずつ (列の後ろは μ = 0 で埋めてある)
            {
                const size_t na = cnt[s1];
                const __m256d vv = _mm256_set1_pd(vd), cc = _mm256_set1_pd(0x1p-17);
                __m256d acc = _mm256_setzero_pd();
                for (size_t a = 0; a < na; a += 4) {
                    const __m256d qv = _mm256_floor_pd(_mm256_add_pd(_mm256_mul_pd(vv, _mm256_loadu_pd(&sfr[a])), cc));
                    acc = _mm256_fmadd_pd(_mm256_loadu_pd(&sfm[a]), qv, acc);
                }
                // na を 4 の倍数に切り上げた分は、t > s1 を含むので引き戻す
                for (size_t a = na; a < (na + 3) / 4 * 4; ++a) r += (i64)(sfm[a] * std::floor(vd * sfr[a] + 0x1p-17));
                alignas(32) double h[4];
                _mm256_store_pd(h, acc);
                r -= (i64)((h[0] + h[1]) + (h[2] + h[3]));
            }
            W[qi] = r;
        }
    }
    // Σ_{d≤N} μ(d) ⌊N/d⌋^2 mod p
    u64 S2 = 0;
    auto sq = [](u64 x) { x %= MOD; return x * x % MOD; };
    for (u64 d = 1; d <= R; ++d)
        if (mus[d]) S2 = (S2 + (mus[d] > 0 ? sq(N / d) : MOD - sq(N / d))) % MOD;
    const u64 q1 = N / (R + 1);
    for (u64 qv = 1; qv <= q1; ++qv) {
        const i64 diff = W[qv] - (qv < q1 ? W[qv + 1] : (i64)Ms[R]);
        const u64 dm = (u64)((diff % (i64)MOD + MOD) % MOD);
        S2 = (S2 + sq(qv) * dm) % MOD;
    }
    return (u32)((1 + S2) % MOD * ((MOD + 1) / 2) % MOD);
}

}  // namespace tot5
