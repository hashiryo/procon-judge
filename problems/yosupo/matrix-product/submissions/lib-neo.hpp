#pragma once
// NeoLibrary の Matrix<ZMod<998244353>> の積 (neo/linalg/Matrix.hpp)。大きさで核を選び、1024 × 1024 は Strassen の再帰
// (zmod_strassen の法を実行時の奇数に広げたもの)、ほかは 4 行 × 8 列のタイル。値の写しも計測区間に入る。
#include "../common.hpp"
#include "neo/linalg/Matrix.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) {
 using Z= ZMod<998244353>;
 Matrix<Z> A(N, M), B(M, P);
 for(size_t i= 0; i < a.size(); ++i) A.a[i]= Z::raw(a[i]);
 for(size_t i= 0; i < b.size(); ++i) B.a[i]= Z::raw(b[i]);
 const Matrix<Z> C= A * B;
 vector<u32> ret(C.a.size());
 for(size_t i= 0; i < ret.size(); ++i) ret[i]= C.a[i].val();
 return ret;
}
