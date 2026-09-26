#pragma once
// Library の convolve (u64 は複素 FFT で分割する) をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/fft/convolve.hpp"
inline vector<u64> run(const vector<u64>& a, const vector<u64>& b) {
 auto c= convolve(a, b);
 c.resize(a.size() + b.size() - 1);
 return vector<u64>(c.begin(), c.end());
}
