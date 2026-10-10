#pragma once
// 今の Library (mylib) の sums_of_powers_on_primes (Lucy の DP、商の列を double の割り算で引く) で 0 乗の和を取る。
#include "../common.hpp"
#include "mylib/number_theory/sum_on_primes.hpp"

inline u64 run(u64 N) { return sums_of_powers_on_primes<long long>(N, 0)[0].sum(); }
