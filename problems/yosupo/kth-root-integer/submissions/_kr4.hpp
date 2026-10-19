#pragma once
// 整数の k 乗根の 4 回目。3 回目 (_kr3.hpp) の ±1 の直しを分岐なしにする。all_k2 と all_k3_2 と near_border は A の 3 割ほどが
// r^k のすぐそばにあり、double に丸めた A からの見積もりが 1 大きく出るかが半分近くの確率で変わるので、直しの分岐が外れていた
// (all_k2 では 45 % が 1 大きい)。答えは r - (r^k > A) + ((r + 1)^k ≤ A) で、2 つの比べは同時には立たない。
#include "_kr3.hpp"

namespace kr4 {

[[gnu::always_inline]] inline u64 kth_root(u64 A, u32 k, const kr2::Tables& T2, const kr3::Tables& T) {
    if (k == 1 || A <= 1) return A;
    const double x = kr2::to_double(A);
    if (k == 2) {
        u64 r = (u64)(i64)std::sqrt(x);
        r = r < 0xffffffffull ? r : 0xfffffffeull;  // r + 1 ≤ 2^32 - 1 にして (r + 1)^2 を u64 に収める
        const u64 s = r + 1;
        return r - (u64)(r * r > A) + (u64)(s * s <= A);
    }
    const u64 r = (u64)(i64)kr2::root(x, k, T2);
    if (k == 3) {
        const u128 p = (u128)(r * r) * r, s = r + 1, q = (u128)(s * s) * s;
        return r - (u64)(p > A) + (u64)(q <= A);
    }
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
    for (const auto& q : qs) *out++ = kr4::kth_root(q.a, q.k, T2, T);  // 引数の型から kr3::kth_root も見つかるので名前を付ける
}

}  // namespace kr4
