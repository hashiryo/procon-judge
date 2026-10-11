#pragma once
// lyndon_fs_hybrid2 で、2 つの Lyndon 配列を FastSet でなく連結リストで求める版 (_lyndon_hybrid.hpp の lyndon_ends_list)。位置を並べた
// 双方向の連結リストから順位の大きい順に位置を外すと、外すときの次の位置が、順位が小さい最初の位置になる。順位の小さい順に外す
// リストも別に持ち、1 つのループで交互に外す。根は文字を直接比べて伸ばし、比べた文字の数が全体で 32n を超えたら LCE と LCS を
// LCP 配列と RangeMin で引く形に切り替える (最悪でも O(n))。根を伸ばす部分は lyndon_fs_hybrid2 と同じ。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <string>
#include <utility>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"
#include "_lyndon_runs.hpp"
#include "_lyndon_hybrid.hpp"

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    const vector<int> sa = suffix_array(s);
    vector<int> nxt0, nxt1;
    lyndon_hybrid::lyndon_ends_list(sa, nxt0, nxt1);
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    long long budget = 32LL * n;
    lyndon_hybrid::Slow slow{s, sa, nullptr, nullptr};
    ans = lyndon_runs::collect(nxt0, nxt1, [&](int i, int j, int p) __attribute__((always_inline)) {
      if (__builtin_expect(budget < 0, 0)) return slow(i, j, p);
      int e = 0, b = 0;
      while (j + e < n && t[i + e] == t[j + e]) ++e;
      budget -= e + 1;
      for (const int need = p - e; b < need; ++b)
        if (b == i || t[i - 1 - b] != t[j - 1 - b]) return budget -= b + 1, pair<int, int>(-1, 0);
      while (b < i && t[i - 1 - b] == t[j - 1 - b]) ++b;
      budget -= b + 1;
      return pair<int, int>(i - b, j + e);
    });
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
