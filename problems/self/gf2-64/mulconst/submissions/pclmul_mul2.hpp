#pragma once
// 定数倍を共有の mul2 (VPCLMULQDQ) で入力 2 つずつ計算する版。
#pragma GCC optimize("O3,unroll-loops")
#include "common.hpp"
#include "_shared/gf2-64/mul.hpp"
#include "_shared/gf2-64/mul2.hpp"
inline vector<u64> run(const vector<u64>& as) {
 using gf2_64_pclmul::mul;
 using gf2_64_pclmul::mul2;
 using gf2_64_pclmul::unpack;
 const size_t n= as.size();
 vector<u64> ans(n);
 const __m256i cv= _mm256_set1_epi64x(MUL_CONST);
 size_t i= 0;
 for(; i + 1 < n; i+= 2) tie(ans[i], ans[i + 1])= unpack(mul2(cv, _mm256_set_epi64x(0, as[i + 1], 0, as[i])));
 if(i < n) ans[i]= mul(MUL_CONST, as[i]);
 return ans;
}
