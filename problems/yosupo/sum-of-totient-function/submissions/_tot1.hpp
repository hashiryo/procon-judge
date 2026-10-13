#pragma once
// トーシェントの和の 1 回目。Φ(N) = Σ_{i≤N} φ(i) = (1 + Σ_{d≤N} μ(d) ⌊N/d⌋^2) / 2 を、Mertens 関数 M(x) = Σ_{d≤x} μ(d) から
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
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <vector>
#include "../common.hpp"

namespace tot1 {
using u8 = unsigned char;
using i8 = signed char;

constexpr u32 MOD = 998244353;

inline u64 isqrt(u64 n) {
    u64 r = (u64)std::sqrt((double)n);
    while (r * r > n) --r;
    while ((r + 1) * (r + 1) <= n) ++r;
    return r;
}

// Φ(N) mod 998244353
inline u32 totient_sum(u64 N) {
    if (N == 0) return 0;
    const u64 R = isqrt(N);
    // L は N^{2/3} の数倍。区画の大きさの倍数に揃える
    constexpr u32 S = 1u << 16;
    u64 L = (u64)(0.5 * std::cbrt((double)N) * std::cbrt((double)N));
    L = std::max<u64>(L, R + 1);
    L = (L + S - 1) / S * S;
    const u64 K = N / L;  // q ≤ K は ⌊N/q⌋ > L ... (⌊N/q⌋ ≥ L のこともある)
    // R 以下の μ と M (μ は小さい篩で)
    std::vector<i8> mus(R + 2, 1);
    std::vector<int> Ms(R + 2, 0);
    {
        std::vector<u32> pr;
        std::vector<u8> comp(R + 2, 0);
        mus[0] = 0;
        for (u64 i = 2; i <= R + 1; ++i) {
            if (!comp[i]) pr.push_back((u32)i), mus[i] = -1;
            for (u32 p : pr) {
                if ((u64)p * i > R + 1) break;
                comp[p * i] = 1;
                if (i % p == 0) {
                    mus[p * i] = 0;
                    break;
                }
                mus[p * i] = (i8)-mus[i];
            }
        }
        for (u64 i = 1; i <= R + 1; ++i) Ms[i] = Ms[i - 1] + mus[i];
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
                    pr.push_back(i), lgp.push_back((u8)std::lround(std::log2((double)i) * 8));
                    for (u32 j = i * i; j <= sq; j += i) c[j] = 1;
                }
        }
        std::vector<i8> mu(S);
        std::vector<u8> lg(S);
        i64 M = 0;
        u64 q = R;  // 次に書き留める W の添字
        while (q > K && N / q <= R) --q;  // ⌊N/q⌋ ≤ R の分は Ms から
        for (u64 qq = R; qq > q; --qq) W[qq] = Ms[N / qq];
        for (u64 lo = 1; lo <= L; lo += S) {
            const u64 hi = lo + S;  // [lo, hi)
            std::fill(mu.begin(), mu.end(), 1), std::fill(lg.begin(), lg.end(), 0);
            for (size_t k = 0; k < pr.size(); ++k) {
                const u64 p = pr[k];
                if (p >= hi) break;
                const u8 l = lgp[k];
                for (u64 m = (lo + p - 1) / p * p; m < hi; m += p) mu[m - lo] = (i8)-mu[m - lo], lg[m - lo] += l;
                const u64 p2 = p * p;
                if (p2 < hi)
                    for (u64 m = (lo + p2 - 1) / p2 * p2; m < hi; m += p2) mu[m - lo] = 0;
            }
            // 素数 p ≤ √L も上の輪で -1 になる (lg = log p なので反転しない)
            const int th = (int)std::lround((std::log2((double)lo) - 0.5 * std::log2((double)L) * 0.5) * 8);
            for (u32 i = 0; i < S; ++i) {
                const u64 x = lo + i;
                if (x <= 1) {
                    mu[i] = x == 1 ? 1 : 0;
                    continue;
                }
                // lo が小さい区画は x ごとに閾値を求める
                const int t = lo < S ? (int)std::lround((std::log2((double)x) - 0.5 * std::log2((double)L) * 0.5) * 8) : th;
                if (mu[i] && (int)lg[i] < t) mu[i] = (i8)-mu[i];
            }
            // M を足しながら W を書き留める
            u64 nx = q > K ? N / q : ~0ull;
            for (u32 i = 0; i < S; ++i) {
                M += mu[i];
                if (lo + i == nx) {
                    W[q] = M, --q;
                    nx = q > K ? N / q : ~0ull;
                }
            }
        }
    }
    // 大きい v = ⌊N/q⌋ (q ≤ K) は q の大きい順に
    {
        std::vector<double> rt(R + 2);
        for (u64 t = 1; t <= R + 1; ++t) rt[t] = 1.0 / (double)t;
        for (u64 qi = K; qi >= 1; --qi) {
            const u64 v = N / qi, s = isqrt(v), s1 = v / (s + 1);
            const double vd = (double)v;
            i64 r = 1 + (i64)s * Ms[s1];
            // Σ_{j=2}^{s} M(⌊v/j⌋)
            u64 j = 2;
            for (; j <= s && qi * j <= R; ++j) r -= W[qi * j];
            for (; j <= s; ++j) r -= Ms[(u64)(vd * rt[j] + 0x1p-17)];
            // Σ_{t≤s1} μ(t) ⌊v/t⌋
            for (u64 t = 1; t <= s1; ++t)
                if (mus[t]) r -= mus[t] * (i64)(u64)(vd * rt[t] + 0x1p-17);
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

}  // namespace tot1
