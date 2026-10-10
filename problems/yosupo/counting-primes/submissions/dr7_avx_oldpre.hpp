#pragma once
// 比べる用。dr7_avx の 17 と 19 を、型の AND でなく今までどおり 1 つずつ消し、π の表を 0 で埋めてから作る。
#define DR7_OLDPRE
#include "_dr7.hpp"

inline u64 run(u64 N) { return dr7::prime_pi<1>(N); }
