#include "mylib/algebra/GF2p64.hpp"
#include <vector>
using namespace std;
using u64= unsigned long long;
inline vector<u64> run(const vector<u64>& as, const vector<u64>& bs) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= (u64)(GF2p64(as[i]) * GF2p64(bs[i]));
 return ans;
}
