#pragma once
// NeoLibrary の sps::exp (neo/base/core/set_fps/exp.hpp) を u64 (法 2^64) で呼ぶ。
#include "pj.hpp"
#include "neo/base/core/set_fps/exp.hpp"
struct Solver {
 vector<u64> f, h;
 Solver(int, const vector<u64>& f_): f(f_) {}
 void run() { h= sps::exp(f); }
 vector<u64> answer() const { return h; }
};
