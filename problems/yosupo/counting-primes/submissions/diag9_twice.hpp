#pragma once
// 診断用。dr9 を 2 回呼ぶ (2 回目は確保したメモリや表が温まった状態)。dr9 との差が 2 回目の時間で、1 回目との差が 1 回目だけにかかる手間。
#include "../common.hpp"
#include "_dr9.hpp"

inline u64 run(u64 N) {
    volatile u64 first = dr9::prime_pi<0>(N);  // 1 回目の値は捨てる (volatile で消されないようにする)
    (void)first;
    return dr9::prime_pi<0>(N);
}
