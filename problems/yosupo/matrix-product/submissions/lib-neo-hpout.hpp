#pragma once
// 確かめ用。lib-neo の答えの vector<u32> だけを、_strassen_hp.hpp の out_vector (huge page を頼んでから 0 で埋める) で作る版。
// lib-neo との差は、答えの 4 MB を 4 KB のページで初めて触る分。
#include "../common.hpp"
#include "neo/linalg/Matrix.hpp"
#include "_strassen_hp.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) {
 using Z= ZMod<998244353>;
 Matrix<Z> A(N, M), B(M, P);
 for(size_t i= 0; i < a.size(); ++i) A.a[i]= Z::raw(a[i]);
 for(size_t i= 0; i < b.size(); ++i) B.a[i]= Z::raw(b[i]);
 const Matrix<Z> C= A * B;
 vector<u32> ret= strassen_hp::out_vector(C.a.size());
 for(size_t i= 0; i < ret.size(); ++i) ret[i]= C.a[i].val();
 return ret;
}
