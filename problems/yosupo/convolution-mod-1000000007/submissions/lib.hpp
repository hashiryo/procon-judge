#pragma once
// Library の convolve (mod 10^9+7 は NTT 素数でないので中で分割する) をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/convolve.hpp"
inline vector<u32> run(const vector<u32>& a, const vector<u32>& b) {
 using Mint= ModInt<int(1e9 + 7)>;
 vector<Mint> x(a.begin(), a.end()), y(b.begin(), b.end());
 auto c= convolve(x, y);
 size_t n= a.size() + b.size() - 1;
 c.resize(n);
 vector<u32> ret(n);
 for(size_t k= 0; k < n; ++k) ret[k]= c[k].val();
 return ret;
}
