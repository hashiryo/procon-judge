#pragma once
// いつも大きいほうを小さいほうで 1 回割ってから Stein に入る。割り算の重さを見るための対照。
#include "_stein.hpp"
inline u64 run(u64 a, u64 b) { return gcd_div1<-1>(a, b); }
