#pragma once
#include "../common.hpp"
// SIMD の動作確認用。Union-Find 自体は SIMD の恩恵が薄いが、
// ifdef の切り替えとコンパイルが通ることの確認として使う。
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

struct Solver {
    vector<int> par;
    Solver(int n) : par(n, -1) {
        // SIMD コンパイル確認: AVX2 の命令が使えるか
        __m256i a = _mm256_set1_epi32(42);
        __m256i b = _mm256_set1_epi32(1);
        __m256i c = _mm256_add_epi32(a, b);
        (void)c;
    }
    int find(int x) { return par[x] < 0 ? x : par[x] = find(par[x]); }
    bool unite(int x, int y) {
        x = find(x); y = find(y);
        if (x == y) return false;
        if (par[x] > par[y]) swap(x, y);
        par[x] += par[y];
        par[y] = x;
        return true;
    }
    bool same(int x, int y) { return find(x) == find(y); }
};

#ifdef PJ_CLANG_TARGET_PUSHED
#undef PJ_CLANG_TARGET_PUSHED
#pragma clang attribute pop
#endif
