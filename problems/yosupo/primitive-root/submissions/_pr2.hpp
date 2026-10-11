#pragma once
// 原始根の 2 回目。1 回目 (_pr1.hpp) と同じく p - 1 = n を NeoLibrary の factorize_pairs で分解し、候補の試し方を変える。
//
// 候補 g ごとに、奇数の素因数 q_1, ..., q_k のすべてで g^(n / q_j) を二分木で一度に求める。M = n / (q_1 ... q_k) として
// h = g^M を作り、素因数の組を半分に分けて、左の組には h を右の組の積で上げた値を、右の組には左の組の積で上げた値を
// 渡す (組が 1 つになったらそれが g^(n / q))。指数のビット数の合計は log2(q_1 ... q_k) × 段の数で、k 本の累乗を
// 別々に求めるより少ない。
//
// q = 2 は Jacobi 記号で、小さい素数から最初の平方非剰余 g を探す。g が奇数の q をすべて満たせば g を返す。満たさない
// q があれば、次の小さい素数 (平方剰余でもよい) で木を求めて、満たす q を足していく。すべての q がどれかの候補で
// 満たされたら、q を満たす候補ごとに q^e の積 A_i を作り、Π g_i^(n / A_i) を返す (g_i^(n / A_i) の位数は A_i ちょうどで、
// A_i どうしは互いに素なので、積の位数は n)。最小の原始根が大きい p でも、小さい q はそれぞれ小さい候補で満たされる。
#include "../common.hpp"
#include "neo/number_theory/factorize.hpp"

namespace pr2 {
using u128 = unsigned __int128;

struct Mont {
    u64 n, ninv, r1, r2;  // r1 = 2^64 mod n、r2 = 2^128 mod n
    explicit Mont(u64 n_) : n(n_) {
        u64 x = (3 * n) ^ 2, y = 1 - n * x;
        x *= 1 + y, y *= y;
        x *= 1 + y, y *= y;
        x *= 1 + y, y *= y;
        ninv = x * (1 + y);
        r1 = (u64)(((u128)1 << 64) % n);
        r2 = (u64)((u128)r1 * r1 % n);
    }
    // a b / 2^64 mod n を [0, 2n) で返す (a, b < 2n、n < 2^62)
    u64 mul(u64 a, u64 b) const {
        const u128 t = (u128)a * b;
        return (u64)(t >> 64) + n - (u64)((u128)((u64)t * ninv) * n >> 64);
    }
    u64 to(u64 a) const { return mul(a, r2); }
    u64 norm(u64 a) const { return a >= n ? a - n : a; }
    // b^e (b は Montgomery 形、[0, 2n))。e ≥ 2^16 なら 4 bit の窓、それより小さければ 2 進で、どちらも分岐なし。
    u64 pow(u64 b, u64 e) const {
        if (e == 0) return r1;
        if (e < (1u << 16)) {
            int sh = 63 - __builtin_clzll(e);
            u64 r = b;
            for (--sh; sh >= 0; --sh) {
                r = mul(r, r);
                const u64 t = mul(r, b);
                r = (e >> sh & 1) ? t : r;
            }
            return r;
        }
        u64 tab[16];
        tab[0] = r1, tab[1] = b;
        for (int i = 2; i < 16; ++i) tab[i] = mul(tab[i - 1], b);
        int sh = (63 - __builtin_clzll(e)) & ~3;
        u64 r = tab[(e >> sh) & 15];
        for (sh -= 4; sh >= 0; sh -= 4) {
            r = mul(r, r), r = mul(r, r), r = mul(r, r), r = mul(r, r);
            r = mul(r, tab[(e >> sh) & 15]);
        }
        return r;
    }
};

// Jacobi 記号 (a / n)。n は奇数。
inline int jacobi(u64 a, u64 n) {
    int s = 1;
    a %= n;
    while (a) {
        const int z = __builtin_ctzll(a);
        a >>= z;
        if ((z & 1) && ((n & 7) == 3 || (n & 7) == 5)) s = -s;
        if ((a & 3) == 3 && (n & 3) == 3) s = -s;
        std::swap(a, n);
        a %= n;
    }
    return n == 1 ? s : 0;
}

inline constexpr std::array<u32, 168> SMALL_PRIMES = [] {
    std::array<u32, 168> t{};
    u32 k = 0;
    for (u32 v = 2; k < 168; ++v) {
        bool pr = true;
        for (u32 d = 2; d * d <= v; ++d) pr &= v % d != 0;
        if (pr) t[k++] = v;
    }
    return t;
}();

// out[j] = b^(Π q / q[j]) (j < cnt)
inline void tree(const Mont& m, u64 b, const u64* q, int cnt, u64* out) {
    if (cnt == 1) return void(out[0] = b);
    const int half = cnt / 2;
    u64 pl = 1, pr = 1;
    for (int j = 0; j < half; ++j) pl *= q[j];
    for (int j = half; j < cnt; ++j) pr *= q[j];
    tree(m, m.pow(b, pr), q, half, out);
    tree(m, m.pow(b, pl), q + half, cnt - half, out + half);
}

inline u64 primitive_root(u64 p) {
    if (p == 2) return 1;
    const Mont m(p);
    const u64 n = p - 1;
    u64 qs[16], qe[16];  // 奇数の素因数と、その冪 q^e
    int k = 0;
    u64 rad = 1, pow2 = 1;
    for (auto [q, e] : factorize_pairs(n)) {
        u64 qq = 1;
        for (int i = 0; i < e; ++i) qq *= q;
        if (q == 2) pow2 = qq;
        else qs[k] = q, qe[k] = qq, rad *= q, ++k;
    }
    const u64 one = m.norm(m.r1), M = n / rad;
    // i 番目の候補: 小さい素数の表、その先は整数
    auto cand_at = [](u32 i) -> u64 { return i < SMALL_PRIMES.size() ? SMALL_PRIMES[i] : (u64)i; };
    // 候補 g が満たす奇数の q の集合
    auto sat_of = [&](u64 gm) {
        u64 out[16];
        tree(m, m.pow(gm, M), qs, k, out);
        u32 sat = 0;
        for (int j = 0; j < k; ++j) sat |= (u32)(m.norm(m.norm(out[j])) != one) << j;
        return sat;
    };
    // q = 2: 最初の平方非剰余
    u32 gi = 0;
    while (jacobi(cand_at(gi), p) != -1) ++gi;
    const u64 g0 = cand_at(gi) % p;
    if (k == 0) return g0;
    const u32 all = (1u << k) - 1;
    u64 cand[17], A[17];  // 使う候補 (Montgomery 形) と、それが受け持つ冪の積
    cand[0] = m.to(g0);
    const u32 sat0 = sat_of(cand[0]);
    if (sat0 == all) return g0;
    A[0] = pow2;  // 最初の候補は q = 2 も受け持つ
    for (int j = 0; j < k; ++j)
        if (sat0 >> j & 1) A[0] *= qe[j];
    int nc = 1;
    u32 need = all & ~sat0;  // まだ満たされていない奇数の q
    for (u32 i = 0; need; ++i) {
        if (i == gi || cand_at(i) % p == 0) continue;
        const u64 gm = m.to(cand_at(i) % p);
        const u32 add = sat_of(gm) & need;
        if (!add) continue;
        u64 a = 1;
        for (int j = 0; j < k; ++j)
            if (add >> j & 1) a *= qe[j];
        cand[nc] = gm, A[nc] = a, ++nc;
        need &= ~add;
    }
    u64 r = one;
    for (int i = 0; i < nc; ++i) r = m.mul(r, m.pow(cand[i], n / A[i]));
    return m.norm(m.norm(m.mul(r, 1)));
}

}  // namespace pr2
