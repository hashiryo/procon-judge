#pragma once
// 3 回目に置いた ac_mix (7a0760d5、今は消した) と同じく文字の種類の数 k で 2 つの作りを選び、中身を ac_goto3 (k が 8 以下) と ac_hybrid3 (k が 9 以上) にした版。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "_ac_goto3.hpp"
#include "_ac_hybrid3.hpp"

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() {
    const ac_common::Coded cd = ac_common::remap_total(ss);
    if (cd.k <= 8) ac_goto3_body::solve(ss, cd, ans);
    else ac_hybrid3_body::solve(ss, cd, ans);
  }

  const AhoCorasickAnswer &answer() const { return ans; }
};
