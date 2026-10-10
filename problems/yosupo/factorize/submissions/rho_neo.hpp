#pragma once
// _ecm.hpp の試し割りと Pollard rho (Brent、2 本の列) だけで割る (ECM を使わない)。ecm_t40 などと比べるための対照。
#include "_ecm.hpp"
inline vector<vector<u64>> run(const vector<u64>& qs) {
 vector<vector<u64>> ans;
 ans.reserve(qs.size());
 for(u64 x: qs) ans.push_back(ecm_fact::factorize<61>(x));
 return ans;
}
