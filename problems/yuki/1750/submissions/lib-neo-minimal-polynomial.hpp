#pragma once
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/algo/compute/poly/berlekamp_massey.hpp"
#include "neo/algo/compute/poly/linear_recurrence.hpp"
// 求めるのは (A^T)[0][0] = e0^T A^T e0。数列 s_i = e0^T A^i e0 (i < 2N) を隣接リストで 1 ずつ回して作り、NeoLibrary の
// berlekamp_massey で線形漸化式にして、linear_recurrence で第 T 項を出す。
struct Solver {
 int n;
 i64 days;
 vector<array<int, 2>> edges;
 i64 ans= 0;
 Solver(int n, i64 days, const vector<array<int, 2>>& edges): n(n), days(days), edges(edges) {}
 void run() {
  using Z= ZMod<998244353>;
  vector<array<int, 2>> es;
  for(auto& e: edges) es.push_back({e[0], e[1]}), es.push_back({e[1], e[0]});
  sort(es.begin(), es.end());
  es.erase(unique(es.begin(), es.end()), es.end());
  vector<Z> v(n), w(n), s(2 * n);
  v[0]= 1;
  for(int i= 0; i < 2 * n; ++i) {
   s[i]= v[0];
   fill(w.begin(), w.end(), Z(0));
   for(auto& e: es) w[e[0]]+= v[e[1]];
   swap(v, w);
  }
  ans= linear_recurrence(berlekamp_massey(s), s, days).val();
 }
 i64 answer() const { return ans; }
};
