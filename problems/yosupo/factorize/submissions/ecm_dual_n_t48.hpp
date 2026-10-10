#pragma once
// 2 本同時の ECM で、stage 1 を始点を正規化した 1 本の ladder にしたもの (B1 = 150、B2 ≈ 3400)。中身は _ecm2.hpp。
#include "_ecm2.hpp"
inline vector<vector<u64>> run(const vector<u64>& qs) {
 vector<vector<u64>> ans;
 ans.reserve(qs.size());
 for(u64 x: qs) ans.push_back(ecm_fact::factorize2<true, false>(x));
 return ans;
}
