#pragma once
// 確かめ用。NeoLibrary の行列の積の核 (linalg_internal::zmat::mul) を、入力の vector<u32> を ZMod の配列と見て直接呼び、答えは
// _strassen_hp.hpp の out_vector (huge page を頼んでから 0 で埋める) に書く。lib-neo との差は Matrix に写す分と答えを写す分で、
// zmod_winograd_hpo との差は核の中の違い (実行時の法、還元の前の畳み込み)。
#include "../common.hpp"
#include "neo/linalg/Matrix.hpp"
#include "_strassen_hp.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) {
 using Z= ZMod<998244353>;
 static_assert(sizeof(Z) == sizeof(u32));
 vector<u32> ret= strassen_hp::out_vector((size_t)N * P);
 linalg_internal::zmat::mul(reinterpret_cast<const Z*>(a.data()), reinterpret_cast<const Z*>(b.data()), reinterpret_cast<Z*>(ret.data()), N, M, P);
 return ret;
}
