#pragma once
// 平方根 mod p の 4 回目。3 回目の Jacobi 記号の 1 段を、差 d = a - n の ctz を取る形にする。|d| と d の末尾の 0 の数は
// 同じなので、ctz を |d| と min(a, n) の選択と並べて進められ、1 段の依存が引き算、ctz、シフトの 3 つになる (3 回目は
// ctz、シフト、引き算、選択の 4 つ)。輪は a = n で止める。梯子の V_k^2 - 2Q'^k などは、Montgomery の掛け算の上位の項に
// p - 2Q'^k を先に足しておき、引き算と補正を 1 回にまとめる (mulsub)。以下は 3 回目の説明。
//
// 平方根 mod p の 3 回目。2 回目の Cipolla (P - 1 の 2 の冪が大きいときの道) を Lucas 列の梯子に替える。
//
// a^2 - Y が平方非剰余なら、α = 1 + ω / a (ω^2 = a^2 - Y) は F_P^2 の元で、和が 2、積 (ノルム) が Y / a^2 になる。
// α^{(P+1)/2} は F_P に入り、2 乗するとノルム Y / a^2 なので、Y の平方根は a α^{(P+1)/2} = a V_{(P+1)/2} / 2 となる
// (V_n = α^n + ᾱ^n は P' = 2、Q' = Y / a^2 の Lucas 列)。V_n は (V_k, V_{k+1}, Q'^k, Q'^{k+1}) の梯子で求め、1 段の掛け算は
// V_k^2 か V_{k+1}^2、V_k V_{k+1}、Q'^k か Q'^{k+1} の 2 乗、Q'^k Q'^{k+1} の 4 回で、どれも前の段から 1 回ぶんの深さしかない
// (Cipolla の F_P^2 の累乗は 1 段 9 回で深さ 2)。P' = 2 にするため a は 2 の冪に限り、Y / a^2 は Y を半分にして作る。
// 8 回までの a で見つからなければ 2 回目の Cipolla に回す。
//
// 平方剰余かの判定と a の探し方は Jacobi 記号。PAIRY なら (Y / P) と (1 - Y / P) を、そうでなければ候補の a を 2 つずつ並べて
// 求める (二進の Jacobi 記号は 1 回の段の依存が長いので、2 本を交互に進めると 1 本ぶんに近い時間で済む)。
// CMOV なら x86 で Jacobi 記号の 1 段の |a - n| と min(a, n) を cmov で書く (gcc は三項演算子を分岐にし、マスクで書くと
// setb、neg、xor、add の 4 段になる)。BRL なら梯子の選択をビットの分岐にする (P が同じ問いが続くと当たる)。
// Montgomery の定数は pinv を 3 回の Newton 法で、R mod P を 32 bit の割り算で作り、Y は (Y << 32) mod P で Montgomery 形にする。
// Tonelli-Shanks の道は、t = Y^Q、R = Y^{(Q+1)/2}、c = z^Q を 1 つの輪で求める。P ≡ 3 (mod 4)、P ≡ 5 (mod 8) の道は 2 回目と同じ。
#include "../common.hpp"

namespace sq4 {

struct Mont {
    u32 p, pinv, r1;  // pinv = p^{-1} mod 2^32、r1 = 2^32 mod p
    explicit Mont(u32 p_) : p(p_) {
        u32 x = (3 * p) ^ 2;  // 下位 5 ビットが正しい
        for (int i = 0; i < 3; ++i) x *= 2 - p * x;
        pinv = x, r1 = (0u - p) % p;
    }
    u32 mul(u32 a, u32 b) const {
        const u64 t = (u64)a * b;
        return (u32)(t >> 32) + p - (u32)((u64)((u32)t * pinv) * p >> 32);
    }
    u32 to(u32 a) const { return (u32)(((u64)a << 32) % p); }
    u32 norm(u32 a) const { return a >= p ? a - p : a; }
    u32 from(u32 a) const { return norm(mul(a, 1)); }
    bool eq(u32 a, u32 b) const { return norm(a) == norm(b); }
    // [0, 2p) どうしの和と差を [0, 2p) に。補正はマスクで足す
    u32 add(u32 a, u32 b) const {
        const u32 t = a + b - 2 * p;
        return t + (2 * p & (u32)((int)t >> 31));
    }
    u32 sub(u32 a, u32 b) const { return a - b + (2 * p & (0u - (u32)(a < b))); }
    u32 half(u32 a) const { return (a + (p & (0u - (a & 1)))) >> 1; }
    // a b R^{-1} - c を [0, 2p) に (c ∈ [0, 2p))。和は (-2p, 2p) に入るので、負なら 2p を足す
    u32 mulsub(u32 a, u32 b, u32 c) const {
        const u64 t = (u64)a * b;
        const u32 r = (u32)(t >> 32) + (p - c) - (u32)((u64)((u32)t * pinv) * p >> 32);
        return r + (2 * p & (u32)((int)r >> 31));
    }
    u32 sel(u32 b, u64 bit) const { return r1 ^ ((r1 ^ b) & (0u - (u32)(bit & 1))); }
    u32 pow(u32 b, u64 e) const {
        u32 r = r1;
        for (; e; e >>= 1, b = mul(b, b)) r = mul(r, sel(b, e));
        return r;
    }
    // b^e1、b^e2、c^e1 (e2 ≤ e1)
    std::array<u32, 3> pow3(u32 b, u32 c, u64 e1, u64 e2) const {
        u32 x = r1, y = r1, w = r1;
        for (; e1; e1 >>= 1, e2 >>= 1, b = mul(b, b), c = mul(c, c))
            x = mul(x, sel(b, e1)), y = mul(y, sel(b, e2)), w = mul(w, sel(c, e1));
        return {x, y, w};
    }
};

// 二進の Jacobi 記号の 1 段。a と n は奇数で a ≠ n。sg のビット 0 が符号。
// (a / n) は、a > n なら (a - n / n)、a < n なら相互法則で (n - a / a) に符号 (a ≡ n ≡ 3 (mod 4) なら反転) を掛けたもの。
// |a - n| の 2 の冪を外すと、z が奇数で新しい n が ≡ 3, 5 (mod 8) のとき反転する。
template <bool CMOV>
inline void jacobi_step(u32& a, u32& n, u32& sg) {
    u32 d = a - n;
    const u32 z = (u32)__builtin_ctz(d);
    sg ^= (u32)(a < n) & (a & n) >> 1;
    u32 nn = n;
#if defined(__x86_64__)
    if constexpr (CMOV) {
        const u32 e = n - a;
        asm("cmp %[n], %[a]\n\tcmovb %[e], %[d]\n\tcmovb %[a], %[nn]" : [d] "+&r"(d), [nn] "+&r"(nn) : [a] "r"(a), [n] "r"(n), [e] "r"(e) : "cc");
    } else
#endif
    {
        const u32 m = 0u - (u32)(a < n);
        nn ^= (a ^ n) & m;  // min(a, n)
        d = (d ^ m) - m;    // |a - n|
    }
    sg ^= z & ((nn >> 1) ^ (nn >> 2));
    a = d >> z, n = nn;
}
inline int jacobi_sign(u32 n, u32 sg) { return n == 1 ? 1 - 2 * (int)(sg & 1) : 0; }
// a の 2 の冪を外す (a は 0 でない)
inline u32 jacobi_init(u32& a, u32 n) {
    const u32 z = (u32)__builtin_ctz(a);
    a >>= z;
    return z & ((n >> 1) ^ (n >> 2));
}

// Jacobi 記号 (a / n)、n は奇数、a < n
template <bool CMOV = false>
inline int jacobi(u32 a, u32 n) {
    if (!a) return n == 1;
    u32 sg = jacobi_init(a, n);
    while (a != n) jacobi_step<CMOV>(a, n, sg);
    return jacobi_sign(n, sg);
}

// (a1 / n) と (a2 / n) を 2 本並べて求める。n は奇数、a1, a2 < n
template <bool CMOV = false>
inline std::pair<int, int> jacobi2(u32 a1, u32 a2, u32 n) {
    if (!a1 || !a2) return {jacobi<CMOV>(a1, n), jacobi<CMOV>(a2, n)};
    u32 n1 = n, n2 = n, s1 = jacobi_init(a1, n), s2 = jacobi_init(a2, n);
    while (a1 != n1 && a2 != n2) jacobi_step<CMOV>(a1, n1, s1), jacobi_step<CMOV>(a2, n2, s2);
    while (a1 != n1) jacobi_step<CMOV>(a1, n1, s1);
    while (a2 != n2) jacobi_step<CMOV>(a2, n2, s2);
    return {jacobi_sign(n1, s1), jacobi_sign(n2, s2)};
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

// P' = 2、Q' = q (Montgomery 形) の Lucas 列の V_e (e は 3 以上の奇数)
template <bool BRL = false>
inline u32 lucas_v(const Mont& M, u32 q, u32 e) {
    const u32 two = M.add(M.r1, M.r1);
    u32 v0 = two, v1 = M.sub(M.add(two, two), M.add(q, q)), q0 = q, q1 = M.mul(q, q);  // k = 1
    for (int i = 30 - __builtin_clz(e); i > 0; --i) {
        if constexpr (BRL) {
            const u32 mx = M.mulsub(v0, v1, M.add(q0, q0)), qmx = M.mul(q0, q1);
            if (e >> i & 1) {
                v1 = M.mulsub(v1, v1, M.add(q1, q1)), q1 = M.mul(q1, q1);
                v0 = mx, q0 = qmx;
            } else {
                v0 = M.mulsub(v0, v0, M.add(q0, q0)), q0 = M.mul(q0, q0);
                v1 = mx, q1 = qmx;
            }
            continue;
        }
        const u32 m = 0u - ((e >> i) & 1);
        const u32 s = v0 ^ ((v0 ^ v1) & m), qs = q0 ^ ((q0 ^ q1) & m);  // ビットが 1 なら k + 1 の側
        const u32 sq = M.mulsub(s, s, M.add(qs, qs)), mx = M.mulsub(v0, v1, M.add(q0, q0));
        const u32 qsq = M.mul(qs, qs), qmx = M.mul(q0, q1);
        const u32 dv = (sq ^ mx) & m, dq = (qsq ^ qmx) & m;
        v0 = sq ^ dv, v1 = mx ^ dv, q0 = qsq ^ dq, q1 = qmx ^ dq;
    }
    return M.mulsub(v0, v1, M.add(q0, q0));  // e の最下位ビットは 1
}

// 2 回目の Cipolla (a を 1 から順に探す)。Y は平方剰余
inline i64 cipolla(const Mont& M, u32 y, u32 P) {
    const u32 r2 = M.to(M.r1);
    u32 a = 1, w;
    for (;; ++a) {
        const u32 am = M.mul(a, r2);
        w = M.sub(M.mul(am, am), y);
        if (jacobi(M.from(w), P) == -1) break;
    }
    const u32 am = M.mul(a, r2);
    u32 x0 = M.r1, x1 = 0, b0 = am, b1 = M.r1;
    for (u64 e = (P + 1) / 2; e; e >>= 1) {
        const u32 m = 0u - (u32)(e & 1), c0 = M.r1 ^ ((M.r1 ^ b0) & m), c1 = b1 & m;
        const u32 n0 = M.add(M.mul(x0, c0), M.mul(M.mul(x1, c1), w)), n1 = M.add(M.mul(x0, c1), M.mul(x1, c0));
        x0 = n0, x1 = n1;
        const u32 s0 = M.add(M.mul(b0, b0), M.mul(M.mul(b1, b1), w)), s1 = M.mul(M.add(b0, b0), b1);
        b0 = s0, b1 = s1;
    }
    return (i64)M.from(x0);
}

template <int STS, bool PAIRY, bool CMOV = false, bool BRL = false>
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
        auto [t, R, c] = M.pow3(y, M.to(z), Q, (Q + 1) / 2);
        // t^{2^{s-1}} ≠ 1 なら平方非剰余
        {
            u32 u = t;
            for (int i = 0; i < s - 1; ++i) u = M.mul(u, u);
            if (!M.eq(u, M.r1)) return -1;
        }
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
    // Lucas 列。a = 2^j で (a^2 - Y) / P = -1 となる j を探す (候補 a^2 - Y を [0, P) の整数で持つ)
    auto cand = [&](int j) {
        const u32 c = (u32)((1ull << (2 * j)) % P);
        return c >= Y ? c - Y : c + P - Y;
    };
    int j = -1;
    if constexpr (PAIRY) {
        const auto [jy, j0] = jacobi2<CMOV>(Y, cand(0), P);
        if (jy != 1) return -1;
        if (j0 == -1) j = 0;
        for (int k = 1; j < 0 && k < 8; k += 2) {
            const auto [e1, e2] = jacobi2<CMOV>(cand(k), cand(k + 1), P);
            j = e1 == -1 ? k : e2 == -1 ? k + 1 : -1;
        }
    } else {
        if (jacobi<CMOV>(Y, P) != 1) return -1;
        for (int k = 0; j < 0 && k < 8; k += 2) {
            const auto [e1, e2] = jacobi2<CMOV>(cand(k), cand(k + 1), P);
            j = e1 == -1 ? k : e2 == -1 ? k + 1 : -1;
        }
    }
    if (j < 0) return cipolla(M, y, P);
    u32 q = y;
    for (int i = 0; i < 2 * j; ++i) q = M.half(q);  // Y / a^2
    const u32 v = lucas_v<BRL>(M, q, (P + 1) / 2);
    // 平方根は a V / 2 = 2^{j-1} V
    u32 x = j == 0 ? M.half(v) : v;
    for (int i = 1; i < j; ++i) x = M.add(x, x);
    return (i64)M.from(x);
}

}  // namespace sq4
