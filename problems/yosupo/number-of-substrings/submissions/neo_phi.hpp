#pragma once
// NeoLibrary の suffix_array で接尾辞配列を作り、LCP 配列を Φ 配列を経由する方法 (Kärkkäinen、Manzini、Puglisi 2009 の PLCP) で求めて
// 数える。plcp[sa[i]] = sa[i - 1] (接尾辞配列で 1 つ前の接尾辞) を置いてから、文字列の順に plcp[i] を i とその接尾辞の LCP に書き換え、
// 最後に lcp[i] = plcp[sa[i + 1]] と並べ替える。Kasai の方法と同じく比べる文字は合わせて 2n 以下で、飛び飛びに読み書きするのは、
// 1 つ前の接尾辞を置く段の書き込み、比べる相手の文字の読み込み、並べ替えの読み込みの 3 つになる (Kasai の方法は 4 つ)。
// sa[0] には文字列の長さ n を置き、比べる長さの上限 n - max(i, k) を 0 にして比べないようにする (libsais と同じ。その位置では前から
// 引き継いだ長さが必ず 0 なので、0 が書かれる)。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <algorithm>
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"

namespace neo_phi {
// i を文字列の位置として、plcp[i] を i の接尾辞と、接尾辞配列でその 1 つ前の接尾辞の LCP にする (sa[0] では 0)。
inline std::vector<int> plcp_array(const std::string &s, const std::vector<int> &sa) {
  const int n = int(s.size());
  std::vector<int> plcp(n);
  if (n == 0) return plcp;
  plcp[sa[0]] = n;
  for (int i = 1; i < n; ++i) plcp[sa[i]] = sa[i - 1];
  const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
  for (int i = 0, h = 0; i < n; ++i) {
    const int k = plcp[i], m = n - std::max(i, k);
    while (h < m && t[i + h] == t[k + h]) ++h;
    plcp[i] = h;
    h -= h > 0;
  }
  return plcp;
}
// lcp[i] は sa[i] と sa[i + 1] の接尾辞の LCP。
inline std::vector<int> lcp_array(const std::string &s, const std::vector<int> &sa) {
  const int n = int(s.size());
  if (n < 2) return {};
  const std::vector<int> plcp = plcp_array(s, sa);
  std::vector<int> lcp(n - 1);
  for (int i = 0; i + 1 < n; ++i) lcp[i] = plcp[sa[i + 1]];
  return lcp;
}
}  // namespace neo_phi

struct Solver {
  string s;
  long long ans = 0;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const vector<int> sa = suffix_array(s);
    const vector<int> lcp = neo_phi::lcp_array(s, sa);
    const int n = int(s.size());
    ans = (long long)n * (n + 1) / 2;
    for (const int x : lcp) ans -= x;
  }

  long long answer() const { return ans; }
};
