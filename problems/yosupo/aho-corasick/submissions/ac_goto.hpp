#pragma once
// ac_remap で、suffix link をたどる代わりに、深さの順に各頂点の遷移の行を埋める版 (Library Checker の参照解と同じ考え)。頂点 v の行の
// 無い子 (0) は、v の suffix link の行から写す。深さの順に埋めるので、写し元の行 (v より浅い) は埋まっている。頂点 v = (親 p の文字
// c の子) の suffix link は、p の suffix link の行の c (深さ 1 なら根)。1 頂点あたり k 個の写しは AVX2 の blend で 8 個ずつ (arm では
// SIMDe)。表には実の子と遷移が混ざるが、頂点を深さの順に回すので実の子を見分けなくてよい。中身の確保は _ac_common.hpp。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <array>
#include <string>
#include <vector>
#include "pj.hpp"
#include "_ac_common.hpp"

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() {
    array<unsigned char, 256> code;
    const int k = ac_common::remap(ss, code), kw = (k + 7) & ~7;  // 行は 8 個の倍数にそろえる
    size_t total = 0;
    for (const auto &s : ss) total += s.size();
    const int cap = int(total) + 1;
    ac_common::Zeroed tab(size_t(cap) * kw * sizeof(int));
    int *ch = static_cast<int *>(tab.get());
    vector<int> &par = ans.par, &link = ans.link;
    par.assign(cap, 0), link.assign(cap, 0), ans.word.resize(ss.size());
    vector<unsigned char> last(cap, 0);
    vector<int> depth(cap, 0);
    int n = 1, maxd = 0;
    for (size_t i = 0; i < ss.size(); ++i) {
      int v = 0;
      for (const char x : ss[i]) {
        const int c = code[(unsigned char)x];
        int &w = ch[size_t(v) * kw + c];
        if (!w) w = n, par[n] = v, last[n] = (unsigned char)c, depth[n] = depth[v] + 1, maxd = max(maxd, depth[n]), ++n;
        v = w;
      }
      ans.word[i] = v;
    }
    vector<int> cnt(maxd + 2, 0), ord(n);
    for (int v = 1; v < n; ++v) ++cnt[depth[v]];
    for (int d = 1, acc = 0; d <= maxd + 1; ++d) {
      const int c = cnt[d];
      cnt[d] = acc, acc += c;
    }
    for (int v = 1; v < n; ++v) ord[cnt[depth[v]]++] = v;
    const __m256i zero = _mm256_setzero_si256();
    for (int t = 0; t < n - 1; ++t) {
      const int v = ord[t];
      const int l = depth[v] == 1 ? 0 : ch[size_t(link[par[v]]) * kw + last[v]];
      link[v] = l;
      // 行 v の 0 を、行 l から写す (根の行は、無い子が 0 = 根のまま遷移になっている)
      int *rv = ch + size_t(v) * kw;
      const int *rl = ch + size_t(l) * kw;
      for (int c = 0; c < kw; c += 8) {
        const __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(rv + c)), b = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(rl + c));
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(rv + c), _mm256_blendv_epi8(a, b, _mm256_cmpeq_epi32(a, zero)));
      }
    }
    par.resize(n), link.resize(n);
  }

  const AhoCorasickAnswer &answer() const { return ans; }
};
