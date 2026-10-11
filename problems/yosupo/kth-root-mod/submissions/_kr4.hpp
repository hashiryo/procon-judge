#pragma once
// k 乗根 mod p の 4 回目。3 回目の逆元を、拡張 Euclid の互除法 (64 bit の割り算を 20 回ほど) から NeoLibrary の inv_mod
// (割り算を使わない Stein の方法、32 bit) に替える。g < 2^16 の試し割りは、256 未満の素数の表と逆元の掛け算で割り切れるかを
// 見る (3 回目は奇数で割り算していた)。s = e なら T の成分の 2 つの逆元は同じなので 1 回で済ませる。
// 以下は 3 回目の説明。k 乗根 mod p の 3 回目。2 回目の累乗を、右から左の 2 進で、掛ける値をマスクの算術で選ぶ分岐なしの形にする (2 乗の鎖と
// 積の鎖が並ぶ)。同じ底の 2 つの累乗 (T と S の成分) は 2 乗の列を共有する。g < 2^16 の素因数は factorize を呼ばずに
// 小さい素数で試し割りする (配列を確保しない)。
// 以下は 2 回目の説明。k 乗根 mod p の 2 回目。1 回目の位数 r の離散対数の baby-step giant-step で、表を呼ぶたびに確保して並べ替えるのをやめ、
// スタックに置いた開番地法のハッシュ表 (512 個、キーは正規化した値) にする。Pohlig-Hellman の最後の桁では a_S z^{-L} を
// 更新しないので、z^{-1} とその冪を作らない。ζ = z^{r^{s-1}} は r 乗非剰余を確かめたときの ρ^{m / r} と同じなので使い回す。
// 以下は 1 回目の説明。k 乗根 mod p の 1 回目。X^K ≡ Y (mod P) を解く。m = P - 1、g = gcd(K, m) とすると、解があるのは Y^(m / g) = 1 のとき
// (Y = 0 と K = 0 は別に扱う)。K = g u とすると gcd(u, m / g) = 1 なので、Z^g = Y となる Z を求めて X = Z^v
// (v = u^{-1} mod (m / g)) とする (X^K = Y^{v u} = Y (Y^{m/g})^* = Y)。
//
// g の素因数 r ごとに、r^e (r^e ‖ g) 乗根を取っていく。m = r^s t (gcd(r, t) = 1) として、群を r の Sylow 部分群 S (位数
// r^s) と位数 t の部分 T に分ける。a の T の成分 a_T = a^{e_T} (e_T ≡ 1 mod t、≡ 0 mod r^s) の q = r^e 乗根は、q が t と
// 互いに素なので x_T = a_T^{q^{-1} mod t} と 1 つに決まる。S の成分 a_S = a^{e_S} は、r 乗非剰余 ρ から z = ρ^t (位数 r^s)
// を作り、a_S = z^L の L を Pohlig-Hellman で r 進の桁ごとに求める (桁は位数 r の ζ = z^{r^{s-1}} の離散対数で、小さい r は
// 表を順に見て、大きい r は baby-step giant-step で引く)。a_S は q 乗なので L は r^e で割り切れ、x_S = z^{L / r^e}。
// s = e なら a_S = 1 で x_S = 1。x = x_T x_S は残りの (g / q) 乗根も持つ (T の成分は a_T の冪で、S では g / q 乗が全単射)。
//
// 剰余の掛け算は R = 2^32 の Montgomery で、値は [0, 2P) のまま持つ (P < 2^30)。
#include "../common.hpp"
#include "neo/number_theory/factorize.hpp"
#include "neo/number_theory/inv_gcd.hpp"

namespace kr4 {

struct Mont {
    u32 p, pinv, r1, r2;  // pinv = p^{-1} mod 2^32、r1 = 2^32 mod p、r2 = 2^64 mod p
    explicit Mont(u32 p_) : p(p_) {
        u32 x = p;
        for (int i = 0; i < 5; ++i) x *= 2 - p * x;
        pinv = x;
        r1 = (u32)((1ull << 32) % p);
        r2 = (u32)((u64)r1 * r1 % p);
    }
    u32 mul(u32 a, u32 b) const {
        const u64 t = (u64)a * b;
        return (u32)(t >> 32) + p - (u32)((u64)((u32)t * pinv) * p >> 32);
    }
    u32 to(u32 a) const { return mul(a, r2); }
    u32 from(u32 a) const {
        const u32 x = mul(a, 1);
        return x >= p ? x - p : x;
    }
    u32 norm(u32 a) const { return a >= p ? a - p : a; }
    bool eq(u32 a, u32 b) const { return norm(a) == norm(b); }
    u32 pow(u32 b, u64 e) const {
        u32 r = r1;
        for (; e; e >>= 1, b = mul(b, b)) r = mul(r, r1 ^ ((r1 ^ b) & (0u - (u32)(e & 1))));
        return r;
    }
    // (b^e1, b^e2)。2 乗の列を共有する。
    std::pair<u32, u32> pow2(u32 b, u64 e1, u64 e2) const {
        u32 x = r1, y = r1;
        for (; e1 | e2; e1 >>= 1, e2 >>= 1, b = mul(b, b)) {
            x = mul(x, r1 ^ ((r1 ^ b) & (0u - (u32)(e1 & 1))));
            y = mul(y, r1 ^ ((r1 ^ b) & (0u - (u32)(e2 & 1))));
        }
        return {x, y};
    }
};

// a^{-1} mod m (gcd(a, m) = 1、1 ≤ m < 2^32)
inline u64 inv_m(u64 a, u64 m) { return m == 1 ? 0 : (u64)::inv_mod<u32>((u32)(a % m), (u32)m); }

// 256 未満の奇素数と、その逆元 (mod 2^32) と floor((2^32 - 1) / p)
struct SmallPrime {
    u32 p, inv, lim;
};
inline constexpr auto SMALLP = [] {
    std::array<SmallPrime, 53> t{};
    int k = 0;
    for (u32 p = 3; p < 256; p += 2) {
        bool pr = true;
        for (u32 d = 3; d * d <= p; d += 2) pr &= p % d != 0;
        if (!pr) continue;
        u32 x = p;
        for (int i = 0; i < 5; ++i) x *= 2 - p * x;
        t[k++] = {p, x, ~0u / p};
    }
    return t;
}();

// 位数 r の元 zeta について、c = zeta^l の l (0 ≤ l < r) を求める
inline u32 dlog_order_r(const Mont& M, u32 zeta, u32 c, u32 r) {
    if (r <= 64) {
        u32 v = M.r1;
        for (u32 l = 0; l < r; ++l, v = M.mul(v, zeta))
            if (M.eq(v, c)) return l;
        return 0;
    }
    // r 乗が要るのは r^2 | p - 1 のときだけなので r ≤ 31623、B ≤ 178。表は 512 個 (使うのは 35 % 以下)。
    constexpr u32 H = 512, EMPTY = ~0u;
    u32 key[H], val[H];
    for (u32 i = 0; i < H; ++i) key[i] = EMPTY;
    const u32 B = (u32)std::sqrt((double)r) + 1;
    u32 v = M.r1;
    for (u32 j = 0; j < B; ++j, v = M.mul(v, zeta)) {
        const u32 k = M.norm(v);
        u32 h = (k * 0x9e3779b1u) >> 23;
        while (key[h] != EMPTY && key[h] != k) h = (h + 1) & (H - 1);
        if (key[h] == EMPTY) key[h] = k, val[h] = j;
    }
    const u32 step = M.pow(zeta, r - B % r);  // zeta^{-B}
    u32 cur = c;
    for (u32 i = 0; i <= B; ++i, cur = M.mul(cur, step)) {
        const u32 k = M.norm(cur);
        for (u32 h = (k * 0x9e3779b1u) >> 23; key[h] != EMPTY; h = (h + 1) & (H - 1))
            if (key[h] == k) return (i * B + val[h]) % r;
    }
    return 0;
}

// a (q = r^e 乗剰余、Montgomery 形) の r^e 乗根 (Montgomery 形)。m = p - 1。
inline u32 root_prime_power(const Mont& M, u32 a, u32 r, int e, u32 m) {
    u32 t = m;
    int s = 0;
    while (t % r == 0) t /= r, ++s;
    u64 rs = 1;
    for (int i = 0; i < s; ++i) rs *= r;
    u64 q = 1;
    for (int i = 0; i < e; ++i) q *= r;
    // T の成分の q 乗根: a^{e_T w}、e_T = r^s (r^s)^{-1} mod t、w = q^{-1} mod t
    const u64 v1 = inv_m(rs % t, t), eT = rs * v1 % m, w = s == e ? v1 : inv_m(q % t, t);
    const u64 eTw = (u64)((unsigned __int128)eT * w % m);
    if (s == e) return M.pow(a, eTw);
    // S の成分 a_S = a^{e_S}、e_S = t (t^{-1} mod r^s)
    const u64 eS = (u64)t * inv_m(t % rs, rs) % m;
    const auto [xT, aS] = M.pow2(a, eTw, eS);
    // r 乗非剰余 ρ から z = ρ^t (位数 r^s)、ζ = z^{r^{s-1}} = ρ^{m / r} (位数 r)
    u32 rho = 1, zeta;
    do zeta = M.pow(M.to(++rho), m / r);
    while (M.eq(zeta, M.r1));
    const u32 z = M.pow(M.to(rho), t);
    // a_S = z^L の L を r 進で求める (下の e 桁は 0)。最後の桁のあとは a_S z^{-L} を更新しない
    u64 L = 0, re = q;  // re = r^i
    u32 cur = aS;       // a_S z^{-L}
    u32 zinv_re = 0;    // z^{-r^i} (要るときだけ作る)
    for (int i = e; i < s; ++i) {
        const u32 c = M.pow(cur, rs / re / r);  // (a_S z^{-L})^{r^{s-1-i}}
        const u32 l = dlog_order_r(M, zeta, c, r);
        L += (u64)l * re;
        if (i + 1 < s) {
            if (i == e) zinv_re = M.pow(M.pow(z, rs - 1), q);
            if (l) cur = M.mul(cur, M.pow(zinv_re, l));
            zinv_re = M.pow(zinv_re, r);
        }
        re *= r;
    }
    return M.mul(xT, M.pow(z, L / q));
}

inline i64 kth_root(u32 K, u32 Y, u32 P) {
    if (Y == 0) return K > 0 ? 0 : -1;
    if (K == 0) return Y == 1 ? 1 : -1;
    if (P == 2) return 1;
    const Mont M(P);
    const u32 m = P - 1, g = std::gcd(K, m);
    const u32 y = M.to(Y);
    if (!M.eq(M.pow(y, m / g), M.r1)) return -1;
    u32 z = y;
    if (g < (1u << 16)) {
        // 256 未満の素数で試し割り (g < 2^16 なので、残りは 1 か素数)
        u32 h = g;
        if (const int c = __builtin_ctz(h)) h >>= c, z = root_prime_power(M, z, 2, c, m);
        for (const auto& [r, inv, lim] : SMALLP) {
            if (r * r > h) break;
            if (h * inv > lim) continue;
            int e = 0;
            do h *= inv, ++e;
            while (h * inv <= lim);
            z = root_prime_power(M, z, r, e, m);
        }
        if (h > 1) z = root_prime_power(M, z, h, 1, m);
    } else
        for (auto [r, e] : factorize_pairs(g)) z = root_prime_power(M, z, (u32)r, e, m);
    const u64 v = inv_m(K / g, m / g);
    return M.from(M.pow(z, v));
}

}  // namespace kr4
