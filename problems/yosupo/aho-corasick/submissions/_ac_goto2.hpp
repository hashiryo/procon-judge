#pragma once
// ac_goto2 と ac_mix が共有する中身 (説明は ac_goto2.hpp)。深さの順に各頂点の遷移の行を埋める。表の幅は文字の種類の数のまま。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <array>
#include <string>
#include <vector>
#include "_ac_common.hpp"

namespace ac_goto2_body {
inline void solve(const std::vector<std::string> &ss, AhoCorasickAnswer &ans) {
  using namespace std;
  array<unsigned char, 256> code;
  const int k = ac_common::remap(ss, code);
  size_t total = 0;
  for (const auto &s : ss) total += s.size();
  const int cap = int(total) + 1;
  ac_common::Zeroed tab(size_t(cap) * k * sizeof(int));
  int *ch = static_cast<int *>(tab.get());
  vector<int> &par = ans.par, &link = ans.link;  // link には、suffix link を書くまで深さを置く
  par.assign(cap, 0), link.assign(cap, 0), ans.word.resize(ss.size());
  vector<unsigned char> last(cap, 0);
  int n = 1, maxd = 0;
  for (size_t i = 0; i < ss.size(); ++i) {
    int v = 0;
    for (const char x : ss[i]) {
      const int c = code[(unsigned char)x];
      int &w = ch[size_t(v) * k + c];
      if (!w) w = n, par[n] = v, last[n] = (unsigned char)c, link[n] = link[v] + 1, maxd = max(maxd, link[n]), ++n;
      v = w;
    }
    ans.word[i] = v;
  }
  vector<int> cnt(maxd + 2, 0), ord(n);
  for (int v = 1; v < n; ++v) ++cnt[link[v]];
  for (int d = 1, acc = 0; d <= maxd + 1; ++d) {
    const int c = cnt[d];
    cnt[d] = acc, acc += c;
  }
  for (int v = 1; v < n; ++v) ord[cnt[link[v]]++] = v;
  const __m256i zero = _mm256_setzero_si256();
  for (int t = 0; t < n - 1; ++t) {
    const int v = ord[t];
    const int l = par[v] == 0 ? 0 : ch[size_t(link[par[v]]) * k + last[v]];
    link[v] = l;
    // 行 v の 0 を、行 l から写す (根の行は、無い子が 0 = 根のまま遷移になっている)
    int *rv = ch + size_t(v) * k;
    const int *rl = ch + size_t(l) * k;
    int c = 0;
    for (; c + 8 <= k; c += 8) {
      const __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(rv + c)), b = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(rl + c));
      _mm256_storeu_si256(reinterpret_cast<__m256i *>(rv + c), _mm256_blendv_epi8(a, b, _mm256_cmpeq_epi32(a, zero)));
    }
    for (; c < k; ++c) {  // 分岐にしないよう、マスクの算術で選ぶ
      const int a = rv[c];
      rv[c] = a | (rl[c] & -int(a == 0));
    }
  }
  par.resize(n), link.resize(n);
}
}  // namespace ac_goto2_body
