#pragma once
// _neo_frob_ab.hpp の FIX = true (dot の最後を分岐のない u64 の和にした形)。lib-neo と同じく Matrix<ZMod<998244353>> から呼ぶ。決めたら消す。
#include "_neo_frob_ab.hpp"
inline vector<u32> run(int N, u64 K, const vector<u32>& a) {
 using Z= ZMod<998244353>;
 Matrix<Z> A(N, N);
 for(size_t i= 0; i < a.size(); ++i) A.a[i]= Z::raw(a[i]);
 Matrix<Z> R(N, N);
 neo_frob_ab::pow<Z, true>(A.a.data(), N, K, R.a.data());
 vector<u32> ret(R.a.size());
 for(size_t i= 0; i < ret.size(); ++i) ret[i]= R.a[i].val();
 return ret;
}
