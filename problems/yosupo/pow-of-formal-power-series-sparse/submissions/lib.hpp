#pragma once
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/sparse_fps.hpp"
inline vector<u32> run(int n, u64 m, const vector<pair<int, u32>> &f) {
 using Mint= ModInt<998244353>;
 vector<Mint> a(n);
 for(auto [i, v]: f) a[i]= v;
 vector<Mint> g= sfps::pow(a, (long long)m, n - 1);
 vector<u32> r(g.size());
 for(size_t i= 0; i < g.size(); ++i) r[i]= g[i].val();
 return r;
}
