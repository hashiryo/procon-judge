#pragma once
// 文字ごとの行列。状態を行ベクトル (0 で終わる部分列の数、1 で終わる部分列の数、1) とし、v' = v M で進める。
//   '0': dp0' = dp0 + dp1 + 1、dp1' = dp1
//   '1': dp0' = dp0、dp1' = dp0 + dp1 + 1
//   '?': dp0' = dp1' = dp0 + dp1 + 1
// 全体の積 T について、答えは (0, 0, 1) T の第 0 成分と第 1 成分の和、つまり T(2, 0) + T(2, 1)。
#include "../common.hpp"
inline const array<u32, 9>& char_mat(char c) {
 static const array<u32, 9> m0= {1, 0, 0, 1, 1, 0, 1, 0, 1}, m1= {1, 1, 0, 0, 1, 0, 0, 1, 1}, mq= {1, 1, 0, 1, 1, 0, 1, 1, 1};
 return c == '0' ? m0 : c == '1' ? m1 : mq;
}
