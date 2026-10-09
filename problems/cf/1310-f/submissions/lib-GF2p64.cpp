// https://codeforces.com/problemset/problem/1310/F
// nimber の離散対数。from_nimber で GF2p64 に移して log(base) を呼ぶ (体の同型なので冪はそのまま移る)。解が無ければ log は 2^64-1 を返す
#include <iostream>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int t;
 cin >> t;
 while(t--) {
  u64 a, b;
  cin >> a >> b;
  const u64 x= GF2p64::from_nimber(b).log(GF2p64::from_nimber(a));
  if(x == ~0ull) cout << -1 << '\n';
  else cout << x << '\n';
 }
 return 0;
}
