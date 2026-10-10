#pragma once
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
// 逆元をまとめて求める関数の試作 (Montgomery の方法)。T は * と inv() を持つ型で、a のどの値も逆元を持つこと。
// 前から積を取り、全体の積の逆元を 1 回だけ求めて、後ろから 1 つずつの逆元を取り出す。掛け算は 1 要素あたり 3 回。
// 積の鎖を K 本に分け (添字を K で割った余りで分ける)、交互に進めて、掛け算の latency を待たずに次を始められるようにする。
// K 本の鎖の積は、同じ方法で K 個まとめて逆元にする。
template <int K, class T> vector<T> batch_inv(const vector<T>& a) {
 const int n= a.size();
 vector<T> q(n);
 if(n < 2 * K) {
  if(n == 0) return q;
  T p= a[0];
  q[0]= p;
  for(int i= 1; i < n; ++i) q[i]= p= p * a[i];
  T t= p.inv();
  for(int i= n - 1; i > 0; --i) q[i]= t * q[i - 1], t= t * a[i];
  q[0]= t;
  return q;
 }
 // 配列の添字を定数にしておかないと、p と t がレジスタに載らない。
 T p[K], s[K], t[K];
#pragma GCC unroll 16
 for(int j= 0; j < K; ++j) q[j]= p[j]= a[j];
 int i= K;
 for(; i + K <= n; i+= K) {
#pragma GCC unroll 16
  for(int j= 0; j < K; ++j) q[i + j]= p[j]= p[j] * a[i + j];
 }
 const int r= n - i;  // 端の r 個は鎖 0, ..., r - 1 の続き
#pragma GCC unroll 16
 for(int j= 0; j < K; ++j)
  if(j < r) q[i + j]= p[j]= p[j] * a[i + j];
 s[0]= p[0];
#pragma GCC unroll 16
 for(int j= 1; j < K; ++j) s[j]= s[j - 1] * p[j];
 T u= s[K - 1].inv();
#pragma GCC unroll 16
 for(int j= K - 1; j > 0; --j) t[j]= u * s[j - 1], u= u * p[j];
 t[0]= u;
#pragma GCC unroll 16
 for(int j= K - 1; j >= 0; --j)
  if(j < r) q[i + j]= t[j] * q[i + j - K], t[j]= t[j] * a[i + j];
 for(i-= K; i > 0; i-= K) {
#pragma GCC unroll 16
  for(int j= K - 1; j >= 0; --j) q[i + j]= t[j] * q[i + j - K], t[j]= t[j] * a[i + j];
 }
#pragma GCC unroll 16
 for(int j= 0; j < K; ++j) q[j]= t[j];
 return q;
}

// 法が奇数の定数の ZMod に限った版。値を Plantard の表現に直さずに、P(x, y) = -x y 2^{-64} mod M で掛ける。
// 前から P で積を取ると、i 番目の積には (-2^{-64})^i が付く。後ろからも P で掛けると付いた分が打ち消し合い、取り出した値が
// そのまま逆元になる (全体の積は付いた分ごと inv() で逆元にする)。後ろからの 2 回の掛け算は t rho を共有するので、
// 掛け算は 1 要素あたり 8 回になる (Barrett なら 9 回)。
template <int K, unsigned MOD> vector<ZMod<MOD>> batch_inv_plantard(const vector<ZMod<MOD>>& a) {
 using Z= ZMod<MOD>;
 static_assert(MOD & 1);
 constexpr u64 RHO= zmod_internal::inv64(MOD);
 auto red= [](u64 w) { return u32((u128(w | u32(-1)) * MOD) >> 64); };  // w = x y rho
 const int n= a.size();
 vector<Z> res(n);
 if(n == 0) return res;
 vector<u32> q(n), b(n);
 for(int i= 0; i < n; ++i) b[i]= a[i].val();
 const int k= n < 2 * K ? 1 : K;
 u32 p[K]{}, s[K]{}, t[K]{};
#pragma GCC unroll 16
 for(int j= 0; j < K; ++j)
  if(j < k) q[j]= p[j]= b[j];
 int i= k;
 if(k == K) {
  for(; i + K <= n; i+= K) {
#pragma GCC unroll 16
   for(int j= 0; j < K; ++j) q[i + j]= p[j]= red(u64(p[j]) * b[i + j] * RHO);
  }
 } else
  for(; i < n; ++i) q[i]= p[0]= red(u64(p[0]) * b[i] * RHO);
 const int r= n - i;
#pragma GCC unroll 16
 for(int j= 0; j < K; ++j)
  if(j < r) q[i + j]= p[j]= red(u64(p[j]) * b[i + j] * RHO);
 s[0]= p[0];
#pragma GCC unroll 16
 for(int j= 1; j < K; ++j)
  if(j < k) s[j]= red(u64(s[j - 1]) * p[j] * RHO);
 u32 u= Z(s[k - 1]).inv().val();
#pragma GCC unroll 16
 for(int j= K - 1; j > 0; --j)
  if(j < k) {
   const u64 ur= u * RHO;
   t[j]= red(ur * s[j - 1]), u= red(ur * p[j]);
  }
 t[0]= u;
#pragma GCC unroll 16
 for(int j= K - 1; j >= 0; --j)
  if(j < r) {
   const u64 tr= t[j] * RHO;
   q[i + j]= red(tr * q[i + j - k]), t[j]= red(tr * b[i + j]);
  }
 if(k == K) {
  for(i-= K; i > 0; i-= K) {
#pragma GCC unroll 16
   for(int j= K - 1; j >= 0; --j) {
    const u64 tr= t[j] * RHO;
    q[i + j]= red(tr * q[i + j - K]), t[j]= red(tr * b[i + j]);
   }
  }
 } else
  for(--i; i > 0; --i) {
   const u64 tr= t[0] * RHO;
   q[i]= red(tr * q[i - 1]), t[0]= red(tr * b[i]);
  }
#pragma GCC unroll 16
 for(int j= 0; j < K; ++j)
  if(j < k) q[j]= t[j];
 for(int j= 0; j < n; ++j) res[j]= Z::raw(q[j]);
 return res;
}

// ハーネスの形に合わせる。0 は逆元が無いので -1 を返し、ほかの値だけをまとめて逆元にする。
template <class F> inline vector<u32> run_batch(const vector<u32>& qs, F f) {
 using Z= ZMod<998244353>;
 vector<Z> a;
 a.reserve(qs.size());
 for(u32 x : qs)
  if(x % Z::mod()) a.push_back(Z(x));
 const vector<Z> iv= f(a);
 vector<u32> ans(qs.size());
 size_t k= 0;
 for(size_t i= 0; i < qs.size(); ++i) ans[i]= qs[i] % Z::mod() ? iv[k++].val() : u32(-1);
 return ans;
}
