#pragma once
// LCP 配列を Kasai の方法 (Kasai ほか 2001、ACL の lcp_array と同じ手順) で求める。順位の配列 rnk[sa[i]] = i を作ってから文字列の順に
// 進み、位置 i と、接尾辞配列で i の 1 つ前の接尾辞 sa[rnk[i] - 1] を比べる (LCP は 1 つ進むごとに高々 1 しか減らないので、比べる
// 文字は合わせて 2n 以下)。飛び飛びに読み書きするのは、順位を作る段の書き込み、1 つ前の接尾辞と、その文字の読み込み、LCP の書き込みの
// 4 つ。ACL は文字列を int の列に写してから比べるが、ここでは byte のまま比べる。yosupo-number-of-substrings と
// yosupo-longest-common-substring の提出が使う。LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <string>
#include <vector>

namespace lcp_kasai {
// lcp[i] は sa[i] と sa[i + 1] の接尾辞の LCP。
inline std::vector<int> lcp_array(const std::string &s, const std::vector<int> &sa) {
  const int n = int(s.size());
  if (n < 2) return {};
  std::vector<int> rnk(n), lcp(n - 1);
  for (int i = 0; i < n; ++i) rnk[sa[i]] = i;
  for (int i = 0, h = 0; i < n; ++i) {
    if (h > 0) --h;
    if (rnk[i] == 0) continue;
    const int j = sa[rnk[i] - 1];
    while (j + h < n && i + h < n && s[j + h] == s[i + h]) ++h;
    lcp[rnk[i] - 1] = h;
  }
  return lcp;
}
}  // namespace lcp_kasai
