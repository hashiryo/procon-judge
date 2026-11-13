#pragma once
// 確かめ用。lib-neo-core と同じ呼び方で、NeoLibrary の Strassen の道をブロックの 1 辺 32 にした写し (_neo_strassen_sb.hpp) を
// 通す。1024 次では再帰が 1 段深くなる (zmod_winograd_hpo に対する zmod_winograd32_hpo と同じ変え方)。
#include "_neo_strassen_sb.hpp"
#include "_strassen_hp.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) {
 using Z= ZMod<998244353>;
 static_assert(sizeof(Z) == sizeof(u32));
 vector<u32> ret= strassen_hp::out_vector((size_t)N * P);
 neo_sb::mul<32>(reinterpret_cast<const Z*>(a.data()), reinterpret_cast<const Z*>(b.data()), reinterpret_cast<Z*>(ret.data()), N, M, P);
 return ret;
}
