#pragma once
// NeoLibrary の linear_recurrence (neo/polynomial/linear_recurrence.hpp)。この問題で書き比べた bmo を、法を定数にしない形で
// 写したもの。
#include "neo/algebra/ZMod.hpp"
#include "neo/polynomial/linear_recurrence.hpp"
inline u32 run(const vector<u32>& a, const vector<u32>& c, u64 k) {
 using Z= ZMod<998244353>;
 vector<Z> A(a.size()), C(c.size());
 for(size_t i= 0; i < a.size(); ++i) A[i]= Z::raw(a[i]);
 for(size_t i= 0; i < c.size(); ++i) C[i]= Z::raw(c[i]);
 return linear_recurrence(C, A, k).val();
}
