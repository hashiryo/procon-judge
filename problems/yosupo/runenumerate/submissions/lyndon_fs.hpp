#pragma once
// lyndon_direct で、2 つの Lyndon 配列をスタックでなく NeoLibrary の FastSet (64 分木) で求める版。接尾辞配列を順位の小さい順に 1 回
// 読み、順位が今より小さい位置の集合 S と、大きい位置の集合 C から、i より後ろで最初の位置を next(i + 1) で引く。スタックで求める
// と、降ろす回数が一様な文字列ではでたらめで分岐が外れやすく、手元の M2 の max_random_00 では接尾辞配列と同じ 3.7 ms かかっていた。
// ほかは lyndon_direct と同じ (根は文字を直接比べて伸ばす)。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <string>
#include <utility>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"
#include "neo/data_structure/FastSet.hpp"
#include "_lyndon_runs.hpp"

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    const vector<int> sa = suffix_array(s);
    vector<int> nxt0(n), nxt1(n);
    {
      FastSet smaller(n), larger(n, [](int) { return true; });
      for (int k = 0; k < n; ++k) {
        const int i = sa[k];
        larger.erase(i);
        const int a = smaller.next(i + 1), b = larger.next(i + 1);
        nxt0[i] = a < 0 ? n : a, nxt1[i] = b < 0 ? n : b;
        smaller.insert(i);
      }
    }
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    ans = lyndon_runs::collect(nxt0, nxt1, [&](int i, int j, int p) {
      int e = 0, b = 0;
      while (j + e < n && t[i + e] == t[j + e]) ++e;  // run にならないなら e < p で止まる
      for (const int need = p - e; b < need; ++b)
        if (b == i || t[i - 1 - b] != t[j - 1 - b]) return pair<int, int>(-1, 0);
      while (b < i && t[i - 1 - b] == t[j - 1 - b]) ++b;
      return pair<int, int>(i - b, j + e);
    });
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
