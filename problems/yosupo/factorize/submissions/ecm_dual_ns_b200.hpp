#pragma once
// ecm_dual_ns_t48 の B1 を 200、m の上限を 14 にしたもの。中身は _ecm2.hpp。
#include "_ecm2.hpp"
inline vector<vector<u64>> run(const vector<u64>& qs) {
 vector<vector<u64>> ans;
 ans.reserve(qs.size());
 for(u64 x: qs) ans.push_back(ecm_fact::factorize2<true, true, 200, 14>(x));
 return ans;
}
