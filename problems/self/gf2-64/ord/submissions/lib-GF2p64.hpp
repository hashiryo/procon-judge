// Library の GF2p64::ord() をそのまま呼ぶ。中身は frobenius_il16 と同じ手順。
#include "neo/algebra/GF2p64.hpp"
#include <vector>
using namespace std;
using u64= unsigned long long;
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= GF2p64(as[i]).ord();
 return ans;
}
