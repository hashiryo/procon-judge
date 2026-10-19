#pragma once
// 整数の k 乗根の 1 回目。⌊A^{1/k}⌋ を、浮動小数点の見積もり r と整数での確かめで求める。
// - k = 1 は A、A ≤ 1 は A。k ≥ 41 なら答えは 1 か 2 (3^41 > 2^64) で、A ≥ 2^k かで決まる (k = 64 は 1)。
// - k = 2 は sqrt の切り捨てを r とし、r^2 と (r+1)^2 を A と比べて ±1 直す。r は 2^32 - 1 で止める。
// - 3 ≤ k ≤ 40 は std::pow で r を見積もり、r^k > A なら 1 減らし、(r+1)^k ≤ A なら 1 増やす。k ≥ 7 は答えが 566 以下
//   なので、r^k - 1 (r^k ≥ 2^64 なら 2^64 - 1) の表を引く (表は全部で 1300 項ほど)。r^k > A は r^k - 1 ≥ A と、
//   (r+1)^k ≤ A は (r+1)^k - 1 < A と比べる (A = 2^64 - 1 でも頭打ちの値と取り違えない)。k ≤ 6 は掛け算で r^k を作る。
// A を double にするときは、A ≥ 2^63 なら 1 bit ずらしてから符号付きで変換して 2 倍する (相対誤差 2^-53 で、見積もりには
// 足りる。境目は整数の確かめで直す)。
#include "../common.hpp"

namespace kr1 {

constexpr int KT = 7;  // 表を使う k の下限
struct Tables {
    u32 off[42];
    std::vector<u64> pw;
    double inv[65];
    Tables() {
        for (int k = 1; k <= 64; ++k) inv[k] = 1.0 / k;
        u32 o = 0;
        for (int k = KT; k <= 40; ++k) {
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
            pw.push_back(~0ull), ++o;  // r + 1 を引いても範囲内にする
        }
    }
};
inline const Tables& tables() {
    static const Tables t;
    return t;
}

// r^k を返す。2^64 以上なら 0 を返す (r ≥ 1 なので、呼ぶ側で 1 引くと 2^64 - 1 になる)
inline u64 sat_pow(u64 r, u32 k) {
    u64 p = 1;
    for (u32 i = 0; i < k; ++i)
        if (__builtin_mul_overflow(p, r, &p)) return 0;
    return p;
}

inline double to_double(u64 a) {
    const u64 s = a >> 63;
    return (double)(i64)(a >> s) * (double)(1 + s);
}

inline u64 kth_root(u64 A, u32 k, const Tables& T) {
    if (k == 1 || A <= 1) return A;
    if (k >= 41) return 1 + (k < 64 && A >= (1ull << (k & 63)));
    if (k == 2) {
        u64 r = (u64)std::sqrt(to_double(A));
        if (r > 0xffffffffull) r = 0xffffffffull;
        if (r * r > A) --r;
        else if (r < 0xffffffffull && (r + 1) * (r + 1) <= A) ++r;
        return r;
    }
    u64 r = (u64)std::pow(to_double(A), T.inv[k]);
    if (k >= KT) {
        const u64* p = T.pw.data() + T.off[k];
        if (p[r] >= A) return r - 1;  // r^k > A
        return r + (p[r + 1] < A);    // (r + 1)^k ≤ A
    }
    if (sat_pow(r, k) - 1 >= A) return r - 1;
    return r + (sat_pow(r + 1, k) - 1 < A);
}

inline void run(const std::vector<Query>& qs, u64* out) {
    const Tables& T = tables();
    for (const auto& q : qs) *out++ = kth_root(q.a, q.k, T);
}

}  // namespace kr1
