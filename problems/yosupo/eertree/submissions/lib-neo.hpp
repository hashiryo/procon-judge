#pragma once
// NeoLibrary の eertree (eer_lean4 を写したもの) を呼ぶ提出。NeoLibrary の頂点の番号 (0 が長さ -1 の根、1 が長さ 0 の根) から、問題の
// 番号 (-1 と 0 が根) へは 1 を引き、列の先頭の 1 個を除いて合わせる。
#include <string>
#include <utility>
#include <vector>
#include "pj.hpp"
#include "neo/string/eertree.hpp"

struct Solver {
  string s;
  EertreeAnswer ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    Eertree e = eertree(s);
    e.par.erase(e.par.begin()), e.link.erase(e.link.begin());
    for (int &x : e.par) --x;
    for (int &x : e.link) --x;
    for (int &x : e.last) --x;
    ans.par = std::move(e.par), ans.link = std::move(e.link), ans.last = std::move(e.last);
  }

  const EertreeAnswer &answer() const { return ans; }
};
