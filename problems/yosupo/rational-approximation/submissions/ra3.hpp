#pragma once
// 3 回目: 2 回目 (_ra2.hpp) で、連を全部進めた先の確かめを P を進めてから P + E で行う (_ra2.hpp の V = 1)
#include "_ra2.hpp"
inline void run(const std::vector<Query>& qs, u32* out) { ra2::run<1>(qs, out); }
