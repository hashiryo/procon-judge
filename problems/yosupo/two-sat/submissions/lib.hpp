#pragma once
// Library の TwoSatisfiability (mylib/misc/TwoSatisfiability.hpp)。変数 x の否定を x + n とし、節 a ∨ b を辺 ¬a → b と ¬b → a で
// 含意グラフ (Graph) に積み、StronglyConnectedComponents (Kosaraju) で分ける。raw のときの lib.cpp と同じく、変数を 1 から n で
// 持つため n + 1 個の変数で作る。
#include "pj.hpp"
#include "mylib/misc/TwoSatisfiability.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& cs;
 vector<bool> ans;
 Solver(int n, const vector<array<int, 2>>& cs): n(n), cs(cs) {}
 void run() {
  TwoSatisfiability sat(n + 1);
  for(auto& c: cs) {
   int a= c[0], b= c[1];
   if(a < 0) a= sat.neg(-a);
   if(b < 0) b= sat.neg(-b);
   sat.add_or(a, b);
  }
  ans= sat.solve();
 }
 bool satisfiable() const { return !ans.empty(); }
 bool value(int i) const { return ans[i]; }
};
