#include "mylib/algebra/GF2p64.hpp"
#include <vector>
using namespace std;
using u64= unsigned long long;
// Library の GF2p64::log(base) (base^k = x となる最小の k。無ければ 2^64-1) をそのまま呼ぶ。
inline vector<u64> run(const vector<u64>& as, const vector<u64>& bs) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= GF2p64(bs[i]).log(GF2p64(as[i]));
 return ans;
}
