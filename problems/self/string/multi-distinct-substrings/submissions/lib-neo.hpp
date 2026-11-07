#pragma once
// NeoLibrary の SuffixAutomaton (neo/string/SuffixAutomaton.hpp) の広義の形を呼ぶ提出。文字列ごとに根から足し、異なる部分文字列の数を
// 足しながら数える。中身は gsam_art と同じ作り方。文字列の基本の道具を比べた記録は algo-notes の notes/string-basics.md にある。
#include "pj.hpp"
#include "neo/string/SuffixAutomaton.hpp"

struct Solver {
  const vector<string> &ss;
  u64 ans = 0;
  explicit Solver(const vector<string> &ss) : ss(ss) {}
  void run() { ans = (u64)SuffixAutomaton<>(ss).distinct_substrings(); }
  u64 answer() const { return ans; }
};
