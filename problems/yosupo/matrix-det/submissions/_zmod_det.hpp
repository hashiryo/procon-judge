#pragma once
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
// NeoLibrary の ZMod で行列式を掃き出しで求める試作 (行列のヘッダの掃き出しの核)。法は素数とする (要の逆元を inv() で取る)。
// 設計の記録は algo-notes の notes/modular_arithmetic/modint_problems.md。

// ZMod の演算子だけで書く。
template <class Z> Z det_naive(vector<vector<Z>> a) {
 const int n= a.size();
 Z d= Z::raw(1);
 for(int c= 0; c < n; ++c) {
  int p= c;
  while(p < n && a[p][c] == Z()) ++p;
  if(p == n) return Z();
  if(p != c) swap(a[p], a[c]), d= -d;
  d*= a[c][c];
  const Z iv= a[c][c].inv();
  for(int r= c + 1; r < n; ++r) {
   const Z f= a[r][c] * iv;
   if(f == Z()) continue;
   for(int j= c + 1; j < n; ++j) a[r][j]-= f * a[c][j];
  }
 }
 return d;
}

// 行ごとの掛ける数 -f を fixed() で前計算してから掛ける。法が奇数の定数なら Plantard の掛け算 2 回で済む。
template <class Z> Z det_fixed(vector<vector<Z>> a) {
 const int n= a.size();
 Z d= Z::raw(1);
 for(int c= 0; c < n; ++c) {
  int p= c;
  while(p < n && a[p][c] == Z()) ++p;
  if(p == n) return Z();
  if(p != c) swap(a[p], a[c]), d= -d;
  d*= a[c][c];
  const Z iv= a[c][c].inv();
  const Z* pr= a[c].data();
  for(int r= c + 1; r < n; ++r) {
   const Z f= a[r][c] * iv;
   if(f == Z()) continue;
   const auto F= (-f).fixed();
   Z* row= a[r].data();
   for(int j= c + 1; j < n; ++j) row[j]+= pr[j] * F;
  }
 }
 return d;
}

// 行を u64 で持ち、要の行 (M 未満に直したもの) を -f 倍して足す。積は M^2 < 2^60 なので、行ごとに足した回数を数え、
// 8 回ごとに 2^32 の位を 2^32 mod M に掛けて畳む (行列積の zmod_acc と同じ)。畳んだ値は 2^62 未満なので、8 回足しても
// u64 に入る。要を探す列と要の行だけを M 未満に直す。列を L 個ずつの塊で回し、行の幅を L の倍数まで 0 で埋めて、
// 塊の始まりを要の列から L の倍数に切り下げる (切り下げた分の要の行は 0 なので値は変わらない)。GCC の -O2 は回数が
// L の倍数と分かるループだけをベクトル化するため。
template <class Z> Z det_lazy(const vector<vector<u32>>& in) {
 constexpr int L= 8;
 const int n= in.size(), w= (n + L - 1) / L * L;
 const u32 r32= u32((u64(1) << 32) % Z::mod());
 vector<u64> a((size_t)n * w);
 for(int r= 0; r < n; ++r)
  for(int j= 0; j < n; ++j) a[(size_t)r * w + j]= Z(in[r][j]).val();
 vector<u32> piv(w);
 vector<int> cnt(n);
 auto full= [](u64 x) { return Z(x).val(); };
 Z d= Z::raw(1);
 for(int c= 0; c < n; ++c) {
  int p= -1;
  for(int r= c; r < n; ++r) {
   u64& x= a[(size_t)r * w + c];
   x= full(x);
   if(p < 0 && x) p= r;
  }
  if(p < 0) return Z();
  if(p != c) swap_ranges(&a[(size_t)p * w], &a[(size_t)p * w] + w, &a[(size_t)c * w]), swap(cnt[p], cnt[c]), d= -d;
  const int j0= c / L * L;
  for(int j= j0; j < w; ++j) piv[j]= j < c ? 0 : full(a[(size_t)c * w + j]);
  d*= Z::raw(piv[c]);
  const Z iv= Z::raw(piv[c]).inv();
  for(int r= c + 1; r < n; ++r) {
   u64* __restrict row= &a[(size_t)r * w];
   const u32 x= u32(row[c]);  // 要を探したときに M 未満に直してある
   if(!x) continue;
   const u32 f= (-(Z::raw(x) * iv)).val();
   const u32* __restrict pv= piv.data();
   for(int j= j0; j < w; j+= L)
    for(int l= 0; l < L; ++l) row[j + l]+= u64(f) * pv[j + l];
   if(++cnt[r] == 8) {
    for(int j= j0; j < w; j+= L)
     for(int l= 0; l < L; ++l) row[j + l]= u64(u32(row[j + l] >> 32)) * r32 + u32(row[j + l]);
    cnt[r]= 0;
   }
  }
 }
 return d;
}

template <class F> inline u32 run_det(const vector<vector<u32>>& a, F f) {
 using Z= ZMod<998244353>;
 vector<vector<Z>> b(a.size());
 for(size_t i= 0; i < a.size(); ++i) {
  b[i].resize(a[i].size());
  for(size_t j= 0; j < a[i].size(); ++j) b[i][j]= Z(a[i][j]);
 }
 return f(std::move(b)).val();
}
