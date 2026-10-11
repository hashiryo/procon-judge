#pragma once
// mod 2 の行列積 C = A B を、B の 8 行ずつの 256 通りの XOR の表 (Four Russians) で求める。
// lib (Library の Matrix<bool>) と同じ考え方だが、lib は表 1 枚ごとに C の行全体を読み書きする。ここでは C の列を 1024 bit の
// 帯に分け、帯ごとに B の 64 行分 (A の 1 語分) の表 8 枚 (256 KB) を作ってから、A の各行の C の帯 (ymm 4 本) に表の行を
// 8 本 XOR する。C の帯の読み書きは、B の 64 行につき 1 回になる。文字列と bit の変換は _shared/linalg/gf2_elim.hpp を使う。
#include "../common.hpp"
#include "_shared/linalg/gf2_elim.hpp"
inline vector<string> run(int n, int m, int k, const vector<string>& a, const vector<string>& b) {
 constexpr int PW= 16;  // 帯の語の数 (1024 bit)
 const int MW= (m + 63) / 64, KW= (k + 1023) / 1024 * PW;
 vector<u64> A((size_t)n * MW), B((size_t)MW * 64 * KW), C((size_t)n * KW);
 for(int i= 0; i < n; ++i) gf2_elim::pack_row(a[i].data(), m, &A[(size_t)i * MW]);
 for(int i= 0; i < m; ++i) gf2_elim::pack_row(b[i].data(), k, &B[(size_t)i * KW]);
 vector<u64> T((size_t)8 * 256 * PW);  // 表 g の行 s は T[(g * 256 + s) * PW]
 for(int p= 0; p < KW; p+= PW) {
  for(int kw= 0; kw < MW; ++kw) {
   for(int g= 0; g < 8; ++g) {
    u64* tg= &T[(size_t)g * 256 * PW];
    fill(tg, tg + PW, 0);
    for(int s= 1; s < 256; ++s) {
     u64* __restrict t= tg + (size_t)s * PW;
     const u64* __restrict prev= tg + (size_t)(s & (s - 1)) * PW;
     const u64* __restrict br= &B[(size_t)(kw * 64 + g * 8 + __builtin_ctz(s)) * KW + p];
     for(int w= 0; w < PW; ++w) t[w]= prev[w] ^ br[w];
    }
   }
   for(int i= 0; i < n; ++i) {
    const u64 x= A[(size_t)i * MW + kw];
    if(!x) continue;
    u64* c= &C[(size_t)i * KW + p];
    __m256i c0= _mm256_loadu_si256((const __m256i*)c), c1= _mm256_loadu_si256((const __m256i*)(c + 4));
    __m256i c2= _mm256_loadu_si256((const __m256i*)(c + 8)), c3= _mm256_loadu_si256((const __m256i*)(c + 12));
    for(int g= 0; g < 8; ++g) {
     const u64* t= &T[((size_t)g * 256 + (x >> (8 * g) & 255)) * PW];
     c0= _mm256_xor_si256(c0, _mm256_loadu_si256((const __m256i*)t)), c1= _mm256_xor_si256(c1, _mm256_loadu_si256((const __m256i*)(t + 4)));
     c2= _mm256_xor_si256(c2, _mm256_loadu_si256((const __m256i*)(t + 8))), c3= _mm256_xor_si256(c3, _mm256_loadu_si256((const __m256i*)(t + 12)));
    }
    _mm256_storeu_si256((__m256i*)c, c0), _mm256_storeu_si256((__m256i*)(c + 4), c1);
    _mm256_storeu_si256((__m256i*)(c + 8), c2), _mm256_storeu_si256((__m256i*)(c + 12), c3);
   }
  }
 }
 vector<string> res(n, string(k, '0'));
 for(int i= 0; i < n; ++i) gf2_elim::unpack_bits(&C[(size_t)i * KW], 0, k, res[i].data());
 return res;
}
