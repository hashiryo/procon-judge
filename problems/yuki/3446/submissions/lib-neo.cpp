#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
using namespace std;
// 区間 [L, R) の値の多重集合を持ち、値の集合の隣り合う差の多重集合を 64 分木で保つ。
// 値を 1 つ足す/外すと、その前後の値との差が 3 つだけ入れ替わる。
// 差の問い合わせは、差の 64 分木で X 以下の最大/以上の最小を返す。
int main() {
 int n, q;
 scanf("%d %d", &n, &q);
 vector<int> av(n);
 for(auto& x: av) scanf("%d", &x);
 vector<int> xs(av);
 sort(xs.begin(), xs.end()), xs.erase(unique(xs.begin(), xs.end()), xs.end());
 const int u= (int)xs.size();
 vector<int> rank_(n);
 for(int i= 0; i < n; ++i) rank_[i]= (int)(lower_bound(xs.begin(), xs.end(), av[i]) - xs.begin());
 const int dn= xs.back() - xs.front() + 1;  // 差は 0 .. dn-1
 vector<int> ql(q), qr(q), qx(q);
 vector<char> qc(q);
 Mo mo;
 for(int i= 0; i < q; ++i) {
  int l, r, x;
  char c[3];
  scanf("%d %d %d %2s", &l, &r, &x, c), --l;
  ql[i]= l, qr[i]= r, qx[i]= x, qc[i]= c[0];
  mo.query(l, r);
 }
 // 値の集合 (圧縮した順位) の 64 分木。
 const int nv0= (u + 63) >> 6, nv1= (nv0 + 63) >> 6, nv2= (nv1 + 63) >> 6;
 vector<unsigned long long> v0(nv0, 0), v1(nv1, 0), v2(nv2, 0);
 auto set_val= [&](int r) {
  int i= r >> 6;
  const unsigned long long old= v0[i];
  v0[i]= old | (1ull << (r & 63));
  if(old) return;
  const int j= i >> 6;
  const unsigned long long old1= v1[j];
  v1[j]= old1 | (1ull << (i & 63));
  if(old1) return;
  v2[j >> 6]|= 1ull << (j & 63);
 };
 auto clear_val= [&](int r) {
  int i= r >> 6;
  v0[i]&= ~(1ull << (r & 63));
  if(v0[i]) return;
  const int j= i >> 6;
  v1[j]&= ~(1ull << (i & 63));
  if(v1[j]) return;
  v2[j >> 6]&= ~(1ull << (j & 63));
 };
 auto pred_val= [&](int r) {
  if(r < 0) return -1;
  int i= r >> 6;
  unsigned long long w= v0[i] & ((r & 63) == 63 ? ~0ull : ((1ull << ((r & 63) + 1)) - 1));
  if(w) return (i << 6) + 63 - __builtin_clzll(w);
  int j= i >> 6;
  w= v1[j] & ((i & 63) ? ((1ull << (i & 63)) - 1) : 0);
  if(!w) {
   w= v2[0] & (j ? ((1ull << j) - 1) : 0);
   if(!w) return -1;
   j= 63 - __builtin_clzll(w);
   w= v1[j];
  }
  i= (j << 6) + 63 - __builtin_clzll(w);
  w= v0[i];
  return (i << 6) + 63 - __builtin_clzll(w);
 };
 auto succ_val= [&](int r) {
  if(r >= u) return u;
  int i= r >> 6;
  unsigned long long w= v0[i] & (~0ull << (r & 63));
  if(w) return (i << 6) + __builtin_ctzll(w);
  const int j= i >> 6;
  w= v1[j] & ((i & 63) == 63 ? 0 : (~0ull << ((i & 63) + 1)));
  if(!w) {
   w= v2[0] & (j == 63 ? 0 : (~0ull << (j + 1)));
   if(!w) return u;
   int jj= __builtin_ctzll(w);
   w= v1[jj], i= (jj << 6) + __builtin_ctzll(w);
   w= v0[i];
   return (i << 6) + __builtin_ctzll(w);
  }
  i= (j << 6) + __builtin_ctzll(w);
  w= v0[i];
  return (i << 6) + __builtin_ctzll(w);
 };
 // 差の多重集合の 64 分木 (論理和で持つ) と、差ごとの個数。
 const int d0n= (dn + 63) >> 6, d1n= (d0n + 63) >> 6, d2n= (d1n + 63) >> 6, d3n= (d2n + 63) >> 6;
 vector<unsigned long long> d0(d0n, 0), d1(d1n, 0), d2(d2n, 0), d3(d3n, 0);
 vector<unsigned> dcnt(dn, 0);
 auto set_diff= [&](int x) {
  int i= x >> 6;
  const unsigned long long old= d0[i];
  d0[i]= old | (1ull << (x & 63));
  if(old) return;
  int j= i >> 6;
  const unsigned long long old1= d1[j];
  d1[j]= old1 | (1ull << (i & 63));
  if(old1) return;
  int k= j >> 6;
  const unsigned long long old2= d2[k];
  d2[k]= old2 | (1ull << (j & 63));
  if(old2) return;
  d3[k >> 6]|= 1ull << (k & 63);
 };
 auto clear_diff= [&](int x) {
  int i= x >> 6;
  d0[i]&= ~(1ull << (x & 63));
  if(d0[i]) return;
  int j= i >> 6;
  d1[j]&= ~(1ull << (i & 63));
  if(d1[j]) return;
  int k= j >> 6;
  d2[k]&= ~(1ull << (j & 63));
  if(d2[k]) return;
  d3[k >> 6]&= ~(1ull << (k & 63));
 };
 auto pred_diff= [&](int x) {
  if(x >= dn) x= dn - 1;
  if(x < 0) return -1;
  int i= x >> 6;
  unsigned long long w= d0[i] & ((x & 63) == 63 ? ~0ull : ((1ull << ((x & 63) + 1)) - 1));
  if(w) return (i << 6) + 63 - __builtin_clzll(w);
  int j= i >> 6;
  w= d1[j] & ((i & 63) ? ((1ull << (i & 63)) - 1) : 0);
  if(!w) {
   int k= j >> 6;
   w= d2[k] & ((j & 63) ? ((1ull << (j & 63)) - 1) : 0);
   if(!w) {
    w= d3[0] & (k ? ((1ull << k) - 1) : 0);
    if(!w) return -1;
    k= 63 - __builtin_clzll(w);
    w= d2[k];
    j= (k << 6) + 63 - __builtin_clzll(w);
    w= d1[j];
   } else {
    j= (k << 6) + 63 - __builtin_clzll(w);
    w= d1[j];
   }
  }
  i= (j << 6) + 63 - __builtin_clzll(w);
  w= d0[i];
  return (i << 6) + 63 - __builtin_clzll(w);
 };
 auto succ_diff= [&](int x) {
  if(x < 0) x= 0;
  if(x >= dn) return dn;
  int i= x >> 6;
  unsigned long long w= d0[i] & (~0ull << (x & 63));
  if(w) return (i << 6) + __builtin_ctzll(w);
  int j= i >> 6;
  w= d1[j] & ((i & 63) == 63 ? 0 : (~0ull << ((i & 63) + 1)));
  if(!w) {
   int k= j >> 6;
   w= d2[k] & ((j & 63) == 63 ? 0 : (~0ull << ((j & 63) + 1)));
   if(!w) {
    w= d3[0] & (k == 63 ? 0 : (~0ull << (k + 1)));
    if(!w) return dn;
    k= __builtin_ctzll(w);
    w= d2[k];
    j= (k << 6) + __builtin_ctzll(w);
    w= d1[j];
   } else {
    j= (k << 6) + __builtin_ctzll(w);
    w= d1[j];
   }
  }
  i= (j << 6) + __builtin_ctzll(w);
  w= d0[i];
  return (i << 6) + __builtin_ctzll(w);
 };
 auto add_diff= [&](int x) {
  if(dcnt[x]++ == 0) set_diff(x);
 };
 auto erase_diff= [&](int x) {
  if(--dcnt[x] == 0) clear_diff(x);
 };
 vector<int> cnt_val(u, 0), ans(q);
 auto add= [&](int pos) {
  const int r= rank_[pos];
  if(cnt_val[r]++ != 0) {  // 同じ値がすでにある: 差 0 が 1 つ増える
   add_diff(0);
   return;
  }
  const int p= pred_val(r - 1), s= succ_val(r + 1);
  if(p >= 0 && s < u) erase_diff(xs[s] - xs[p]);
  if(p >= 0) add_diff(xs[r] - xs[p]);
  if(s < u) add_diff(xs[s] - xs[r]);
  set_val(r);
 };
 auto erase= [&](int pos) {
  const int r= rank_[pos];
  if(--cnt_val[r] != 0) {
   erase_diff(0);
   return;
  }
  const int p= pred_val(r - 1), s= succ_val(r + 1);
  if(p >= 0 && s < u) add_diff(xs[s] - xs[p]);
  if(p >= 0) erase_diff(xs[r] - xs[p]);
  if(s < u) erase_diff(xs[s] - xs[r]);
  clear_val(r);
 };
 auto out= [&](int id) {
  int v;
  if(qc[id] == 'L') v= pred_diff(qx[id]);
  else v= succ_diff(qx[id]), v= (v >= dn ? -1 : v);
  ans[id]= v;
 };
 mo.run(add, erase, out);
 for(int i= 0; i < q; ++i) printf("%d\n", ans[i]);
 return 0;
}
