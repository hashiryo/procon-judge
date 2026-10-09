// https://atcoder.jp/contests/abc274/tasks/abc274_h
// GF2p64 のローリングハッシュ。A_i をそのまま GF2p64 の元とみると、xor は足し算なので、
// 2 つの区間を xor した列のハッシュは、それぞれのハッシュの和になる
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
int main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
 const GF2p64 r= GF2p64(rng() | 2);
 int N, Q;
 cin >> N >> Q;
 vector<u64> A(N);
 for(auto& a: A) cin >> a;
 vector<GF2p64> H(N + 1), pw(N + 1, GF2p64(1));  // H[i] は A[0, i) のハッシュ
 for(int i= 0; i < N; i++) H[i + 1]= H[i] * r + GF2p64(A[i]), pw[i + 1]= pw[i] * r;
 auto hash= [&](int l, int len) { return H[l + len] - H[l] * pw[len]; };
 while(Q--) {
  int a, b, c, d, e, f;
  cin >> a >> b >> c >> d >> e >> f;
  a--, c--, e--;
  const int r1= b - a, r2= f - e, r= min(r1, r2);
  int ok= 0, ng= r + 1;
  while(ng - ok > 1) {
   const int x= (ok + ng) / 2;
   (hash(a, x) + hash(c, x) == hash(e, x) ? ok : ng)= x;
  }
  if(ok == r) cout << (r1 < r2 ? "Yes" : "No") << '\n';
  else cout << ((A[a + ok] ^ A[c + ok]) < A[e + ok] ? "Yes" : "No") << '\n';
 }
 return 0;
}
