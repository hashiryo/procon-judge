#include <bit>
#include <iostream>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
using namespace std;
// 区間 [l, r) の答は、その中で両端が l, r に近い順に足し引きして作る。
// 右端を 1 つ伸ばすときに増えるのは、新しい右端で終わる部分列の最小値の和で、
// これは「右端より左で最後に a[i] より小さい位置」の鎖 (狭義単調減少) を使うと
// f[i] - f[m] + a[m] * (m - l + 1) になる。m は [l, i] の最小値のうち右端に一番近いもの。
// 左端を伸ばすときは、逆向きの鎖と g で同じ形になる。
// 最小値の位置は sparse table で O(1) に取り、左右で違うので run の 4 引数の形を使う。
int main() {
 cin.tie(0), ios::sync_with_stdio(0);
 int n, q;
 cin >> n >> q;
 vector<long long> a(n);
 for(auto& x: a) cin >> x;
 // pre[i] は i より左で a[i] より真に小さい最後の位置。nxt[i] は右で最初の位置。
 // f[i] は a[0..i] の中で i で終わる部分列の最小値の和、g[i] は i で始まる部分列の和。
 vector<long long> f(n), g(n);
 {
  vector<int> stk;
  stk.reserve(n);
  for(int i= 0; i < n; ++i) {
   while(!stk.empty() && a[stk.back()] >= a[i]) stk.pop_back();
   const int p= stk.empty() ? -1 : stk.back();
   f[i]= (p < 0 ? 0 : f[p]) + a[i] * (long long)(i - p);
   stk.push_back(i);
  }
  stk.clear();
  for(int i= n - 1; i >= 0; --i) {
   while(!stk.empty() && a[stk.back()] >= a[i]) stk.pop_back();
   const int p= stk.empty() ? n : stk.back();
   g[i]= (p == n ? 0 : g[p]) + a[i] * (long long)(p - i);
   stk.push_back(i);
  }
 }
 // st_lo / st_hi は区間の最小値の位置。同点のとき st_lo は左、st_hi は右を選ぶ。
 const int lg= (int)bit_width((unsigned)n);
 vector<int> st_lo((size_t)lg * n), st_hi((size_t)lg * n);
 for(int i= 0; i < n; ++i) st_lo[i]= st_hi[i]= i;
 for(int k= 1; k < lg; ++k) {
  const int half= 1 << (k - 1), len= 1 << k;
  for(int i= 0; i + len <= n; ++i) {
   const int l1= st_lo[(size_t)(k - 1) * n + i], r1= st_lo[(size_t)(k - 1) * n + i + half];
   st_lo[(size_t)k * n + i]= a[l1] <= a[r1] ? l1 : r1;
   const int l2= st_hi[(size_t)(k - 1) * n + i], r2= st_hi[(size_t)(k - 1) * n + i + half];
   st_hi[(size_t)k * n + i]= a[l2] < a[r2] ? l2 : r2;
  }
 }
 auto argmin_lo= [&](int l, int r) {  // [l, r] の最小値の位置 (一番左)
  const int k= (int)bit_width((unsigned)(r - l + 1)) - 1;
  const int x= st_lo[(size_t)k * n + l], y= st_lo[(size_t)k * n + r - (1 << k) + 1];
  return a[x] <= a[y] ? x : y;
 };
 auto argmin_hi= [&](int l, int r) {  // [l, r] の最小値の位置 (一番右)
  const int k= (int)bit_width((unsigned)(r - l + 1)) - 1;
  const int x= st_hi[(size_t)k * n + l], y= st_hi[(size_t)k * n + r - (1 << k) + 1];
  return a[x] < a[y] ? x : y;
 };
 Mo mo;
 for(int i= 0; i < q; ++i) {
  int l, r;
  cin >> l >> r, --l;
  mo.query(l, r);
 }
 vector<long long> ans(q);
 long long cur= 0;
 int cl= 0, cr= 0;
 auto add_l= [&](int i) {
  const int m= argmin_lo(i, cr - 1);
  cur+= g[i] - g[m] + a[m] * (long long)(cr - m);
  cl= i;
 };
 auto add_r= [&](int i) {
  const int m= argmin_hi(cl, i);
  cur+= f[i] - f[m] + a[m] * (long long)(m - cl + 1);
  cr= i + 1;
 };
 auto erase_l= [&](int i) {
  const int m= argmin_lo(i, cr - 1);
  cur-= g[i] - g[m] + a[m] * (long long)(cr - m);
  cl= i + 1;
 };
 auto erase_r= [&](int i) {
  const int m= argmin_hi(cl, i);
  cur-= f[i] - f[m] + a[m] * (long long)(m - cl + 1);
  cr= i;
 };
 auto out= [&](int id) { ans[id]= cur; };
 mo.run(add_l, add_r, erase_l, erase_r, out);
 for(int i= 0; i < q; ++i) cout << ans[i] << '\n';
}
