#pragma once
// neo_phi で、LCP 配列へ並べ替えずに、文字列の順の plcp の和をそのまま引く版。隣り合う接尾辞の LCP の和は、どの位置の plcp も 1 回ずつ
// 足したものと同じなので、最後の並べ替え (飛び飛びの読み込み n 回と、LCP 配列の確保) が要らない。この問題に特有の近道で、LCP 配列を
// 返す関数の形 (neo_phi) と比べる。ほかは neo_phi と同じ。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <algorithm>
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"

namespace neo_phi_sum {
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
}  // namespace neo_phi_sum

struct Solver {
  string s;
  long long ans = 0;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const vector<int> sa = suffix_array(s);
    const vector<int> plcp = neo_phi_sum::plcp_array(s, sa);
    const int n = int(s.size());
    ans = (long long)n * (n + 1) / 2;
    for (const int x : plcp) ans -= x;
  }

  long long answer() const { return ans; }
};
