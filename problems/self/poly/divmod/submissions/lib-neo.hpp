#pragma once
// NeoLibrary の poly_divmod (neo/base/core/poly/poly_divmod.hpp)。答えは末尾の 0 を落とすので、決まった長さに延ばして返す。
#include <utility>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/poly_divmod.hpp"
inline pair<vector<u32>, vector<u32>> run(const vector<u32>& a, const vector<u32>& b) {
 using Z= ZMod<998244353>;
 vector<Z> A(a.size()), B(b.size());
 for(size_t i= 0; i < a.size(); ++i) A[i]= Z::raw(a[i]);
 for(size_t i= 0; i < b.size(); ++i) B[i]= Z::raw(b[i]);
 const auto [Q, R]= poly_divmod(A, B);
 vector<u32> q(a.size() - b.size() + 1, 0), r(b.size() - 1, 0);
 for(size_t i= 0; i < Q.size(); ++i) q[i]= Q[i].val();
 for(size_t i= 0; i < R.size(); ++i) r[i]= R[i].val();
 return {q, r};
}
