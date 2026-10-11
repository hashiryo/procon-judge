#pragma once
// 原始根の 1 回目。p - 1 を NeoLibrary の factorize_pairs (97 以下の試し割り、rho、ECM) で分解し、小さい素数 g = 2, 3, 5, ...
// を順に試す。g が原始根であることは、p - 1 のどの素因数 q でも g^((p-1)/q) ≠ 1 であること。q = 2 の条件は g が平方非剰余で
// あることなので、累乗の代わりに Jacobi 記号 (g / p) で見る (平方剰余の g はここで落ちる)。奇数の q は小さい順に
// g^((p-1)/q) を求め、1 になったらその g をやめる。累乗は R = 2^64 の Montgomery で、値は [0, 2p) のまま持つ
// (p < 2^62)。指数は 4 bit ずつ上から見て、g^0 から g^15 の表を引く (分岐なし)。
#include "../common.hpp"
#include "neo/number_theory/factorize.hpp"

namespace pr1 {
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

inline u64 primitive_root(u64 p) {
    if (p == 2) return 1;
    const Mont m(p);
    // p - 1 の奇数の素因数 (小さい順)
    u64 qs[16];
    int k = 0;
    for (auto [q, e] : factorize_pairs(p - 1))
        if (q != 2) qs[k++] = q;
    const u64 one = m.r1 >= p ? m.r1 - p : m.r1;
    auto test = [&](u64 g) {
        if (jacobi(g, p) != -1) return false;
        u64 tab[16];
        tab[0] = one, tab[1] = m.to(g);
        for (int i = 2; i < 16; ++i) tab[i] = m.mul(tab[i - 1], tab[1]);
        for (int j = 0; j < k; ++j) {
            const u64 e = (p - 1) / qs[j];
            int sh = 60;
            while (sh > 0 && (e >> sh) == 0) sh -= 4;
            u64 r = tab[(e >> sh) & 15];
            for (sh -= 4; sh >= 0; sh -= 4) {
                r = m.mul(r, r), r = m.mul(r, r), r = m.mul(r, r), r = m.mul(r, r);
                r = m.mul(r, tab[(e >> sh) & 15]);
            }
            if (m.norm(m.norm(r)) == one) return false;
        }
        return true;
    };
    for (u32 g : SMALL_PRIMES)
        if (test(g)) return g % p;
    for (u64 g = 1000;; ++g)
        if (test(g)) return g % p;
}

}  // namespace pr1
