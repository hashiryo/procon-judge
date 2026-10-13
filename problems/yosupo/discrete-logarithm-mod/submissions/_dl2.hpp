#pragma once
// 離散対数の 2 回目。1 回目の baby-step giant-step の表を作り直す。項を 4 バイト (鍵のハッシュの 16 bit の指紋と、添字
// j + 1 の 16 bit) にし、表の大きさを B の HK 倍以上にとって、ほとんど空きにする。giant-step で引く鍵はほとんど表に無く、
// 線形探索で空きに当たるまでの回数が 1 回で済めば、引くたびの分岐が外れない (手元の M2 では、詰まり具合 1/2 の表より
// 16 倍の表のほうが 2 倍速かった)。指紋が合ったら ζ^j を累乗で作って確かめる。表の領域は問ごとに確保せず使い回す
// (B < 2^16)。B は √r、鎖は 8 本。
// 以下は 1 回目の説明。離散対数の 1 回目。X^K ≡ Y (mod M) となる最小の K を Pohlig-Hellman で求める。
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

namespace dl2 {

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

// 位数 r の ζ の冪 ζ^j (j < B) の表。c = ζ^l の l を引く (無ければ -1)。項は (指紋 16 bit << 16) | (j + 1)、0 は空き。
inline std::vector<u32>& table_buf() {
    static std::vector<u32> buf;
    return buf;
}
template <int HK>
struct OrderRLog {
    const Barrett& M;
    u64 zeta, r, B, H, step;  // step = ζ^{-B}
    int lg;
    u32* tab = nullptr;
    OrderRLog(const Barrett& M_, u64 zeta_, u64 r_) : M(M_), zeta(zeta_), r(r_) {
        if (r <= 64) return;
        B = ((u64)std::sqrt((double)r) + 8) & ~7ull;
        lg = 4;
        while ((1ull << lg) < HK * B) ++lg;
        H = 1ull << lg;
        auto& buf = table_buf();
        if (buf.size() < H) buf.resize(H);
        tab = buf.data();
        std::memset(tab, 0, H * sizeof(u32));
        u64 zp[8];
        zp[0] = 1 % M.m;
        for (int d = 1; d < 8; ++d) zp[d] = M.mul(zp[d - 1], zeta);
        const u64 z8 = M.mul(zp[7], zeta);
        for (u64 j = 0; j < B; j += 8)
            for (int d = 0; d < 8; ++d) ins(zp[d], j + d), zp[d] = M.mul(zp[d], z8);
        step = M.pow(zp[0], r - 1);  // zp[0] = ζ^B
    }
    u64 hash(u64 k) const { return k * 0x9e3779b97f4a7c15ull; }
    void ins(u64 k, u64 j) {
        const u64 h = hash(k);
        u64 i = h >> (64 - lg);
        const u32 e = (u32)(h >> 16 & 0xffff0000u) | (u32)(j + 1);
        while (tab[i]) i = (i + 1) & (H - 1);
        tab[i] = e;
    }
    // 鍵 k の候補の j を順に試す
    long long look(u64 k) const {
        const u64 h = hash(k);
        const u32 fp = (u32)(h >> 16 & 0xffff0000u);
        for (u64 i = h >> (64 - lg); tab[i]; i = (i + 1) & (H - 1))
            if ((tab[i] & 0xffff0000u) == fp) {
                const u64 j = (tab[i] & 0xffffu) - 1;
                if (M.pow(zeta, j) == k) return (long long)j;
            }
        return -1;
    }
    long long operator()(u64 c) const {
        if (r <= 64) {
            u64 v = 1 % M.m;
            for (u64 l = 0; l < r; ++l, v = M.mul(v, zeta))
                if (v == c) return (long long)l;
            return -1;
        }
        u64 sp[8], u[8];
        sp[0] = 1 % M.m;
        for (int d = 1; d < 8; ++d) sp[d] = M.mul(sp[d - 1], step);
        const u64 s8 = M.mul(sp[7], step);
        for (int d = 0; d < 8; ++d) u[d] = M.mul(c, sp[d]);
        for (u64 i = 0; i <= r / B + 8; i += 8) {
            for (int d = 0; d < 8; ++d)
                if (const long long j = look(u[d]); j >= 0) return (long long)(((i + d) * B + j) % r);
            for (int d = 0; d < 8; ++d) u[d] = M.mul(u[d], s8);
        }
        return -1;
    }
};

// gcd(x, m) = 1 で x^k ≡ t (mod m) となる最小の k (無ければ -1)
template <int HK>
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
        const OrderRLog<HK> lg(M, zeta, r);
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

template <int HK>
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
    const i64 k = coprime_log<HK>(X % m, t, m);
    return k < 0 ? -1 : k + c;
}

}  // namespace dl2
