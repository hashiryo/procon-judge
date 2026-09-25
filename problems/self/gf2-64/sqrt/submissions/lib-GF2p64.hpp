#include "mylib/algebra/GF2p64.hpp"
#include "vector"
using namespace std;
struct GF2_64Op {
 using u64= unsigned long long;
 static vector<u64> run(const vector<u64>& as) {
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= (u64)GF2p64(as[i]).sqrt();
  return ans;
 }
};
