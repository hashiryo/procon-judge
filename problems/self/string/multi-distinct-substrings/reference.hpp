#pragma once
// 参照実装。文字列を区切り文字 (英小文字より大きい '{') でつないだ文字列の接尾辞配列 (_shared/string/_ref_lcp.hpp の SA-IS) と、
// Kasai の方法の LCP 配列で数える。接尾辞配列の順に、区切り文字でない位置 i から始まる接尾辞は、次の区切り文字までの長さ b(i) のうち、
// 1 つ前の接尾辞との LCP を超える分だけ新しい部分文字列を足す (ある長さの接頭辞が前のどれかの接尾辞に現れるなら、間の接尾辞はすべて
// それを共有するので、1 つ前とだけ比べればよい)。
#include "_shared/string/_ref_lcp.hpp"

struct Solver {
  const vector<string> &ss;
  u64 ans = 0;
  explicit Solver(const vector<string> &ss) : ss(ss) {}
  void run() {
    string t;
    for (const auto &s : ss) t += s, t += '{';
    const int n = (int)t.size();
    const vector<int> sa = string_ref::suffix_array(t);
    vector<int> rnk(n), lcp(n, 0), bound(n);
    for (int i = 0; i < n; ++i) rnk[sa[i]] = i;
    for (int i = 0, h = 0; i < n; ++i) {
      if (h > 0) --h;
      if (rnk[i] == 0) {
        h = 0;
        continue;
      }
      const int j = sa[rnk[i] - 1];
      while (i + h < n && j + h < n && t[i + h] == t[j + h]) ++h;
      lcp[rnk[i]] = h;  // lcp[r] は sa[r - 1] と sa[r] の LCP
    }
    for (int i = n, d = 0; i--;) d = t[i] == '{' ? 0 : d + 1, bound[i] = d;
    ans = 0;
    for (int r = 0; r < n; ++r) ans += (u64)max(0, bound[sa[r]] - lcp[r]);
  }
  u64 answer() const { return ans; }
};
