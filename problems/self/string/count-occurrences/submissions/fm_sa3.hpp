#pragma once
// fm_sa に先頭 3 文字の表を足したもの (NeoLibrary の FMIndex の写し元)。文字を使われているものに 1 から番号を振り直し、末尾の
// 短い接尾辞は足りない文字を 0 として、3 文字の組ごとにそれより小さい接尾辞の数 P を持つ (26 種類なら 27^3 個)。3 文字以下の
// パターンは P だけで答え、長さ 4 から 32 は最後の 3 文字の範囲から FM-index で後ろへ絞り、33 以上は最初の 3 文字の範囲の中を
// 二分探索する。表は K^3 <= 4n のときだけ作り、作らないときは fm_sa と同じく、FM-index を最後の 1 文字から回す (二分探索は
// 使わない)。rank の表は fm_sa と同じ、文字ごとの bit の列と 64 個ごとの累積の 16 byte の組。(BWT の文字を 64 byte ずつの塊に
// 置いて AVX2 で数える形の分岐 (if (false) の側) は、x64 で速くならなかった試しの名残。)
#include "pj.hpp"
#include <algorithm>
#include <cstring>
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include "neo/string/suffix_array.hpp"

struct Solver {
  struct alignas(64) Block {
    unsigned char bwt[64];
    u32 cnt[32];
  };
  struct Ent {
    u64 bits;
    u32 cnt, pad;
  };
  int n, sigma = 0, K = 1, words = 0;
  unsigned char code[256];  // 0 は使われていない文字
  int C[258];               // C[x] は番号が x 未満の文字の数に 1 (番兵の行) を足したもの
  vector<int> sa, P;
  string t, pb;
  vector<Block> blk;
  vector<Ent> occ;  // 文字が 32 種類以上のとき
  explicit Solver(const string &s) : n((int)s.size()), sa(suffix_array(s)), t(s) {
    const unsigned char *u = (const unsigned char *)s.data();
    int freq[256] = {};
    for (int i = 0; i < n; ++i) ++freq[u[i]];
    memset(code, 0, sizeof(code));
    for (int c = 0; c < 256; ++c)
      if (freq[c]) code[c] = (unsigned char)++sigma;
    K = sigma + 1;
    C[1] = 1;
    for (int c = 0, x = 1; c < 256; ++c)
      if (freq[c]) C[x + 1] = C[x] + freq[c], ++x;
    // 3 文字の組 (足りない文字は 0) ごとの数を数えて、それより小さい接尾辞の数にする。表がテキストより大きいと作る手間が目立つので、
    // K^3 <= 4 n のときだけ作る。
    if ((size_t)K * K * K <= 4 * (size_t)n) {
      P.assign((size_t)K * K * K + 1, 0);
      for (int i = 0; i < n; ++i) {
        const int a = code[u[i]], b = i + 1 < n ? code[u[i + 1]] : 0, c = i + 2 < n ? code[u[i + 2]] : 0;
        ++P[((size_t)a * K + b) * K + c + 1];
      }
      for (size_t k = 1; k < P.size(); ++k) P[k] += P[k - 1];
    }
    // BWT の行 r (0 <= r <= n) は、行 0 が番兵の接尾辞で、行 r >= 1 が sa[r - 1]。前の文字の番号は 0 始まり、番兵は 255。
    auto bw = [&](int r) -> int {
      const int i = r == 0 ? n : sa[r - 1];
      return i == 0 ? 255 : code[u[i - 1]] - 1;
    };
    if (false) {
      blk.resize((n + 1) / 64 + 1);
      u32 acc[32] = {};
      for (int b = 0; b < (int)blk.size(); ++b) {
        Block &B = blk[b];
        memcpy(B.cnt, acc, sizeof(acc));
        for (int k = 0; k < 64; ++k) {
          const int r = 64 * b + k;
          const int c = r <= n ? bw(r) : 255;
          B.bwt[k] = (unsigned char)c;
          if (c != 255) ++acc[c];
        }
      }
    } else {
      words = (n + 1) / 64 + 1;
      occ.assign((size_t)sigma * words, Ent{0, 0, 0});
      for (int r = 0; r <= n; ++r) {
        const int c = bw(r);
        if (c != 255) occ[(size_t)c * words + (r >> 6)].bits |= (u64)1 << (r & 63);
      }
      for (int c = 0; c < sigma; ++c) {
        Ent *e = occ.data() + (size_t)c * words;
        u32 a = 0;
        for (int w = 0; w < words; ++w) e[w].cnt = a, a += (u32)__builtin_popcountll(e[w].bits);
      }
    }
    t.append(8, '\0');
  }
  // BWT の行 [0, i) の中の番号 c (0 始まり) の数
  int rank(int c, int i) const {
    if (!blk.empty()) {
      const Block &B = blk[i >> 6];
      const __m256i v = _mm256_set1_epi8((char)c);
      const u64 lo = (u32)_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i *)B.bwt), v));
      const u64 hi = (u32)_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i *)(B.bwt + 32)), v));
      return (int)B.cnt[c] + __builtin_popcountll((lo | hi << 32) & (((u64)1 << (i & 63)) - 1));
    }
    const Ent &e = occ[(size_t)c * words + (i >> 6)];
    return (int)e.cnt + __builtin_popcountll(e.bits & (((u64)1 << (i & 63)) - 1));
  }
  // s[i, ...) の先頭 m 文字のうち from 文字目からと、p の from 文字目からを比べる。
  int cmp(int i, int from, int m) const {
    const char *x = t.data() + i, *y = pb.data();
    for (int j = from; j < m; j += 8) {
      u64 a, b;
      memcpy(&a, x + j, 8), memcpy(&b, y + j, 8);
      if (a != b) {
        const int d = j + (__builtin_ctzll(a ^ b) >> 3);
        if (d >= m) return 0;
        return (unsigned char)x[d] < (unsigned char)y[d] ? -1 : 1;
      }
    }
    return 0;
  }
  int count(const string &p) {
    const int m = (int)p.size();
    const unsigned char *q = (const unsigned char *)p.data();
    if (P.empty() || m <= 32) {
      int lo, hi, j;
      if (!P.empty() && m >= 3) {
        const int a = code[q[m - 3]], b = code[q[m - 2]], c = code[q[m - 1]];
        if (!a || !b || !c) return 0;
        const size_t k = ((size_t)a * K + b) * K + c;
        lo = P[k] + 1, hi = P[k + 1] + 1, j = m - 4;
      } else if (!P.empty()) {
        const int a = code[q[0]], b = m > 1 ? code[q[1]] : 0;
        if (!a || (m > 1 && !b)) return 0;
        const size_t k = ((size_t)a * K + b) * K;
        return P[k + (m == 2 ? K : (size_t)K * K)] - P[k];
      } else {
        const int c = code[q[m - 1]];
        if (!c) return 0;
        lo = C[c], hi = C[c + 1], j = m - 2;
      }
      for (; j >= 0 && lo < hi; --j) {
        const int x = code[q[j]];
        if (!x) return 0;
        lo = C[x] + rank(x - 1, lo), hi = C[x] + rank(x - 1, hi);
      }
      return lo < hi ? hi - lo : 0;
    }
    if (m > n) return 0;
    const int a = code[q[0]], b = code[q[1]], c = code[q[2]];
    if (!a || !b || !c) return 0;
    const size_t k = ((size_t)a * K + b) * K + c;
    const int *lo = sa.data() + P[k], *hi = sa.data() + P[k + 1];
    pb.assign(p);
    pb.append(8, '\0');
    const int *x = partition_point(lo, hi, [&](int i) { return cmp(i, 3, m) < 0; });
    const int *y = partition_point(x, hi, [&](int i) { return cmp(i, 3, m) == 0; });
    return int(y - x);
  }
};
