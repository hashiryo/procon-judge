#pragma once
#include "mylib/algebra/GF2p64.hpp"
#include <vector>
using namespace std;
struct NimProduct {
 using u64= unsigned long long;
 static vector<u64> run(const vector<u64>& as, const vector<u64>& bs) {
  const size_t T= as.size();
  vector<u64> ans(T);
  for(size_t i= 0; i < T; ++i) {
   ans[i]= (GF2p64::from_nimber(as[i]) * GF2p64::from_nimber(bs[i])).to_nimber();
  }
  return ans;
 }
};
