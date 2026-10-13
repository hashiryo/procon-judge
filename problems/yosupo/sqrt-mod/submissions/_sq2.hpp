#pragma once
// 平方根 mod p の 2 回目。1 回目の Cipolla の F_P^2 での累乗を分岐なしにし (掛ける値をマスクで選ぶ)、Jacobi 記号を割り算を
// 使わない二進の形にする (奇数どうしの差を取り、2 の冪を ctz で外す。入れ替えと符号の更新はマスクで、分岐は輪の出口だけ)。
// 以下は 1 回目の説明。平方根 mod p の 1 回目。X^2 ≡ Y (mod P) を P の剰余で道を分けて解く。
//
// P ≡ 3 (mod 4): X = Y^{(P+1)/4} とし、X^2 = Y を確かめる (違えば平方非剰余)。
// P ≡ 5 (mod 8): Atkin の方法。v = (2Y)^{(P-5)/8}、i = 2Y v^2 (i^2 = -1)、X = Y v (i - 1) とし、X^2 = Y を確かめる。
// P ≡ 1 (mod 8): P - 1 = 2^s Q (Q は奇数)。s ≤ STS なら Tonelli-Shanks、それより大きければ Cipolla。
//   Tonelli-Shanks: t = Y^Q、R = Y^{(Q+1)/2} (2 乗の列を共有)、c = z^Q (z は平方非剰余) とし、t の位数を 2 乗で調べて
//   R に c の冪を掛けていく。t^{2^{s-1}} ≠ 1 なら平方非剰余。z は、P ≡ 1 (mod 4) なので相互法則で (q / P) = (P mod q / q) と
//   なり、小さい素数 q ごとに P mod q が q の平方剰余かを表で見て探す (累乗が要らない)。
//   Cipolla: a^2 - Y が平方非剰余になる a を探し (Jacobi 記号)、F_P[ω] / (ω^2 - (a^2 - Y)) で (a + ω)^{(P+1)/2} を求める。
//   先に Jacobi 記号 (Y / P) で平方非剰余を除く。
// 剰余の掛け算は R = 2^32 の Montgomery で、値は 2P 未満のまま持つ (P < 2^30)。累乗の掛ける値はマスクで選ぶ。
#include "../common.hpp"

namespace sq2 {

struct Mont {
    u32 p, pinv, r1, r2;  // pinv = p^{-1} mod 2^32、r1 = 2^32 mod p、r2 = 2^64 mod p
    explicit Mont(u32 p_) : p(p_) {
        u32 x = p;
        for (int i = 0; i < 5; ++i) x *= 2 - p * x;
        pinv = x, r1 = (u32)((1ull << 32) % p), r2 = (u32)((u64)r1 * r1 % p);
    }
    u32 mul(u32 a, u32 b) const {
        const u64 t = (u64)a * b;
        return (u32)(t >> 32) + p - (u32)((u64)((u32)t * pinv) * p >> 32);
    }
    u32 to(u32 a) const { return mul(a, r2); }
    u32 norm(u32 a) const { return a >= p ? a - p : a; }
    u32 from(u32 a) const { return norm(mul(a, 1)); }
    bool eq(u32 a, u32 b) const { return norm(a) == norm(b); }
    u32 add(u32 a, u32 b) const {  // [0, 2p) どうしの和を [0, 2p) に
        const u32 s = a + b;
        return s >= 2 * p ? s - 2 * p : s;
    }
    u32 sub(u32 a, u32 b) const { return a >= b ? a - b : a + 2 * p - b; }
    u32 sel(u32 b, u64 bit) const { return r1 ^ ((r1 ^ b) & (0u - (u32)(bit & 1))); }
    u32 pow(u32 b, u64 e) const {
        u32 r = r1;
        for (; e; e >>= 1, b = mul(b, b)) r = mul(r, sel(b, e));
        return r;
    }
    std::pair<u32, u32> pow2(u32 b, u64 e1, u64 e2) const {
        u32 x = r1, y = r1;
        for (; e1 | e2; e1 >>= 1, e2 >>= 1, b = mul(b, b)) x = mul(x, sel(b, e1)), y = mul(y, sel(b, e2));
        return {x, y};
    }
};

// Jacobi 記号 (a / n)、n は奇数、a < n。二進の形で、分岐は輪の出口だけ。
inline int jacobi(u32 a, u32 n) {
    u32 sg = 0;  // 符号のビット
    while (a) {
        const u32 z = (u32)__builtin_ctz(a);
        a >>= z;
        sg ^= z & ((n >> 1) ^ (n >> 2));  // 2 の冪: z が奇数で n ≡ 3, 5 (mod 8) なら反転
        const u32 d = a - n, lt = (u32)(a < n), m = 0u - lt;
        sg ^= lt & (a & n) >> 1;  // 入れ替え: どちらも ≡ 3 (mod 4) なら反転
        n ^= (a ^ n) & m;         // n = lt ? a : n
        a = (d ^ m) - m;          // a = |a - n| (偶数)
    }
    return n == 1 ? 1 - 2 * (int)(sg & 1) : 0;
}

// 小さい奇素数 q と、q の平方剰余の集合のビット (bit r が立てば r は平方剰余)。q < 64。
struct QrTable {
    u32 q;
    u64 qr;
};
inline constexpr auto QRT = [] {
    std::array<QrTable, 17> t{};
    int k = 0;
    for (u32 q = 3; q < 64; q += 2) {
        bool pr = true;
        for (u32 d = 3; d * d <= q; d += 2) pr &= q % d != 0;
        if (!pr) continue;
        u64 m = 0;
        for (u32 x = 1; x < q; ++x) m |= 1ull << (x * x % q);
        t[k++] = {q, m};
    }
    return t;
}();

template <int STS>
inline i64 sqrt_mod(u32 Y, u32 P) {
    if (Y == 0 || P == 2) return Y;
    const Mont M(P);
    const u32 y = M.to(Y);
    if ((P & 3) == 3) {
        const u32 x = M.pow(y, (P + 1) / 4);
        return M.eq(M.mul(x, x), y) ? (i64)M.from(x) : -1;
    }
    if ((P & 7) == 5) {
        const u32 y2 = M.add(y, y), v = M.pow(y2, (P - 5) / 8);
        const u32 i = M.mul(y2, M.mul(v, v));
        const u32 x = M.mul(M.mul(y, v), M.sub(i, M.r1));
        return M.eq(M.mul(x, x), y) ? (i64)M.from(x) : -1;
    }
    const int s = __builtin_ctz(P - 1);
    const u32 Q = (P - 1) >> s;
    if (s <= STS) {
        // 平方非剰余 z: (q / P) = (P mod q / q)
        u32 z = 0;
        for (const auto& [q, qr] : QRT)
            if (P % q && !(qr >> (P % q) & 1)) {
                z = q;
                break;
            }
        if (!z) {
            z = 67;
            while (jacobi(z, P) != -1) ++z;
        }
        auto [t, R] = M.pow2(y, Q, (Q + 1) / 2);
        // t^{2^{s-1}} ≠ 1 なら平方非剰余
        {
            u32 u = t;
            for (int i = 0; i < s - 1; ++i) u = M.mul(u, u);
            if (!M.eq(u, M.r1)) return -1;
        }
        u32 c = M.pow(M.to(z), Q);
        int m = s;
        while (!M.eq(t, M.r1)) {
            int i = 0;
            u32 u = t;
            while (!M.eq(u, M.r1)) u = M.mul(u, u), ++i;
            u32 b = c;
            for (int j = 0; j < m - i - 1; ++j) b = M.mul(b, b);
            m = i, c = M.mul(b, b), t = M.mul(t, c), R = M.mul(R, b);
        }
        return (i64)M.from(R);
    }
    // Cipolla
    if (jacobi(Y, P) != 1) return -1;  // Y < P
    u32 a = 1, w;
    for (;; ++a) {
        const u32 am = M.to(a);
        w = M.sub(M.mul(am, am), y);  // a^2 - Y (Montgomery 形)
        if (jacobi(M.from(w), P) == -1) break;
    }
    // (a + ω)^{(P+1)/2}、ω^2 = w
    const u32 am = M.to(a);
    u32 x0 = M.r1, x1 = 0, b0 = am, b1 = M.r1;  // 結果と底 (x0 + x1 ω)
    for (u64 e = (P + 1) / 2; e; e >>= 1) {
        // e の下位ビットが 0 なら (1 + 0 ω) を掛ける
        const u32 m = 0u - (u32)(e & 1), c0 = M.r1 ^ ((M.r1 ^ b0) & m), c1 = b1 & m;
        const u32 n0 = M.add(M.mul(x0, c0), M.mul(M.mul(x1, c1), w)), n1 = M.add(M.mul(x0, c1), M.mul(x1, c0));
        x0 = n0, x1 = n1;
        const u32 s0 = M.add(M.mul(b0, b0), M.mul(M.mul(b1, b1), w)), s1 = M.mul(M.add(b0, b0), b1);
        b0 = s0, b1 = s1;
    }
    return (i64)M.from(x0);
}

}  // namespace sq2
