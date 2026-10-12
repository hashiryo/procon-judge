#pragma once
// NeoLibrary の eertree (eer_lean4 を写したもの) を呼ぶ提出。NeoLibrary の頂点の番号 (0 が長さ -1 の根、1 が長さ 0 の根) から、問題の
// 番号 (-1 と 0 が根) へは 1 を引き、列を 1 つ前へ寄せて合わせる。
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
    // 問題の v 番 (1 以上) は NeoLibrary の v + 1 番。1 回の走査で 1 つ前へ寄せながら 1 を引く (0 番は使わない)
    const int m = int(e.par.size()) - 1;
    for (int v = 0; v < m; ++v) e.par[v] = e.par[v + 1] - 1, e.link[v] = e.link[v + 1] - 1;
    e.par.resize(m), e.link.resize(m);
    for (int &x : e.last) --x;
    ans.par = std::move(e.par), ans.link = std::move(e.link), ans.last = std::move(e.last);
  }

  const EertreeAnswer &answer() const { return ans; }
};
