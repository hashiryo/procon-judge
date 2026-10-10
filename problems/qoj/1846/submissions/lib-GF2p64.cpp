// https://qoj.ac/problem/1846
// 2^32 未満の nimber は部分体をなすので、from_nimber で GF2p64 に移して計算し、答えを to_nimber で戻す。
// 漸化式の特性多項式 P は次数 K-1 で、0 でない係数は 11 個以下。x^(m-1) mod P を 2 乗と x 倍の繰り返しで求める。
// 標数 2 では多項式の 2 乗は係数ごとの 2 乗 (フロベニウス写像) なので、2 乗は GF2p64 の square と疎な P での剰余だけで済む
#include <iostream>
#include <vector>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int K;
 u64 m;
 cin >> K >> m;
 const int D= K - 1;   // 漸化式の次数
 vector<GF2p64> a(D);  // a[i] は a_{i+1}
 vector<GF2p64> q(D + 1);  // a_n = Σ_{j=1}^{D} q[j] a_{n-j}
 auto read= [&]() {
  u64 x;
  cin >> x;
  return GF2p64::from_nimber(x);
 };
 for(auto& v: a) v= read();
 for(int i= 1; i <= 5; ++i) q[i]+= read();
 for(int i= 1; i <= 5; ++i) q[K - i]+= read();
 if(m <= u64(D)) return cout << a[m - 1].to_nimber() << '\n', 0;
 vector<pair<int, GF2p64>> terms;  // x^D = Σ q[j] x^(D-j) の右辺の 0 でない項 (次数, 係数)
 for(int j= 1; j <= D; ++j)
  if(q[j]) terms.emplace_back(D - j, q[j]);
 // 次数 D 以上の項を、上から順に x^D = Σ q[j] x^(D-j) で置き換える
 auto reduce= [&](vector<GF2p64>& r) {
  for(int e= int(r.size()) - 1; e >= D; --e) {
   if(!r[e]) continue;
   const GF2p64 c= r[e];
   r[e]= GF2p64();
   for(auto [d, w]: terms) r[e - D + d]+= c * w;
  }
  r.resize(D);
 };
 vector<GF2p64> r(D), s(2 * D - 1);  // r = x^(e の上位の bit) mod P
 r[0]= GF2p64(1);
 const u64 e= m - 1;
 for(int b= 63 - __builtin_clzll(e); b >= 0; --b) {
  s.assign(2 * D - 1, GF2p64());
  for(int i= 0; i < D; ++i) s[2 * i]= r[i].square();
  reduce(s), swap(r, s);
  if(e >> b & 1) r.insert(r.begin(), GF2p64()), reduce(r);  // x 倍
 }
 // a_m は、x^i を a_{i+1} に移す線形写像で x^(m-1) mod P を移したもの
 GF2p64 ans;
 for(int i= 0; i < D; ++i) ans+= r[i] * a[i];
 cout << ans.to_nimber() << '\n';
 return 0;
}
