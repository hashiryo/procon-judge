#pragma once
// 原始根の 5 回目。4 回目の二分木で、組を分けるときの 2 つの累乗 b^(右の積) と b^(左の積) を、同じ底の表を共有して
// 1 つの輪で並べて回す (2 本の鎖が互いの掛け算の待ちを埋める)。短いほうの指数は上を 0 で埋めてそろえる。
// 以下は 4 回目の説明。原始根の 4 回目。3 回目の形で、p - 1 を TD 未満の奇素数で先に試し割りしてから、残りを factorize_pairs に渡す。
// yosupo の random のケースでは、97 以下で割った残りが 2^48 以上の合成数になって ECM に回るものが 6 割あり、
// その 58 % はいちばん小さい素因数が 4096 以下だった。試し割りは p の逆元 inv と floor((2^64 - 1) / p) を持ち、
// n inv ≤ lim なら割り切れる (1 個あたり掛け算 1 回)。
// 以下は 3 回目の説明。原始根の 3 回目。2 回目の形で、最初の平方非剰余の候補 g_0 で満たされなかった奇数の q を、q ごとに別の候補で満たす。
// 候補は splitmix64 の擬似乱数 (種は p) で作り、q 1 つについて a^(n / q) ≠ 1 を 1 回の累乗で確かめる (乱数の a なら
// q で落ちる確率は 1 / q)。2 回目は小さい素数を順に木で試していて、小さい素数がどれも 3 乗剰余になるように作った p
// (yosupo の large_least_primitive_root、29 以下の素数の積はすべて q = 3 で落ちる) で候補を 12 個も調べていた。
// 以下は 2 回目の説明。1 回目 (_pr1.hpp) と同じく p - 1 = n を NeoLibrary の factorize_pairs で分解し、候補の試し方を変える。
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

namespace pr5 {
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

// TD 未満の奇素数と、その逆元 (mod 2^64) と floor((2^64 - 1) / p)
template <u32 TD>
struct TrialPrimes {
    std::vector<u64> p, inv, lim;
    TrialPrimes() {
        std::vector<bool> c(TD, false);
        for (u32 i = 3; i < TD; i += 2) {
            if (c[i]) continue;
            for (u32 j = i * i; j < TD; j += 2 * i) c[j] = true;
            u64 x = (3 * (u64)i) ^ 2, y = 1 - (u64)i * x;
            x *= 1 + y, y *= y;
            x *= 1 + y, y *= y;
            x *= 1 + y, y *= y;
            p.push_back(i), inv.push_back(x * (1 + y)), lim.push_back(~0ull / i);
        }
    }
};

// (b^e1, b^e2) を、b の 4 bit の表を共有して 2 本の鎖で並べて求める
inline std::pair<u64, u64> pow_pair(const Mont& m, u64 b, u64 e1, u64 e2) {
    const u64 e = e1 | e2;
    if (e < 16) {
        // 小さい指数は 2 進で
        u64 r1 = m.r1, r2 = m.r1;
        for (int sh = 3; sh >= 0; --sh) {
            r1 = m.mul(r1, r1), r2 = m.mul(r2, r2);
            const u64 t1 = m.mul(r1, b), t2 = m.mul(r2, b);
            r1 = (e1 >> sh & 1) ? t1 : r1, r2 = (e2 >> sh & 1) ? t2 : r2;
        }
        return {r1, r2};
    }
    u64 tab[16];
    tab[0] = m.r1, tab[1] = b;
    for (int i = 2; i < 16; ++i) tab[i] = m.mul(tab[i - 1], b);
    int sh = (63 - __builtin_clzll(e)) & ~3;
    u64 r1 = tab[(e1 >> sh) & 15], r2 = tab[(e2 >> sh) & 15];
    for (sh -= 4; sh >= 0; sh -= 4) {
        r1 = m.mul(r1, r1), r2 = m.mul(r2, r2);
        r1 = m.mul(r1, r1), r2 = m.mul(r2, r2);
        r1 = m.mul(r1, r1), r2 = m.mul(r2, r2);
        r1 = m.mul(r1, r1), r2 = m.mul(r2, r2);
        r1 = m.mul(r1, tab[(e1 >> sh) & 15]), r2 = m.mul(r2, tab[(e2 >> sh) & 15]);
    }
    return {r1, r2};
}
// out[j] = b^(Π q / q[j]) (j < cnt)
inline void tree(const Mont& m, u64 b, const u64* q, int cnt, u64* out) {
    if (cnt == 1) return void(out[0] = b);
    const int half = cnt / 2;
    u64 pl = 1, pr = 1;
    for (int j = 0; j < half; ++j) pl *= q[j];
    for (int j = half; j < cnt; ++j) pr *= q[j];
    const auto [xl, xr] = pow_pair(m, b, pr, pl);
    tree(m, xl, q, half, out);
    tree(m, xr, q + half, cnt - half, out + half);
}

template <u32 TD>
inline u64 primitive_root(u64 p) {
    if (p == 2) return 1;
    const Mont m(p);
    const u64 n = p - 1;
    u64 qs[16], qe[16];  // 奇数の素因数と、その冪 q^e
    int k = 0;
    u64 rad = 1, pow2 = 1;
    // 2 と TD 未満の奇素数で割り、残りを factorize_pairs で分解する (どちらも素数の昇順)
    static const TrialPrimes<TD> tp;
    u64 r = n >> __builtin_ctzll(n);
    pow2 = n / r;
    for (size_t i = 0; i < tp.p.size(); ++i) {
        if (r * tp.inv[i] > tp.lim[i]) continue;
        u64 qq = 1;
        do qq *= tp.p[i], r *= tp.inv[i];
        while (r * tp.inv[i] <= tp.lim[i]);
        qs[k] = tp.p[i], qe[k] = qq, rad *= tp.p[i], ++k;
        if (r < (u64)tp.p[i] * tp.p[i]) break;  // 残りは 1 か素数
    }
    if (r > 1) {
        if (r < (u64)TD * TD) qs[k] = r, qe[k] = r, rad *= r, ++k;
        else
            for (auto [q, e] : factorize_pairs(r)) {
                u64 qq = 1;
                for (int i = 0; i < e; ++i) qq *= q;
                qs[k] = q, qe[k] = qq, rad *= q, ++k;
            }
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
    u64 st = p;  // splitmix64 の状態
    for (int j = 0; j < k; ++j) {
        if (sat0 >> j & 1) continue;
        const u64 e = n / qs[j];
        for (;;) {
            u64 z = (st += 0x9e3779b97f4a7c15ull);
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
            z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
            z ^= z >> 31;
            const u64 am = m.to(z % p);
            if (m.norm(m.norm(m.pow(am, e))) != one && z % p != 0) {
                cand[nc] = am, A[nc] = qe[j], ++nc;
                break;
            }
        }
    }
    u64 res = one;
    for (int i = 0; i < nc; ++i) res = m.mul(res, m.pow(cand[i], n / A[i]));
    return m.norm(m.norm(m.mul(res, 1)));
}

}  // namespace pr5
