#pragma once
// 書き比べ: 法 2^64 の集合冪級数の log を、段ごとのランク付きのゼータ変換を AVX2 の u64 × 4 で行う核 (_shared/sps/u64_rz.hpp) で。
#include "pj.hpp"
#include "_shared/sps/u64_rz.hpp"
struct Solver {
 vector<u64> f, h;
 Solver(int, const vector<u64>& f_): f(f_) {}
 void run() { h= sps_u64::log(f); }
 vector<u64> answer() const { return h; }
};
