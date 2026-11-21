#pragma once
// NeoLibrary の sps::log (neo/base/core/set_fps/log.hpp) を u64 (法 2^64) で呼ぶ。
#include "pj.hpp"
#include "neo/base/core/set_fps/log.hpp"
struct Solver {
 vector<u64> f, h;
 Solver(int, const vector<u64>& f_): f(f_) {}
 void run() { h= sps::log(f); }
 vector<u64> answer() const { return h; }
};
