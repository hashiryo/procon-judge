#pragma once
// 整数の k 乗根の 2 回目。1 回目 (_kr1.hpp) の std::pow を、表と短い多項式の log2 と exp2 に替え、k ≥ 41 も r^k の表で
// 同じ道に通す (k による分岐を減らす)。
// - log2(x): x = 2^e m (1 ≤ m < 2) の m の上位 7 bit で c_j = 1 + (j + 1/2)/128 を選び、r = m / c_j - 1 (|r| < 2^-8) の
//   log2(1 + r) を 4 次の多項式で出して、e + log2(c_j) に足す (誤差 2^-40 ほど)。
// - exp2(y): y を 1/64 刻みの n/64 と端数 f (0 ≤ f < 1/64) に分け、2^(n mod 64 / 64) を表で引き、2^f を 4 次の多項式で出し、
//   2^(n / 64) は指数部に足す。
// 見積もり A^{1/k} = exp2(log2(A) / k) の相対誤差は 2^-38 ほどで、どの k でも r = ⌊見積もり⌋ は真の値から ±1 以内。
// 直しは 1 回目と同じで、k ≥ 7 は r^k の表、3 ≤ k ≤ 6 は u128 の掛け算で r^k と (r + 1)^k を作る。
#include "../common.hpp"

namespace kr2 {

constexpr int KT = 7;
struct Tables {
    double lg[128], inv[128], ex[64], invk[65];
    u32 off[65];
    std::vector<u64> pw;
    Tables() {
        for (int j = 0; j < 128; ++j) {
            const double c = 1.0 + (j + 0.5) / 128;
            lg[j] = std::log2(c), inv[j] = 1.0 / c;
        }
        for (int j = 0; j < 64; ++j) ex[j] = std::exp2(j / 64.0);
        for (int k = 1; k <= 64; ++k) invk[k] = 1.0 / k;
        u32 o = 0;
        for (int k = KT; k <= 64; ++k) {
            off[k] = o;
            for (u64 r = 0;; ++r) {
                u128 p = 1;
                bool sat = false;
                for (int i = 0; i < k; ++i) {
                    p *= r;
                    if (p >> 64) { sat = true; break; }
                }
                pw.push_back(sat ? ~0ull : (u64)p - 1), ++o;  // r^k - 1 (r^k ≥ 2^64 なら 2^64 - 1)
                if (sat) break;
            }
            pw.push_back(~0ull), ++o;
        }
    }
};
inline const Tables& tables() {
    static const Tables t;
    return t;
}

inline double to_double(u64 a) {
    const u64 s = a >> 63;
    return (double)(i64)(a >> s) * (double)(1 + s);
}

// x^(1/k) (x ≥ 1)
inline double root(double x, u32 k, const Tables& T) {
    constexpr double L1 = 1.4426950408889634, L2 = -0.7213475204444817, L3 = 0.4808983469629878, L4 = -0.36067376022224085;
    const u64 b = std::bit_cast<u64>(x);
    const int e = (int)(b >> 52) - 1023;
    const u32 j = (u32)(b >> 45) & 127;
    const double m = std::bit_cast<double>((b & 0x000fffffffffffffull) | 0x3ff0000000000000ull);
    const double r = m * T.inv[j] - 1.0;
    const double lg = (double)e + T.lg[j] + r * (L1 + r * (L2 + r * (L3 + r * L4)));
    const double y = lg * T.invk[k];
    const i64 n = (i64)(y * 64.0);
    const double f = (y - (double)n * (1.0 / 64)) * 0.6931471805599453;
    const double p = 1.0 + f * (1.0 + f * (0.5 + f * (1.0 / 6 + f * (1.0 / 24))));
    const double v = T.ex[n & 63] * p;
    return std::bit_cast<double>(std::bit_cast<u64>(v) + ((u64)(n >> 6) << 52));
}

inline u64 kth_root(u64 A, u32 k, const Tables& T) {
    if (k == 1 || A <= 1) return A;
    if (k == 2) {
        u64 r = (u64)std::sqrt(to_double(A));
        if (r > 0xffffffffull) r = 0xffffffffull;
        if (r * r > A) --r;
        else if (r < 0xffffffffull && (r + 1) * (r + 1) <= A) ++r;
        return r;
    }
    const u64 r = (u64)root(to_double(A), k, T);
    if (k >= KT) {
        const u64* p = T.pw.data() + T.off[k];
        if (p[r] >= A) return r - 1;  // r^k > A
        return r + (p[r + 1] < A);    // (r + 1)^k ≤ A
    }
    u128 p = r, q = r + 1;
    for (u32 i = 1; i < k; ++i) p *= r, q *= r + 1;
    if (p > A) return r - 1;
    return r + (q <= A);
}

inline void run(const std::vector<Query>& qs, u64* out) {
    const Tables& T = tables();
    for (const auto& q : qs) *out++ = kth_root(q.a, q.k, T);
}

}  // namespace kr2
