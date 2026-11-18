#pragma once
// 疎な f の 1/f。g_i = Σ_k w_k g_{i - e_k} (w_k = -a_k / a_0、e_k ≥ 1) を前から求める。
// 項は次数の昇順なので、i が e_k を越えるたびに使う項が 1 つ増える。区間ごとに項の数をテンプレートの引数にして、
// 積を u64 にためて 1 回だけ剰余を取る (9 項 × M^2 < 2^63)。いちばん次数の低い項 (多くは e = 1) が待ちの鎖になるので、
// 直前の値はレジスタに持ち、その掛け算は掛ける数が決まっているので Shoup の方法にする。
#include <algorithm>
#include <vector>
namespace sp1 {
constexpr u32 M= 998244353;
inline u32 mulmod(u32 a, u32 b) { return u32(u64(a) * b % M); }
inline u32 powmod(u32 a, u64 e) {
 u32 r= 1;
 for(; e; e>>= 1, a= mulmod(a, a))
  if(e & 1) r= mulmod(r, a);
 return r;
}
// a < 2^32 に x を掛けて 2M 未満で返す (q = floor(x 2^32 / M))。
inline u32 shoup(u32 a, u32 x, u32 q) { return a * x - u32((u64(a) * q) >> 32) * M; }
// [lo, hi) の g を、T 項 (次数 e[0..T)、係数 w[0..T)) の和で求める。
template <int T> void seg(u32* g, int lo, int hi, const int* e, const u32* w) {
 if constexpr(T == 0) {
  std::fill(g + lo, g + hi, 0u);
 } else {
  if(lo >= hi) return;
  if(e[0] == 1) {
   const u32 w0= w[0], q0= u32((u64(w0) << 32) / M);
   u32 prev= g[lo - 1];
   for(int i= lo; i < hi; ++i) {
    u64 acc= 0;
    for(int k= 1; k < T; ++k) acc+= u64(w[k]) * g[i - e[k]];
    u32 r= u32(acc % M) + shoup(prev, w0, q0);  // 3M 未満
    r= r >= 2 * M ? r - 2 * M : r;
    r= r >= M ? r - M : r;
    g[i]= prev= r;
   }
  } else {
   for(int i= lo; i < hi; ++i) {
    u64 acc= 0;
    for(int k= 0; k < T; ++k) acc+= u64(w[k]) * g[i - e[k]];
    g[i]= u32(acc % M);
   }
  }
 }
}
template <int... Ts> void seg_dispatch(int t, u32* g, int lo, int hi, const int* e, const u32* w, std::integer_sequence<int, Ts...>) { ((t == Ts ? seg<Ts>(g, lo, hi, e, w) : void()), ...); }
}  // namespace sp1
inline vector<u32> run(int n, const vector<pair<int, u32>>& f) {
 using namespace sp1;
 vector<u32> g(n);
 const u32 a0= f[0].second, ia= powmod(a0, M - 2);
 int e[10];
 u32 w[10];
 int t= 0;
 for(size_t k= 1; k < f.size(); ++k) {
  if(f[k].first >= n) break;
  e[t]= f[k].first, w[t]= mulmod(M - f[k].second, ia), ++t;
 }
 g[0]= ia;
 for(int j= 0; j <= t; ++j) {
  const int lo= j == 0 ? 1 : e[j - 1], hi= j == t ? n : e[j];
  seg_dispatch(j, g.data(), std::max(lo, 1), hi, e, w, std::make_integer_sequence<int, 10>{});
 }
 return g;
}
