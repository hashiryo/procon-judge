#pragma once
// Stern-Brocot 木の 17 回目。DECODE の (lx, ly) と (rx, ry) を xmm の 64 bit の 2 つの枠に置き、vpmuludq (下位 32 bit どうしの
// 積を 64 bit で返す) で掛ける。1 回目の形をコンパイラがベクトル化した vpmulld は、AMD (Zen 4) では 3 サイクルで速いが
// Intel では 10 サイクル待つ。7 回目のスカラーの imul は Intel では速いが、AMD では掛け算器が 1 つなので遅れた。
// vpmuludq は Intel で 5 サイクル、Zen 4 で 3 サイクル。値は答え (10^9 以下) を超えないので、枠の下位 32 bit がそのまま値になる。
// x86 でなければ 1 回目の形に任せる。
#include "_sb1.hpp"
#if defined(__x86_64__)
#include <immintrin.h>
#endif

namespace sb17 {

// V = 1 なら 7 回目の V = 1 と同じく R と L の 2 連をまとめ (L' = L + n R、R' = (n m + 1) R + m L)、鎖を 2 連で掛け算と
// 足し算 1 回ずつにする。n m + 1 は 2^32 を超えうるが、vpmuludq は下位 32 bit しか見ないので、u32 の桁あふれのまま渡してよい。
template <int V = 0>
inline void decode(const u32* p, u32 len, u32* out) {
#if defined(__x86_64__)
    __m128i L = _mm_set_epi64x(1, 0), R = _mm_set_epi64x(0, 1);  // (lx, ly) = (0, 1)、(rx, ry) = (1, 0)
    u32 i = 0;
    for (; i + 1 < len; i += 2) {
        const __m128i n = _mm_set1_epi64x(p[i]), m = _mm_set1_epi64x(p[i + 1]);
        if constexpr (V == 0) {
            L = _mm_add_epi64(L, _mm_mul_epu32(n, R));
            R = _mm_add_epi64(R, _mm_mul_epu32(m, L));
        } else {
            const __m128i c = _mm_set1_epi64x(p[i] * p[i + 1] + 1);
            const __m128i nL = _mm_add_epi64(L, _mm_mul_epu32(n, R));
            R = _mm_add_epi64(_mm_mul_epu32(c, R), _mm_mul_epu32(m, L));
            L = nL;
        }
    }
    if (i < len) L = _mm_add_epi64(L, _mm_mul_epu32(_mm_set1_epi64x(p[i]), R));
    const __m128i s = _mm_add_epi64(L, R);
    out[0] = (u32)_mm_cvtsi128_si64(s), out[1] = (u32)_mm_extract_epi64(s, 1);
#else
    sb1::decode(p, len, out);
#endif
}

}  // namespace sb17
