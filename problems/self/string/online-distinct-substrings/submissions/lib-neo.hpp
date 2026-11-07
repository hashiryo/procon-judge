#pragma once
// NeoLibrary の SuffixAutomaton (neo/string/SuffixAutomaton.hpp) を呼ぶ提出。中身は、この問題で書き比べた sam_art の列を huge page の
// vector にしたもの。文字列の基本の道具を比べた記録は algo-notes の notes/string-basics.md にある。
#include "pj.hpp"
#include "neo/string/SuffixAutomaton.hpp"

struct Solver {
  SuffixAutomaton<> sam;
  u64 push(char c) {
    sam.push(c);
    return (u64)sam.distinct_substrings();
  }
};
