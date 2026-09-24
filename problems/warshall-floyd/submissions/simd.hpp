#pragma once
#include "../common.hpp"
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
// -march に頼らず、使う AVX2 をここで宣言する (x86 のときだけ)。
#if (defined(__x86_64__) || defined(__i386__)) && !defined(USE_SIMDE)
#if defined(__clang__)
// clang は #pragma GCC target を無視するので、-march に頼らず同じ一覧を attribute push で宣言する。
// push はこのファイルの最後で pop する。
#pragma clang attribute push(__attribute__((target("avx2"))), apply_to = function)
#define PJ_CLANG_TARGET_PUSHED 1
#else
#pragma GCC target("avx2")
#endif
#endif
// AVX2 で内側 j ループをベクトル化した Warshall-Floyd。
// k, i 固定で d[i][j] = min(d[i][j], d[i][k] + d[k][j]) の j 軸を 8-wide で処理。
// Vp は 8 の倍数を仮定 (パディング列は INF で埋まっている)。
struct WF {
 int V, Vp;
 i32* d;
 WF(int v, int vp, i32* dist): V(v), Vp(vp), d(dist) {}
 void run() {
  for(int k= 0; k < V; ++k) {
   const i32* dk= d + k * Vp;
   for(int i= 0; i < V; ++i) {
    i32* di= d + i * Vp;
    __m256i vdik= _mm256_set1_epi32(di[k]);
    for(int j= 0; j < Vp; j+= 8) {
     __m256i vdkj= _mm256_loadu_si256((const __m256i*)(dk + j));
     __m256i vdij= _mm256_loadu_si256((const __m256i*)(di + j));
     __m256i vsum= _mm256_add_epi32(vdik, vdkj);
     __m256i vmin= _mm256_min_epi32(vdij, vsum);
     _mm256_storeu_si256((__m256i*)(di + j), vmin);
    }
   }
  }
 }
};

#ifdef PJ_CLANG_TARGET_PUSHED
#undef PJ_CLANG_TARGET_PUSHED
#pragma clang attribute pop
#endif
