#pragma once
// 整数の k 乗根の 6 回目。5 回目 (_kr5.hpp) の k ≥ 4 の見積もりで、log2 と exp2 の多項式を 4 次から 2 次に落とす。k ≥ 4 なら
// 答えは 65536 未満なので、相対誤差 2^-20 ほど (log2 が 2^-25、exp2 が 2^-21) でも ⌊見積もり⌋ は真の値から ±1 以内に収まる。
#include "_kr5.hpp"

namespace kr6 {

// x^(1/k) (x ≥ 1、k ≥ 4)。相対誤差 2^-20 ほど
[[gnu::always_inline]] inline double root_lo(double x, u32 k, const kr2::Tables& T) {
    constexpr double L1 = 1.4426950408889634, L2 = -0.7213475204444817;
    const u64 b = std::bit_cast<u64>(x);
    const int e = (int)(b >> 52) - 1023;
    const u32 j = (u32)(b >> 45) & 127;
    const double m = std::bit_cast<double>((b & 0x000fffffffffffffull) | 0x3ff0000000000000ull);
    const double r = m * T.inv[j] - 1.0;
    const double lg = (double)e + T.lg[j] + r * (L1 + r * L2);
    const double y = lg * T.invk[k];
    const i64 n = (i64)(y * 64.0);
    const double f = (y - (double)n * (1.0 / 64)) * 0.6931471805599453;
    const double p = 1.0 + f * (1.0 + f * 0.5);
    const double v = T.ex[n & 63] * p;
    return std::bit_cast<double>(std::bit_cast<u64>(v) + ((u64)(n >> 6) << 52));
}

[[gnu::always_inline]] inline u64 kth_root(u64 A, u32 k, const kr2::Tables& T2, const kr3::Tables& T, const kr5::Cbrt& C) {
    if (k == 1 || A <= 1) return A;
    const double x = kr2::to_double(A);
    if (k == 2) {
        u64 r = (u64)(i64)std::sqrt(x);
        r = r < 0xffffffffull ? r : 0xfffffffeull;
        const u64 s = r + 1;
        return r - (u64)(r * r > A) + (u64)(s * s <= A);
    }
    if (k == 3) {
        const u64 r = (u64)(i64)kr5::cbrt3(x, C);
        const u128 p = (u128)(r * r) * r, s = r + 1, q = (u128)(s * s) * s;
        return r - (u64)(p > A) + (u64)(q <= A);
    }
    const u64 r = (u64)(i64)root_lo(x, k, T2);
    if (k == 4) {
        const u64 r2 = r * r, s = r + 1, s2 = s * s;
        const u128 p = (u128)r2 * r2, q = (u128)s2 * s2;
        return r - (u64)(p > A) + (u64)(q <= A);
    }
    const u64* p = T.pw.data() + T.off[k];
    return r - (u64)(p[r] >= A) + (u64)(p[r + 1] < A);
}

inline void run(const std::vector<Query>& qs, u64* out) {
    const kr2::Tables& T2 = kr2::tables();
    const kr3::Tables& T = kr3::tables();
    const kr5::Cbrt& C = kr5::cbrt_table();
    for (const auto& q : qs) *out++ = kr6::kth_root(q.a, q.k, T2, T, C);
}

}  // namespace kr6
