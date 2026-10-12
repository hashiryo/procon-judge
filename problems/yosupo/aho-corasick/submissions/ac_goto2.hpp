#pragma once
// ac_goto で、確保する領域を減らした版。(1) 遷移の表の幅を文字の種類の数 k のままにし (ac_goto は AVX2 で写すために 8 の倍数へ
// 切り上げていて、2 文字の入力でも 1 頂点 32 バイトだった)、行の写しは 8 個ずつ AVX2 の blend で、端は 1 個ずつマスクの算術で行う。(2) 深さは、
// まだ求めていない suffix link の場所に置く (深さの順に回すので、suffix link を書く頂点より浅い頂点は書き終えている)。1 回目
// (0ae191d8 の run 38208579609、EPYC 9V45) では ac_goto が 4 環境のすべてで首位だった (x64-gcc で 14.06 ms)。
// 中身は _ac_goto2.hpp。Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "_ac_goto2.hpp"

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() { ac_goto2_body::solve(ss, ans); }

  const AhoCorasickAnswer &answer() const { return ans; }
};
