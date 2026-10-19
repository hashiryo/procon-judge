#pragma once
// 整数の k 乗根の 5 回目。4 回目 (_kr4.hpp) の k = 3 を、専用の立方根にする。x = 2^e m (1 ≤ m < 2) で、e = 3q + s
// (s = 0, 1, 2) とすると x^{1/3} = 2^q (2^s m)^{1/3}。m の上位 7 bit で c_j = 1 + (j + 1/2)/128 を選び、(2^s c_j)^{1/3} と
// 1/c_j を表で引いて、r = m / c_j - 1 (|r| < 2^-8) の (1 + r)^{1/3} を 3 次の多項式 1 + r/3 - r^2/9 + 5r^3/81 で出す
// (誤差 2^-36 ほど)。log2 と exp2 を通る一般の形 (多項式 2 つと表 2 つ) より演算が半分ほどで済む。
#include "_kr4.hpp"

namespace kr5 {

struct Cbrt {
    double c[3][128], inv[128];
    Cbrt() {
        for (int j = 0; j < 128; ++j) {
            const double cj = 1.0 + (j + 0.5) / 128;
            inv[j] = 1.0 / cj;
            for (int s = 0; s < 3; ++s) c[s][j] = std::cbrt(std::ldexp(cj, s));
        }
    }
};
inline const Cbrt& cbrt_table() {
    static const Cbrt t;
    return t;
}

// x^(1/3) (1 ≤ x < 2^64)
[[gnu::always_inline]] inline double cbrt3(double x, const Cbrt& C) {
    const u64 b = std::bit_cast<u64>(x);
    const u32 e = (u32)(b >> 52) - 1023, q = e / 3, s = e - 3 * q, j = (u32)(b >> 45) & 127;
    const double m = std::bit_cast<double>((b & 0x000fffffffffffffull) | 0x3ff0000000000000ull);
    const double r = m * C.inv[j] - 1.0;
    const double p = 1.0 + r * (1.0 / 3 + r * (-1.0 / 9 + r * (5.0 / 81)));
    const double v = C.c[s][j] * p;
    return std::bit_cast<double>(std::bit_cast<u64>(v) + ((u64)q << 52));
}

[[gnu::always_inline]] inline u64 kth_root(u64 A, u32 k, const kr2::Tables& T2, const kr3::Tables& T, const Cbrt& C) {
    if (k == 3 && A > 1) {
        const u64 r = (u64)(i64)cbrt3(kr2::to_double(A), C);
        const u128 p = (u128)(r * r) * r, s = r + 1, q = (u128)(s * s) * s;
        return r - (u64)(p > A) + (u64)(q <= A);
    }
    return kr4::kth_root(A, k, T2, T);
}

inline void run(const std::vector<Query>& qs, u64* out) {
    const kr2::Tables& T2 = kr2::tables();
    const kr3::Tables& T = kr3::tables();
    const Cbrt& C = cbrt_table();
    for (const auto& q : qs) *out++ = kr5::kth_root(q.a, q.k, T2, T, C);
}

}  // namespace kr5
