#pragma once
// NeoLibrary の aho_corasick (ac_mix5 を写したもの) を呼ぶ提出。
#include <string>
#include <utility>
#include <vector>
#include "pj.hpp"
#include "neo/string/aho_corasick.hpp"

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() {
    AhoCorasick r = aho_corasick(ss);
    ans.par = std::move(r.par), ans.link = std::move(r.link), ans.word = std::move(r.word);
  }

  const AhoCorasickAnswer &answer() const { return ans; }
};
