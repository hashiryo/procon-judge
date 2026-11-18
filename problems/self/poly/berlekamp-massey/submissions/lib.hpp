#pragma once
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/berlekamp_massey.hpp"
inline vector<u32> run(const vector<u32>& a) {
 using Mint= ModInt<998244353>;
 vector<Mint> b(a.begin(), a.end());
 vector<Mint> c= berlekamp_massey(b);
 vector<u32> r(c.size());
 for(size_t i= 0; i < c.size(); ++i) r[i]= c[i].val();
 return r;
}
