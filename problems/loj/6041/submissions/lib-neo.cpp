#include <bit>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include "neo/base/ds/string/suffix_array.hpp"
#include "neo/base/engine/query/RollbackMo.hpp"
using namespace std;
namespace io {
constexpr int BUF= 1 << 16;
char buf[BUF];
int len= 0, pos= 0;
inline int get_char() {
 if(pos == len) len= (int)fread(buf, 1, BUF, stdin), pos= 0;
 return pos < len ? buf[pos++] : -1;
}
inline int read_int() {
 int c= get_char();
 while(c < '0' || c > '9') c= get_char();
 int x= 0;
 while('0' <= c && c <= '9') x= x * 10 + (c - '0'), c= get_char();
 return x;
}
inline void read_word(string& s) {
 int c= get_char();
 while(c <= ' ') c= get_char();
 s.clear();
 while(c > ' ') s.push_back((char)c), c= get_char();
}
}  // namespace io
namespace {
// 0 <= i < n の集合。追加、削除、i 未満の最大、i 以上の最小、全体の最大を O(1) 程度で返す。
struct BitSet {
 int n= 0, wc= 0, sc= 0;
 vector<uint64_t> w, sw;
 void init(int n_) {
  n= n_, wc= (n + 63) >> 6, sc= (wc + 63) >> 6;
  w.assign(wc, 0), sw.assign(sc, 0);
 }
 void add(int i) {
  int a= i >> 6;
  if(!w[a]) sw[a >> 6]|= 1ull << (a & 63);
  w[a]|= 1ull << (i & 63);
 }
 void erase(int i) {
  int a= i >> 6;
  w[a]&= ~(1ull << (i & 63));
  if(!w[a]) sw[a >> 6]&= ~(1ull << (a & 63));
 }
 // i 未満で最大の要素。無ければ -1。
 int prev(int i) const {
  if(i <= 0) return -1;
  int p= i - 1, a= p >> 6, b= p & 63;
  uint64_t m= w[a] & (~0ull >> (63 - b));
  if(m) return (a << 6) + 63 - countl_zero(m);
  int sa= a >> 6, sb= a & 63;
  uint64_t sm= sw[sa] & (sb == 0 ? 0ull : (1ull << sb) - 1);
  if(sm) {
   int x= (sa << 6) + 63 - countl_zero(sm);
   return (x << 6) + 63 - countl_zero(w[x]);
  }
  for(int s= sa - 1; s >= 0; --s)
   if(sw[s]) {
    int x= (s << 6) + 63 - countl_zero(sw[s]);
    return (x << 6) + 63 - countl_zero(w[x]);
   }
  return -1;
 }
 // i 以上で最小の要素。無ければ -1。
 int next(int i) const {
  if(i >= n) return -1;
  int a= i >> 6, b= i & 63;
  uint64_t m= w[a] & (~0ull << b);
  if(m) return (a << 6) + countr_zero(m);
  int sa= a >> 6, sb= a & 63;
  uint64_t sm= sw[sa] & (sb == 63 ? 0ull : ~0ull << (sb + 1));
  if(sm) {
   int x= (sa << 6) + countr_zero(sm);
   return (x << 6) + countr_zero(w[x]);
  }
  for(int s= sa + 1; s < sc; ++s)
   if(sw[s]) {
    int x= (s << 6) + countr_zero(sw[s]);
    return (x << 6) + countr_zero(w[x]);
   }
  return -1;
 }
 int max() const {
  for(int s= sc - 1; s >= 0; --s)
   if(sw[s]) {
    int x= (s << 6) + 63 - countl_zero(sw[s]);
    return (x << 6) + 63 - countl_zero(w[x]);
   }
  return -1;
 }
};
}  // namespace
// 文字列を反転すると、2 つの接頭辞の最長共通接尾辞は 2 つの接尾辞の最長共通接頭辞になる。
// クエリ [l, r] は反転後の接尾辞の開始位置 [n - 1 - r, n - 1 - l] になる。接尾辞配列の順位の集合を持ち、
// 集合の中で隣り合う順位の LCP の最大が答え (LCP(a, c) = min(LCP(a, b), LCP(b, c)) より)。
// 集合は追加だけで最大を聞くので、RollbackMo で右側を伸ばし、左側はクエリごとに足して巻き戻す。
// 順位の前後と LCP 値の最大は、64 bit 語を 2 段にした bit set で持つ。
signed main() {
 int n= io::read_int(), m= io::read_int();
 string s;
 io::read_word(s);
 string t(s.rbegin(), s.rend());
 const vector<int> sa= suffix_array(t);
 vector<int> rnk(n);
 for(int i= 0; i < n; ++i) rnk[sa[i]]= i;
 // lcp[i] は sa[i] と sa[i + 1] の LCP。順位 a < b の LCP は lcp[a..b-1] の最小。
 const vector<int> lcp= lcp_array(t, sa);
 const int L= n - 1, lg= (int)bit_width((unsigned)L);
 vector<int> st((size_t)lg * L);
 for(int i= 0; i < L; ++i) st[i]= lcp[i];
 for(int k= 1; k < lg; ++k)
  for(int i= 0; i + (1 << k) <= L; ++i)
   st[(size_t)k * L + i]= min(st[(size_t)(k - 1) * L + i], st[(size_t)(k - 1) * L + i + (1 << (k - 1))]);
 auto rmq= [&](int l, int r) {  // lcp[l..r] の最小
  int k= (int)bit_width((unsigned)(r - l + 1)) - 1;
  return min(st[(size_t)k * L + l], st[(size_t)k * L + r - (1 << k) + 1]);
 };
 RollbackMo mo;
 for(int i= 0; i < m; ++i) {
  int l= io::read_int(), r= io::read_int();
  mo.query(n - r, n - l + 1);
 }
 BitSet present, vals;
 present.init(n), vals.init(n + 1);
 vector<int> cntv(n + 1, 0);
 auto pair_add= [&](int v) {
  if(cntv[v]++ == 0) vals.add(v);
 };
 auto pair_del= [&](int v) {
  if(--cntv[v] == 0) vals.erase(v);
 };
 struct Ent {
  int p, pred, succ, v1, v2, v3;
 };
 vector<Ent> stk;
 stk.reserve(n);
 auto add= [&](int pos) {
  const int p= rnk[pos], pred= present.prev(p), succ= present.next(p + 1);
  int v1= -1, v2= -1, v3= -1;
  if(pred >= 0) pair_add(v1= rmq(pred, p - 1));
  if(succ >= 0) pair_add(v2= rmq(p, succ - 1));
  if(pred >= 0 && succ >= 0) pair_del(v3= rmq(pred, succ - 1));
  present.add(p);
  stk.push_back({p, pred, succ, v1, v2, v3});
 };
 auto undo= [&]() {
  const Ent e= stk.back();
  stk.pop_back();
  present.erase(e.p);
  if(e.pred >= 0) pair_del(e.v1);
  if(e.succ >= 0) pair_del(e.v2);
  if(e.pred >= 0 && e.succ >= 0) pair_add(e.v3);
 };
 vector<int> ans(m);
 auto out= [&](int id) {
  const int v= vals.max();
  ans[id]= v < 0 ? 0 : v;
 };
 mo.run(add, undo, out);
 string res;
 res.reserve((size_t)m * 7);
 for(int i= 0; i < m; ++i) {
  int x= ans[i];
  char tmp[12];
  int k= 0;
  do tmp[k++]= char('0' + x % 10), x/= 10;
  while(x);
  while(k) res.push_back(tmp[--k]);
  res.push_back('\n');
 }
 fwrite(res.data(), 1, res.size(), stdout);
}
