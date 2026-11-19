#pragma once
// NeoLibrary の SubsetIntervalSum (neo/data_structure/SubsetSum.hpp) で解く (unsigned int なので、バケットの中の加算は AVX2 で足す)。
#include "pj.hpp"
#include "neo/data_structure/SubsetSum.hpp"
struct Solver {
 int n;
 vector<u32> a;
 vector<array<u32, 4>> ops;
 vector<u32> ans;
 Solver(int n_, const vector<u32>& a_, const vector<array<u32, 4>>& ops_): n(n_), a(a_), ops(ops_) {}
 void run() {
  SubsetIntervalSum<unsigned> st(a);
  ans.reserve(ops.size());
  for(auto& o: ops)
   if(o[0] == 1) st.add(o[1], o[2], o[3]);
   else ans.push_back(st.sum(o[1], o[2]));
 }
 vector<u32> answer() const { return ans; }
};
