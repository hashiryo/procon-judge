#pragma once
// NeoLibrary の SubsetSum (neo/data_structure/SubsetSum.hpp) で解く。値 v の補集合 ~v を数え、m を含む要素の数を ~m の部分集合の数として
// 問う。問いが加算の k 倍あるので、下の bit を 2k / 3 個にする (書き比べの ds2 と同じ分け方)。
#include "pj.hpp"
#include "neo/data_structure/SubsetSum.hpp"
struct Solver {
 int k;
 vector<int> a;
 vector<pair<int, int>> ops;
 vector<int> ans;
 Solver(int k_, const vector<int>& a_, const vector<pair<int, int>>& ops_): k(k_), a(a_), ops(ops_) {}
 void run() {
  const u32 full= (u32(1) << k) - 1;
  SubsetSum<int> st(k, 2 * k / 3);
  for(int v: a) st.add(full ^ u32(v), 1);
  ans.reserve(ops.size());
  for(auto [t, v]: ops) {
   if(t == 1) st.add(full ^ u32(v), 1);
   else if(t == 2) st.add(full ^ u32(v), -1);
   else {
    u32 m= 0;
    for(int b= k - 1; b >= 0; --b)
     if(st.sum(full ^ (m | (u32(1) << b))) >= v) m|= u32(1) << b;
    ans.push_back(int(m));
   }
  }
 }
 vector<int> answer() const { return ans; }
};
