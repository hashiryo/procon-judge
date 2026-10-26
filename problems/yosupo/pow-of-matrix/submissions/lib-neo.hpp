#pragma once
// NeoLibrary の Matrix<ZMod<998244353>>::pow (neo/linalg/Matrix.hpp)。法が素数で辺と指数が大きいので、zmod_frob と同じ
// Frobenius 標準形の道を通る。小さい辺や指数では、Montgomery の表現のまま掛け続ける二分累乗 (zmod_tile と同じ核) になる。
#include "../common.hpp"
#include "neo/linalg/Matrix.hpp"
inline vector<u32> run(int N, u64 K, const vector<u32>& a) {
 using Z= ZMod<998244353>;
 Matrix<Z> A(N, N);
 for(size_t i= 0; i < a.size(); ++i) A.a[i]= Z::raw(a[i]);
 const Matrix<Z> R= A.pow(K);
 vector<u32> ret(R.a.size());
 for(size_t i= 0; i < ret.size(); ++i) ret[i]= R.a[i].val();
 return ret;
}
