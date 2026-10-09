// https://atcoder.jp/contests/abc150/tasks/abc150_f
// GF2p64 のローリングハッシュ。a_i をそのまま GF2p64 の元とみると xor は足し算なので、回した a の全項に x を xor した列の
// ハッシュは、回した a のハッシュに x (1 + r + ... + r^(N-1)) を足したものになる
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
 const GF2p64 r= GF2p64(rng() | 2);
 int N;
 cin >> N;
 vector<u64> a(N), b(N);
 for(auto& v: a) cin >> v;
 for(auto& v: b) cin >> v;
 vector<GF2p64> H(N + 1), pw(N + 1, GF2p64(1));  // H[i] は a[0, i) のハッシュ
 GF2p64 hb, ones;                                 // b のハッシュと、1 が N 個並んだ列のハッシュ
 for(int i= 0; i < N; ++i) H[i + 1]= H[i] * r + GF2p64(a[i]), pw[i + 1]= pw[i] * r, hb= hb * r + GF2p64(b[i]), ones= ones * r + GF2p64(1);
 for(int k= 0; k < N; ++k) {
  const u64 x= a[k] ^ b[0];
  // a[k, N) のあとに a[0, k) を並べた列のハッシュ
  const GF2p64 rot= (H[N] - H[k] * pw[N - k]) * pw[k] + H[k];
  if(rot + GF2p64(x) * ones == hb) cout << k << " " << x << '\n';
 }
 return 0;
}
