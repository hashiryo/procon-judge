#pragma once
// 整数の k 乗根の 3 回目。2 回目 (_kr2.hpp) の見積もりはそのままで、まわりの手間を減らす。
// - gcc は kth_root を関数のまま呼んでいた (1 問ごとに push と pop が 5 つずつ)。always_inline で run のループに展開させる。
// - k = 3 と k = 4 は r^k と (r + 1)^k を掛け算で書き下す (2 回目は k を回す u128 のループだった)。k ≥ 5 は r^k - 1 の表
//   (k = 5 は 7133 項、全部で 1 万項ほど) を引く。
// - 見積もりの double は 2^32 未満なので、u64 への変換を符号付きの変換 1 つにする (2^63 以上を分ける比べと分岐が要らない)。
#include "_kr2.hpp"

namespace kr3 {

constexpr int KT = 5;
struct Tables {
    u32 off[65];
    std::vector<u64> pw;
    Tables() {
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

[[gnu::always_inline]] inline u64 kth_root(u64 A, u32 k, const kr2::Tables& T2, const Tables& T) {
    if (k == 1 || A <= 1) return A;
    const double x = kr2::to_double(A);
    if (k == 2) {
        u64 r = (u64)(i64)std::sqrt(x);
        if (r > 0xffffffffull) r = 0xffffffffull;
        if (r * r > A) return r - 1;
        return r + (r < 0xffffffffull && (r + 1) * (r + 1) <= A);
    }
    const u64 r = (u64)(i64)kr2::root(x, k, T2);
    if (k == 3) {
        const u128 p = (u128)(r * r) * r, s = r + 1, q = (u128)(s * s) * s;
        if (p > A) return r - 1;
        return r + (q <= A);
    }
    if (k == 4) {
        const u64 r2 = r * r, s = r + 1, s2 = s * s;
        const u128 p = (u128)r2 * r2, q = (u128)s2 * s2;
        if (p > A) return r - 1;
        return r + (q <= A);
    }
    const u64* p = T.pw.data() + T.off[k];
    if (p[r] >= A) return r - 1;  // r^k > A
    return r + (p[r + 1] < A);    // (r + 1)^k ≤ A
}

inline void run(const std::vector<Query>& qs, u64* out) {
    const kr2::Tables& T2 = kr2::tables();
    const Tables& T = tables();
    for (const auto& q : qs) *out++ = kth_root(q.a, q.k, T2, T);
}

}  // namespace kr3
