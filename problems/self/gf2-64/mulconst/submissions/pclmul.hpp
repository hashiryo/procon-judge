#pragma once
// 定数倍を共有の mul (pclmul + reduce) で素直に計算する版。
#pragma GCC optimize("O3,unroll-loops")
#include "common.hpp"
#include "_shared/gf2-64/mul.hpp"
inline vector<u64> run(const vector<u64>& as) {
 using gf2_64_pclmul::mul;
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= mul(MUL_CONST, as[i]);
 return ans;
}
