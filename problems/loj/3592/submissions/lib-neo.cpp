#include <algorithm>
#include <bit>
#include <iostream>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
using namespace std;
// クエリの区間を重ならず連続した値のブロックに分けるのが最適で、ブロックの長さを昇順に
// c1 <= ... <= cK とすると、c1, c3, c5, ... と c..., c6, c4, c2 の順 (山の形) が最適。
// このとき総多様性は、各ブロックの前後の長さ P, len-P-c について
//   (1/2) * (K len (len+1) - len (K-1) - Σ(P^2 + (len-P-c)^2))
// になる。頻度の分布 comp[c] (頻度 c の値の個数) を Mo で保ち、クエリごとに
// comp の非零の c を昇順に舐めてこの式を組む。非零の c は O(sqrt(N)) 個しかない
// (Σ c * comp[c] = len だから)。
// 頻度の値の集合は std::set だと 1 手ごとの確保と木のたどりが重いので、
// 64 bit 語を 2 段にした bit set にする。
namespace {
// 頻度の値 1..n の集合。追加・削除・「i 以上の最小」を O(1) 程度で返す。
struct Presence {
 int n= 0, wc= 0, sc= 0;
 vector<unsigned long long> w, sw;
 void init(int n_) {
  n= n_;
  wc= (n >> 6) + 1;
  sc= (wc >> 6) + 1;
  w.assign(wc, 0), sw.assign(sc, 0);
 }
 void add(int i) {
  const int a= i >> 6;
  if(w[a] == 0) sw[a >> 6]|= 1ull << (a & 63);
  w[a]|= 1ull << (i & 63);
 }
 void erase(int i) {
  const int a= i >> 6;
  w[a]&= ~(1ull << (i & 63));
  if(w[a] == 0) sw[a >> 6]&= ~(1ull << (a & 63));
 }
 // i 以上の最小の要素。無ければ -1。
 int next(int i) const {
  if(i > n) return -1;
  const int a= i >> 6;
  unsigned long long m= w[a] & (~0ull << (i & 63));
  if(m) return (a << 6) + countr_zero(m);
  const int sa= a >> 6;
  m= sw[sa] & ((a & 63) == 63 ? 0ull : (~0ull << ((a & 63) + 1)));
  if(m) {
   const int x= (sa << 6) + countr_zero(m);
   return (x << 6) + countr_zero(w[x]);
  }
  for(int s= sa + 1; s < sc; ++s)
   if(sw[s]) {
    const int x= (s << 6) + countr_zero(sw[s]);
    return (x << 6) + countr_zero(w[x]);
   }
  return -1;
 }
};
// 値 v のブロックを m 個、前置きの長さ P から並べたときの
// Σ_{j=0}^{m-1} [(P + j v)^2 + (len - P - (j+1) v)^2]。
long long block_contrib(long long P, long long v, long long m, long long len) {
 if(m == 0) return 0;
 const long long s1= m * (m - 1) / 2;
 const long long s2= m * (m - 1) * (2 * m - 1) / 6;
 const long long Q= len - P;
 return m * P * P + 2 * P * v * s1 + v * v * s2 + m * Q * Q - 2 * Q * v * (s1 + m) + v * v * (s2 + 2 * s1 + m);
}
}  // namespace
int main() {
 cin.tie(0), ios::sync_with_stdio(0);
 int N, Q;
 cin >> N >> Q;
 vector<int> a(N);
 for(auto& x: a) cin >> x;
 int mu= 0;
 {
  vector<int> v(a);
  sort(v.begin(), v.end());
  v.erase(unique(v.begin(), v.end()), v.end());
  mu= (int)v.size();
  for(auto& x: a) x= (int)(lower_bound(v.begin(), v.end(), x) - v.begin());
 }
 Mo mo;
 for(int i= 0; i < Q; ++i) {
  int l, r;
  cin >> l >> r, --l;
  mo.query(l, r);
 }
 vector<int> cnt(mu, 0), comp(N + 1, 0);
 Presence present;
 present.init(N);
 int cur_len= 0;
 auto add= [&](int i) {
  const int x= a[i], c= cnt[x];
  if(c == 0) {
   if(comp[1]++ == 0) present.add(1);
   cnt[x]= 1;
  } else {
   if(--comp[c] == 0) present.erase(c);
   if(comp[c + 1]++ == 0) present.add(c + 1);
   cnt[x]= c + 1;
  }
  ++cur_len;
 };
 auto erase= [&](int i) {
  const int x= a[i], c= cnt[x];
  if(c == 1) {
   if(--comp[1] == 0) present.erase(1);
   cnt[x]= 0;
  } else {
   if(--comp[c] == 0) present.erase(c);
   if(comp[c - 1]++ == 0) present.add(c - 1);
   cnt[x]= c - 1;
  }
  --cur_len;
 };
 vector<long long> ans(Q);
 vector<int> gc, gm1;
 gc.reserve(1024), gm1.reserve(1024);
 auto out= [&](int id) {
  const int len= cur_len;
  if(len <= 0) {
   ans[id]= 0;
   return;
  }
  gc.clear(), gm1.clear();
  int r0= 0;
  for(int c= present.next(1); c >= 0; c= present.next(c + 1)) {
   const int m= comp[c];
   const int m1= ((r0 + m + 1) >> 1) - ((r0 + 1) >> 1);
   gc.push_back(c), gm1.push_back(m1);
   r0+= m;
  }
  const int K= r0;
  long long F= 0, P= 0;
  for(size_t g= 0; g < gc.size(); ++g) {
   F+= block_contrib(P, gc[g], gm1[g], len);
   P+= (long long)gm1[g] * gc[g];
  }
  for(size_t g= gc.size(); g-- > 0;) {
   const int m2= comp[gc[g]] - gm1[g];
   F+= block_contrib(P, gc[g], m2, len);
   P+= (long long)m2 * gc[g];
  }
  ans[id]= ((long long)K * len * (len + 1) - (long long)len * (K - 1) - F) / 2;
 };
 mo.run(add, erase, out);
 for(int i= 0; i < Q; ++i) cout << ans[i] << '\n';
}
