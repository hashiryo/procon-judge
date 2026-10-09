// https://codeforces.com/gym/102341/problem/L
// 組の SG は成分の nim 積で、多重集合の SG は全部の置換にわたる nim 積の xor、つまり nimber の体での M のパーマネントになる。
// 標数 2 ではパーマネントは行列式に等しい。from_nimber で GF2p64 に移しても体の同型なので行列式が 0 かどうかは変わらず、掃き出して調べる
#include <iostream>
#include <vector>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n;
 cin >> n;
 vector a(n, vector<GF2p64>(n));
 for(auto& row: a)
  for(auto& v: row) {
   u64 x;
   cin >> x;
   v= GF2p64::from_nimber(x);
  }
 bool regular= true;
 for(int c= 0; c < n && regular; ++c) {
  int p= c;
  while(p < n && !a[p][c]) ++p;
  if(p == n) {
   regular= false;
   break;
  }
  swap(a[p], a[c]);
  const GF2p64 iv= a[c][c].inv();
  for(int i= c + 1; i < n; ++i)
   if(a[i][c]) {
    const GF2p64 m= a[i][c] * iv;
    for(int j= c; j < n; ++j) a[i][j]-= m * a[c][j];
   }
 }
 cout << (regular ? "First" : "Second") << '\n';
 return 0;
}
