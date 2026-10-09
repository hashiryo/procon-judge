#pragma once
#include "neo/fft/convolve_GF2p64.hpp"
#include <vector>
using namespace std;
using u64= unsigned long long;
// ハーネスは u64 で受け渡すので、公開の convolve (GF2p64) が呼ぶ中の口を u64 のまま呼び、写しを計測区間に入れない。
inline vector<u64> run(int, int, const vector<u64>& a, const vector<u64>& b) { return gf2p64_internal::cantor::convolve(a, b); }
