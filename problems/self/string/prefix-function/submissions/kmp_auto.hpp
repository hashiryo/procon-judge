#pragma once
// 使われている文字が 4 種類以下なら、KMP のオートマトンの遷移の表を作りながら接頭辞関数を求める。状態 i の行は、状態 pi[i - 1] の
// 行を写して s[i] の欄だけを i + 1 にしたもので、pi[i] は状態 pi[i - 1] から s[i] で移った先。1 文字に 1 回の表の読み込みで済み、
// 素直な KMP の、border をたどるループの分岐の外れが無くなる。行の幅は種類の数を 2 か 4 に切り上げたもの (8 byte か 16 byte) で、
// 表は長さ 10^6 で 8 MB か 16 MB。行は読んだ値をレジスタで書き換えて 1 回の書き込みで置く (欄ごとに書くと、すぐ後にその行を
// 読むとき、2 つの書き込みにまたがる読み込みになってストアからの転送が効かない)。5 種類以上なら素直な KMP。
#include "pj.hpp"
#include <cstring>
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/sse4.1.h>
#endif

struct Solver {
  const string &s;
  vector<int> pi;
  explicit Solver(const string &s) : s(s) {}
  void auto2(const unsigned char *code) {
    const int n = (int)s.size();
    vector<u64> d(n);
    const unsigned char *t = (const unsigned char *)s.data();
    int *p = pi.data();
    const int *e = (const int *)d.data();
    d[0] = (u64)1 << (32 * code[t[0]]);
    p[0] = 0;
    for (int i = 1; i < n; ++i) {
      const int k = p[i - 1], c = code[t[i]], sh = 32 * c;
      const u64 r = d[k];
      p[i] = e[2 * k + c];
      d[i] = (r & ~((u64)0xffffffffu << sh)) | ((u64)(unsigned)(i + 1) << sh);
    }
  }
  struct alignas(16) Row {
    int v[4];
  };
  void auto4(const unsigned char *code) {
    const int n = (int)s.size();
    vector<Row> d(n);
    const unsigned char *t = (const unsigned char *)s.data();
    int *p = pi.data();
    const int *e = (const int *)d.data();
    __m128i mask[4];
    for (int c = 0; c < 4; ++c) mask[c] = _mm_cmpeq_epi32(_mm_setr_epi32(0, 1, 2, 3), _mm_set1_epi32(c));
    _mm_store_si128((__m128i *)&d[0], _mm_and_si128(mask[code[t[0]]], _mm_set1_epi32(1)));
    p[0] = 0;
    for (int i = 1; i < n; ++i) {
      const int k = p[i - 1], c = code[t[i]];
      const __m128i r = _mm_load_si128((const __m128i *)&d[k]);
      p[i] = e[4 * k + c];
      _mm_store_si128((__m128i *)&d[i], _mm_blendv_epi8(r, _mm_set1_epi32(i + 1), mask[c]));
    }
  }
  void run() {
    const int n = (int)s.size();
    pi.resize(n);
    const unsigned char *t = (const unsigned char *)s.data();
    unsigned char code[256] = {};
    bool seen[256] = {};
    int sigma = 0;
    for (int i = 0; i < n; ++i) seen[t[i]] = true;
    for (int c = 0; c < 256; ++c)
      if (seen[c]) code[c] = (unsigned char)sigma++;
    if (sigma <= 2) return auto2(code);
    if (sigma <= 4) return auto4(code);
    int *p = pi.data();
    p[0] = 0;
    for (int i = 1, k = 0; i < n; ++i) {
      while (k > 0 && t[i] != t[k]) k = p[k - 1];
      k += t[i] == t[k];
      p[i] = k;
    }
  }
  const vector<int> &answer() const { return pi; }
};
