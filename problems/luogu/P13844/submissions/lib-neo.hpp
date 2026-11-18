#pragma once
// NeoLibrary の sps::log (neo/set_power_series/log.hpp) を u64 (法 2^64) で呼ぶ。
#include "pj.hpp"
#include "neo/set_power_series/log.hpp"
struct Solver {
 vector<u64> f, h;
 Solver(int, const vector<u64>& f_): f(f_) {}
 void run() { h= sps::log(f); }
 vector<u64> answer() const { return h; }
};
