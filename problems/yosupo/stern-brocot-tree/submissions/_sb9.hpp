#pragma once
// Stern-Brocot 木の 9 回目。RANGE を、逆元 1 つと割り切れる割り算 1 つで済ませる。区間の端 f/g と h/k は
// a g - b f = 1、b h - a k = 1、f + h = a、g + k = b を満たす。b が奇数なら g = a^{-1} mod b (b を法とする)、
// f = (a g - 1) / b。b が偶数なら a は奇数なので、h = b^{-1} mod a、k = (b h - 1) / a。どちらも奇数の法 B で割り切れる
// 割り算なので、B^{-1} mod 2^64 を掛けて求まる (B^{-1} は Newton 法でループと並んで作る)。
// 逆元は NeoLibrary の inv_gcd.hpp と同じ Kaliski の形の Stein の方法で、u32 の入力と gcd = 1 に絞る。
#include "../common.hpp"

namespace sb9 {

constexpr int GAP = 8;
inline u64 inv64(u64 n) {
    u64 x = (3 * n) ^ 2, y = 1 - n * x;
    x *= 1 + y, y *= y;
    x *= 1 + y, y *= y;
    x *= 1 + y, y *= y;
    return x * (1 + y);
}
// v 2^{-c} mod M を [0, M] で返す (0 ≤ c ≤ 64、v ≤ M、M は奇数、Minv = M^{-1} mod 2^64)
inline u64 redc(u64 v, int c, u64 M, u64 Minv) {
    u128 t = (u128)v << (64 - c);
    u64 hi = u64(t >> 64), mh = u64((u128)(u64(t) * Minv) * M >> 64);
    return hi >= mh ? hi - mh : hi - mh + M;
}
// A x ≡ 1 (mod B) の x を [1, B] で返す。B は奇数、gcd(A, B) = 1、A, B < 2^31
[[gnu::always_inline]] inline u32 inv_odd(u32 A, u32 B, u64 Binv) {
    if (A > B && __builtin_clz(B) - __builtin_clz(A) > GAP) A %= B;
    if (A == 0) return B;  // B = 1
    int k = __builtin_ctz(A);
    u32 a = B, b = A >> k, ca = 0, cb = 1, neg = 0;
    if (__builtin_clz(b) - __builtin_clz(a) > GAP) {
        const u32 q = a / b, r = a - q * b;
        if (r == 0) a = b, ca = q - 1;
        else {
            const int c = __builtin_ctz(r);
            a = r >> c, ca = q, cb <<= c, k += c;
        }
    }
    while (a != b) {
        const u32 d = a - b, e = b - a;
        const int c = __builtin_ctz(d);
        const u32 mn = a < b ? a : b, t = a > b ? d : e, cm = a > b ? cb : ca;
        neg += a < b;
        ca += cb, cb = cm << c, k += c;
        a = t >> c, b = mn;
    }
    u64 x = redc(cb, k, B, Binv);
    x = x >= B ? x - B : x;
    return (u32)((neg & 1) ? B - x : (x ? x : B));
}

inline void range(u32 a, u32 b, u32* out) {
    const u32 sw = ~b & 1;  // b が偶数なら a を法にする
    u32 m = 0u - sw;
    asm("" : "+r"(m));
    const u32 B = (a & m) | (b & ~m), A = a ^ b ^ B;
    const u64 Binv = inv64(B);
    const u32 x = inv_odd(A, B, Binv);
    const u32 y = (u32)(((u64)A * x - 1) * Binv);
    // sw = 0: (f, g) = (y, x)。sw = 1: (h, k) = (x, y)
    const u32 g = (x & ~m) | ((b - y) & m), f = (y & ~m) | ((a - x) & m);
    out[0] = f, out[1] = g, out[2] = a - f, out[3] = b - g;
}

}  // namespace sb9
