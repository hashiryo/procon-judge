#pragma once
// 確かめ用。lib-neo-core と同じ呼び方で、1024 次に広げる積だけ、NeoLibrary の Strassen の道をブロックの 1 辺 32 で直接呼ぶ
// (linalg_internal::zmat::strassen_mul<32>)。再帰が 1 段深くなる (zmod_winograd_hpo に対する zmod_winograd32_hpo と同じ変え方)。
// ほかの大きさは lib-neo-core と同じ zmat::mul。
#include "../common.hpp"
#include "neo/linalg/Matrix.hpp"
#include "_strassen_hp.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) {
 using Z= ZMod<998244353>;
 namespace zm= linalg_internal::zmat;
 static_assert(sizeof(Z) == sizeof(u32));
 vector<u32> ret= strassen_hp::out_vector((size_t)N * P);
 const Z *A= reinterpret_cast<const Z*>(a.data()), *B= reinterpret_cast<const Z*>(b.data());
 Z* C= reinterpret_cast<Z*>(ret.data());
 if(const size_t S= zm::strassen_side(N, M, P); S >= 1024) zm::strassen_mul<32>(A, B, C, N, M, P, S, zm::Mod(Z::mod()));
 else zm::mul(A, B, C, N, M, P);
 return ret;
}
