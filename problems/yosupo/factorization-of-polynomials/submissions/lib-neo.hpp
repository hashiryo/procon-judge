#pragma once
// NeoLibrary の poly_factorize (neo/polynomial/poly_factorize.hpp)。この問題で書き比べた naive を、法の型を引数にしたもの。
#include "neo/algebra/ZMod.hpp"
#include "neo/polynomial/poly_factorize.hpp"
inline vector<pair<int, vector<u32>>> run(u32 p, const vector<u32>& a) {
 using Z= ZMod<0>;
 Z::set_mod(p);
 vector<Z> f(a.size());
 for(size_t i= 0; i < a.size(); ++i) f[i]= Z(a[i]);
 vector<pair<int, vector<u32>>> res;
 for(auto& [g, e]: poly_factorize(f)) {
  vector<u32> b(g.size());
  for(size_t i= 0; i < g.size(); ++i) b[i]= g[i].val();
  res.push_back({e, b});
 }
 return res;
}
