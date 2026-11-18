#pragma once
// 書き比べ: 法 2^64 の集合冪級数の exp を、rz2 に手直しを足した核 (_shared/sps/u64_rz3.hpp) で。旗は先読みだけ。
#define RZ3_PF 1
#define RZ3_VW 0
#define RZ3_R4 0
#define RZ3_ML 0
#include "pj.hpp"
#include "_shared/sps/u64_rz3.hpp"
struct Solver {
 vector<u64> f, h;
 Solver(int, const vector<u64>& f_): f(f_) {}
 void run() { h= sps_u64c::exp(f); }
 vector<u64> answer() const { return h; }
};
