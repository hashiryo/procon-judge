#pragma once
// 2 本同時の ECM で、stage 2 の baby step を 6 刻みの鎖で作り、素数を含まない組を飛ばすもの。中身は _ecm2.hpp。
#include "_ecm2.hpp"
inline vector<vector<u64>> run(const vector<u64>& qs) {
 vector<vector<u64>> ans;
 ans.reserve(qs.size());
 for(u64 x: qs) ans.push_back(ecm_fact::factorize2<false, true>(x));
 return ans;
}
