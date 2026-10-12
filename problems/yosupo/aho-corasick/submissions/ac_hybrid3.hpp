#pragma once
// 3 回目に置いた ac_hybrid (7a0760d5、今は消した) で、ac_goto3 と同じく確保して 0 で埋める配列を減らし、残りに huge page を頼んだ版。幅優先の順は、深さで数え分ける
// 代わりに、取り出した頂点の子 (行があれば行の欄、無ければ 1 つ目の子) を待ち行列に積んで作る。子の表の作りと suffix link のたどり方は
// ac_hybrid と同じ。中身は _ac_hybrid3.hpp。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "_ac_hybrid3.hpp"

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() { ac_hybrid3_body::solve(ss, ac_common::remap_total(ss), ans); }

  const AhoCorasickAnswer &answer() const { return ans; }
};
