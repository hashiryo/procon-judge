#pragma once
// 書き比べ: 法 2^64 の集合冪級数の log を、区画と列に分けて変換の領域で閉じた形の AVX2 の核 (_shared/sps/u64_rz2.hpp) で。
#include "pj.hpp"
#include "_shared/sps/u64_rz2.hpp"
struct Solver {
 vector<u64> f, h;
 Solver(int, const vector<u64>& f_): f(f_) {}
 void run() { h= sps_u64b::log(f); }
 vector<u64> answer() const { return h; }
};
