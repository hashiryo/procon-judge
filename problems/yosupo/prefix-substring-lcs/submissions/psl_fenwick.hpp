#pragma once
// Library Checker の参照解 (sol/correct.cpp) と同じ考えの基準。s の行を 1 つずつ足しながら、t の各列の下端に出る糸の出どころ h[j]
// (上端の列 x から入った糸なら x、左端から入った糸なら -1) を、Tiskin の seaweed の組み替えで求める。行 i では、左端から -1 の糸を
// 入れて列を左から順に見て、s[i] = t[j] か、h[j] が今の横の糸より小さければ 2 本を入れ替える。a 行のあとの h で、s[0, a) と t[b, c)
// の最長共通部分列の長さは、[b, c) のうち h[j] < b の j の数になる。参照解はクエリを a ごとに分け、各 a で c の順に h[j] を Fenwick
// 木に入れて数える。ここでは参照解の vector の vector を、数え分けた添字の列に変えた。
// 接頭辞と部分文字列の LCS の実装を比べた記録は algo-notes の notes/prefix-substring-lcs.md にある。
#include <array>
#include <numeric>
#include <string>
#include <vector>
#include "pj.hpp"

struct Solver {
  string s, t;
  vector<array<int, 3>> qs;
  vector<int> ans;

  Solver(const string &s, const string &t, const vector<array<int, 3>> &qs) : s(s), t(t), qs(qs) {}

  void run() {
    const int m = int(s.size()), n = int(t.size()), q = int(qs.size());
    ans.assign(q, 0);
    vector<int> sa(m + 2, 0), ord(q);
    for (const auto &x : qs) ++sa[x[0] + 1];
    for (int a = 0; a <= m; ++a) sa[a + 1] += sa[a];
    {
      vector<int> pos(sa.begin(), sa.end() - 1);
      for (int k = 0; k < q; ++k) ord[pos[qs[k][0]]++] = k;
    }
    vector<int> h(n), ft(n + 1), sc(n + 2), byc(q);
    iota(h.begin(), h.end(), 0);
    for (int a = 0; a < m; ++a) {
      int v = -1;
      for (int j = 0; j < n; ++j)
        if (s[a] == t[j] || h[j] < v) swap(h[j], v);
      const int lo = sa[a + 1], hi = sa[a + 2];  // a + 1 行のクエリ
      if (lo == hi) continue;
      fill(sc.begin(), sc.end(), 0);
      for (int k = lo; k < hi; ++k) ++sc[qs[ord[k]][2] + 1];
      for (int c = 0; c <= n; ++c) sc[c + 1] += sc[c];
      for (int k = lo; k < hi; ++k) byc[lo + sc[qs[ord[k]][2]]++] = ord[k];
      fill(ft.begin(), ft.end(), 0);
      int k = lo, count = 0;
      for (int c = 0; c <= n; ++c) {
        for (; k < hi && qs[byc[k]][2] == c; ++k) {
          const int b = qs[byc[k]][1];
          int below = 0;
          for (int i = b; i > 0; i &= i - 1) below += ft[i];
          ans[byc[k]] = (c - b) - (count - below);
        }
        if (c < n && h[c] >= 0) {
          for (int i = h[c] + 1; i <= n; i += i & -i) ++ft[i];
          ++count;
        }
      }
    }
  }

  const vector<int> &answer() const { return ans; }
};
