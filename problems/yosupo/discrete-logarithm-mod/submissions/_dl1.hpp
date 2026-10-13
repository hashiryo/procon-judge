#pragma once
// 離散対数の 1 回目。X^K ≡ Y (mod M) となる最小の K を Pohlig-Hellman で求める。
//
// X と M が互いに素でない部分: K ≤ 32 は直に調べる。そこで見つからなければ、g = gcd(X, M) > 1 の間、Y が g で割り切れる
// ことを確かめて M、Y を g で割り、d に X / g を掛けて、段の数 c を数える (c ≤ 30)。残りは d X^{K'} ≡ Y (mod m)、
// gcd(X, m) = 1 で、答えは K = c + K' (K' は最小の解)。
//
// 互いに素な部分: m を factorize_pairs で分解して φ(m) の素因数分解を作り、X の位数 n を φ(m) から素因数を外していって
// 求める。T = Y d^{-1} について T^n ≠ 1 なら解は無い。n の素因数 r ごとに、γ = X^{n / r^e} (位数 r^e)、τ = T^{n / r^e}
// として、γ^x = τ の x を r 進の桁ごとに求める (桁は位数 r の ζ = γ^{r^{e-1}} の離散対数)。桁の離散対数は、r ≤ 64 なら
// 表を順に見て、それより大きければ baby-step giant-step で引く (表 ζ^j は r ごとに 1 回作り、掛け算の鎖は 4 本)。合成数の
// m では群が巡回群とは限らないので、桁が見つからなければ解は無い。x を CRT でまとめると [0, n) の最小の解になる。
//
// 剰余の掛け算は Barrett (m は偶数でもよい、m ≤ 10^9 なので積は 2^60 未満)。
#include "../common.hpp"
#include "neo/number_theory/factorize.hpp"
#include "neo/number_theory/inv_gcd.hpp"

namespace dl1 {

struct Barrett {
    u64 m, im;  // im = floor(2^64 / m) (m ≥ 2)
    explicit Barrett(u64 m_) : m(m_), im(m_ >= 2 ? (u64)(((u128)1 << 64) / m_) : 0) {}
    u64 mul(u64 a, u64 b) const {
        if (m == 1) return 0;
        const u64 t = a * b, q = (u64)((u128)t * im >> 64);
        u64 r = t - q * m;
        return r >= m ? r - m : r;
    }
    u64 pow(u64 b, u64 e) const {
        u64 r = 1 % m;
        for (; e; e >>= 1, b = mul(b, b)) r = mul(r, (e & 1) ? b : 1);
        return r;
    }
};

// 位数 r の ζ の冪 ζ^j (j < B) の表。c = ζ^l の l を引く (無ければ -1)。
struct OrderRLog {
    const Barrett& M;
    u64 zeta, r, B, H, step;  // step = ζ^{-B}
    int lg;
    std::vector<u32> key, val;
    static constexpr u32 EMPTY = ~0u;
    OrderRLog(const Barrett& M_, u64 zeta_, u64 r_) : M(M_), zeta(zeta_), r(r_) {
        if (r <= 64) return;
        B = ((u64)std::sqrt((double)r) + 4) & ~3ull;
        lg = 4;
        while ((1ull << lg) < 2 * B) ++lg;
        H = 1ull << lg;
        key.assign(H, EMPTY), val.resize(H);
        const u64 z2 = M.mul(zeta, zeta), z3 = M.mul(z2, zeta), z4 = M.mul(z3, zeta);
        u64 v[4] = {1 % M.m, zeta, z2, z3};
        for (u64 j = 0; j < B; j += 4)
            for (int d = 0; d < 4; ++d) ins(v[d], j + d), v[d] = M.mul(v[d], z4);
        step = M.pow(v[0], r - 1);  // v[0] = ζ^B
    }
    u64 hash(u64 k) const { return (k * 0x9e3779b97f4a7c15ull) >> (64 - lg); }
    void ins(u64 k, u64 j) {
        u64 h = hash(k);
        while (key[h] != EMPTY && key[h] != k) h = (h + 1) & (H - 1);
        if (key[h] == EMPTY) key[h] = (u32)k, val[h] = (u32)j;
    }
    long long look(u64 k) const {
        for (u64 h = hash(k); key[h] != EMPTY; h = (h + 1) & (H - 1))
            if (key[h] == k) return val[h];
        return -1;
    }
    long long operator()(u64 c) const {
        if (r <= 64) {
            u64 v = 1 % M.m;
            for (u64 l = 0; l < r; ++l, v = M.mul(v, zeta))
                if (v == c) return (long long)l;
            return -1;
        }
        const u64 s2 = M.mul(step, step), s3 = M.mul(s2, step), s4 = M.mul(s3, step);
        u64 u[4] = {c, M.mul(c, step), M.mul(c, s2), M.mul(c, s3)};
        for (u64 i = 0; i <= r / B + 4; i += 4) {
            for (int d = 0; d < 4; ++d)
                if (const long long j = look(u[d]); j >= 0) return (long long)(((i + d) * B + j) % r);
            for (int d = 0; d < 4; ++d) u[d] = M.mul(u[d], s4);
        }
        return -1;
    }
};

// gcd(x, m) = 1 で x^k ≡ t (mod m) となる最小の k (無ければ -1)
inline i64 coprime_log(u64 x, u64 t, u64 m) {
    if (m == 1) return 0;
    const Barrett M(m);
    // φ(m) の素因数分解
    std::vector<std::pair<u64, int>> phi;
    u64 ph = 1;
    auto add = [&](u64 q, int e) {
        for (auto& [r, c] : phi)
            if (r == q) return void(c += e);
        phi.emplace_back(q, e);
    };
    for (auto [p, e] : factorize_pairs(m)) {
        for (int i = 1; i < e; ++i) ph *= p;
        if (e > 1) add(p, e - 1);
        ph *= p - 1;
        for (auto [q, f] : factorize_pairs(p - 1)) add(q, f);
    }
    // x の位数 n
    u64 n = ph;
    for (auto& [r, c] : phi)
        while (c > 0 && M.pow(x, n / r) == 1) n /= r, --c;
    if (M.pow(t, n) != 1) return -1;
    // n の素因数 r^e ごとに x mod r^e を求めて CRT でまとめる
    u64 X = 0, mod = 1;
    for (auto [r, e] : phi) {
        if (e == 0) continue;
        u64 re = 1;
        for (int i = 0; i < e; ++i) re *= r;
        const u64 gam = M.pow(x, n / re), tau = M.pow(t, n / re);
        const u64 zeta = M.pow(gam, re / r);
        const OrderRLog lg(M, zeta, r);
        const u64 ginv = M.pow(gam, re - 1);
        u64 xr = 0, ri = 1, ginv_ri = ginv;  // ginv_ri = γ^{-r^i}
        u64 cur = tau;                       // τ γ^{-xr}
        for (int i = 0; i < e; ++i) {
            const long long l = lg(M.pow(cur, re / ri / r));
            if (l < 0) return -1;
            if (l) cur = M.mul(cur, M.pow(ginv_ri, (u64)l)), xr += (u64)l * ri;
            ginv_ri = M.pow(ginv_ri, r), ri *= r;
        }
        // X ≡ X (mod mod)、≡ xr (mod re) をまとめる
        const u64 k = (u64)((u128)((xr + re - X % re) % re) * ::inv_mod<u64>(mod % re, re) % re);
        X += mod * k, mod *= re;
    }
    return (i64)X;
}

inline i64 discrete_log(u64 X, u64 Y, u64 M) {
    if (M == 1) return 0;
    // K ≤ 32 は直に調べる
    {
        u64 v = 1 % M;
        for (int k = 0; k <= 32; ++k, v = v * X % M)
            if (v == Y) return k;
    }
    u64 m = M, b = Y, d = 1;
    int c = 0;
    for (u64 g; (g = std::gcd(X, m)) > 1;) {
        if (b % g) return -1;
        m /= g, b /= g, ++c;
        d = d * (X / g) % m;
    }
    if (m == 1) return -1;  // K > 32 の解は K ≤ 32 の解より大きいので、ここに来るのは解が無いとき
    const u64 t = (u64)((u128)b * ::inv_mod<u64>(d % m, m) % m);
    const i64 k = coprime_log(X % m, t, m);
    return k < 0 ? -1 : k + c;
}

}  // namespace dl1
