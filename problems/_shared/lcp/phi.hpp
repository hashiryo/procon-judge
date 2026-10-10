#pragma once
// LCP を Φ 配列を経由する方法 (Kärkkäinen、Manzini、Puglisi 2009 の PLCP) で求める。buf[sa[i]] = sa[i - 1] (接尾辞配列で 1 つ前の
// 接尾辞) を置いてから、文字列の順に buf[i] を i とその接尾辞の LCP (PLCP) に書き換える。Kasai の方法と同じく比べる文字は合わせて
// 2n 以下で、飛び飛びに読み書きするのは、1 つ前の接尾辞を置く段の書き込みと、比べる相手の文字の読み込みの 2 つ (LCP 配列まで
// 並べ替えるなら、その読み込みでもう 1 つ)。sa[0] には文字列の長さ n を置き、比べる長さの上限 n - max(i, k) を 0 にして比べないように
// する (libsais と同じ。その位置では前から引き継いだ長さが必ず 0 になる)。yosupo-number-of-substrings と
// yosupo-longest-common-substring の提出が使う。LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <algorithm>
#include <string>
#include <vector>

namespace lcp_phi {
// 文字列の順に、位置 i と、接尾辞配列でその 1 つ前の接尾辞の位置 k (sa[0] では k = n) と、その LCP h を f(i, k, h) に渡す。buf は長さ
// n の作業用の領域で、終わると PLCP (buf[i] = h) が残る。
template <class F> inline void scan_plcp(const std::string &s, const std::vector<int> &sa, int *buf, F &&f) {
  const int n = int(s.size());
  if (n == 0) return;
  buf[sa[0]] = n;
  for (int i = 1; i < n; ++i) buf[sa[i]] = sa[i - 1];
  const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
  for (int i = 0, h = 0; i < n; ++i) {
    const int k = buf[i], m = n - std::max(i, k);
    while (h < m && t[i + h] == t[k + h]) ++h;
    buf[i] = h;
    f(i, k, h);
    h -= h > 0;
  }
}
// plcp[i] は、位置 i の接尾辞と、接尾辞配列でその 1 つ前の接尾辞の LCP (sa[0] では 0)。
inline std::vector<int> plcp_array(const std::string &s, const std::vector<int> &sa) {
  std::vector<int> plcp(s.size());
  scan_plcp(s, sa, plcp.data(), [](int, int, int) {});
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
}  // namespace lcp_phi
