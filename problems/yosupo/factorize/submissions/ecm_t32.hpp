#pragma once
// 2^32 以上の合成数を ECM (B1 = 150、B2 ≈ 3400) で、それより小さければ Pollard rho で割る。中身は _ecm.hpp。
#include "_ecm.hpp"
inline vector<vector<u64>> run(const vector<u64>& qs) {
 vector<vector<u64>> ans;
 ans.reserve(qs.size());
 for(u64 x: qs) ans.push_back(ecm_fact::factorize<32>(x));
 return ans;
}
