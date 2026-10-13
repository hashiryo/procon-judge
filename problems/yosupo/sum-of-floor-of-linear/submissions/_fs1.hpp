#pragma once
// 線形関数の床の和の 1 回目。Σ_{i=0}^{n-1} ⌊(a i + b) / m⌋ を AtCoder Library の floor_sum と同じ Euclid の互除法の形で求める。
//   a ≥ m なら n (n-1) / 2 ⌊a/m⌋ を足して a を a mod m に、b ≥ m なら n ⌊b/m⌋ を足して b を b mod m にする。
//   y = a n + b < m なら終わり。そうでなければ n = ⌊y/m⌋、b = y mod m とし、(a, m) を (m, a) に入れ替えて繰り返す。
// 段の始めの n、m、a、b はどれも 32 bit に収まる (次の n = ⌊(a n + b) / m⌋ は a < m なので n 以下)。V は割り算の形:
//   0: すべて 64 bit (AtCoder Library のまま)
//   1: a / m、b / m は 32 bit の割り算、y / m は 64 bit
//   2: 1 に加え、y / m を x86 の divl (64 bit を 32 bit で割り、商が 32 bit に収まるとき) で
#include "../common.hpp"

namespace fs1 {

// y / m と y % m (y / m < 2^32)
template <int V>
inline void divmod(u64 y, u32 m, u32& q, u32& r) {
#if defined(__x86_64__)
    if constexpr (V == 2) {
        asm("divl %4" : "=a"(q), "=d"(r) : "a"((u32)y), "d"((u32)(y >> 32)), "r"(m) : "cc");
        return;
    }
#endif
    q = (u32)(y / m), r = (u32)(y % m);
}

template <int V>
inline u64 floor_sum(u32 n, u32 m, u32 a, u32 b) {
    if constexpr (V == 0) {
        u64 ans = 0, N = n, M = m, A = a, B = b;
        while (true) {
            if (A >= M) ans += N * (N - 1) / 2 * (A / M), A %= M;
            if (B >= M) ans += N * (B / M), B %= M;
            const u64 y = A * N + B;
            if (y < M) break;
            N = y / M, B = y % M;
            std::swap(M, A);
        }
        return ans;
    } else {
        u64 ans = 0;
        while (true) {
            if (a >= m) ans += (u64)n * (n - 1) / 2 * (a / m), a %= m;
            if (b >= m) ans += (u64)n * (b / m), b %= m;
            const u64 y = (u64)a * n + b;
            if (y < m) break;
            divmod<V>(y, m, n, b);
            std::swap(m, a);
        }
        return ans;
    }
}

}  // namespace fs1
