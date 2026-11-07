#pragma once
// NeoLibrary の Trie (neo/string/Trie.hpp) を呼ぶ提出。中身は、この問題で書き比べた trie_art を写したもの。
// 文字列の基本の道具を比べた記録は algo-notes の notes/string-basics.md にある。
#include "pj.hpp"
#include "neo/string/Trie.hpp"

struct Solver {
  Trie<> tr;
  void insert(const string &s) { tr.insert(s); }
  void erase(const string &s) { tr.erase(s); }
  int count_prefix(const string &p) { return tr.count_prefix(p); }
};
