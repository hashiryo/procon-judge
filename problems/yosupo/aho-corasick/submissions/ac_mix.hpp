#pragma once
// 文字の種類の数 k で 2 つの作りを選ぶ版。k が 8 以下なら ac_goto2 (深さの順に遷移の行を全部埋める、2 文字なら 1 頂点 8 バイト)、
// それより多ければ ac_hybrid (子が 1 つの間は頂点に直接持ち、枝分かれする頂点にだけ行を割り当てて suffix link をたどる)。手元の M2
// では、5 文字の入力で ac_goto2 が速く (random_19 で 19.92 ms と 31.29 ms)、26 文字の入力で ac_hybrid が速かった (random_11 で
// 48.85 ms と 28.13 ms)。中身は _ac_goto2.hpp と _ac_hybrid.hpp。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <array>
#include <string>
#include <vector>
#include "pj.hpp"
#include "_ac_common.hpp"
#include "_ac_goto2.hpp"
#include "_ac_hybrid.hpp"

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() {
    array<bool, 256> used{};
    for (const auto &s : ss)
      for (const char c : s) used[(unsigned char)c] = true;
    int k = 0;
    for (const bool u : used) k += u;
    if (k <= 8) ac_goto2_body::solve(ss, ans);
    else ac_hybrid_body::solve(ss, ans);
  }

  const AhoCorasickAnswer &answer() const { return ans; }
};
