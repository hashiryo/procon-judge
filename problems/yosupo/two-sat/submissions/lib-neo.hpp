#pragma once
// NeoLibrary の two_sat (neo/graph/two_sat.hpp)。この問題で書き比べた occ_gabow_trim_hp を写したもので、作業領域が 2 MB 未満なら
// huge page を頼まない分と、リテラルを NeoLibrary の書き方 (変数 i の否定が ~i) に直した節の列を run() の中で作る分が違う。
// 入力の否定 -k は ~(k - 1) と同じ値なので、直すのは肯定のリテラルだけ。
#include "pj.hpp"
#include "neo/graph/two_sat.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& cs;
 vector<int> val;
 Solver(int n, const vector<array<int, 2>>& cs): n(n), cs(cs) {}
 void run() {
  vector<array<int, 2>> c(cs.size());
  for(size_t j= 0; j < cs.size(); ++j) c[j]= {cs[j][0] > 0 ? cs[j][0] - 1 : cs[j][0], cs[j][1] > 0 ? cs[j][1] - 1 : cs[j][1]};
  val= two_sat(n, c);
 }
 bool satisfiable() const { return !val.empty(); }
 bool value(int i) const { return val[i - 1]; }
};
