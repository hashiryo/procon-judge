// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
#include <map>
#include <vector>
#include <algorithm>
#include <array>
#include <cassert>
template <class T> class WaveletMatrix {
 struct SuccinctIndexableDictionary {
  int len, blocks, zeros= 0;
  std::vector<unsigned> bit, sum;
  SuccinctIndexableDictionary(int len): len(len), blocks((len >> 5) + 1), bit(blocks, 0), sum(blocks, 0) {}
  void set(int k) { bit[k >> 5]|= 1U << (k & 31); }
  void build() {
   for (int i= 1; i < blocks; ++i) sum[i]= sum[i - 1] + __builtin_popcount(bit[i - 1]);
   zeros= rank0(len);
  }
  bool operator[](int k) const { return (bit[k >> 5] >> (k & 31)) & 1; }
  int rank(int k) const { return (sum[k >> 5] + __builtin_popcount(bit[k >> 5] & ((1U << (k & 31)) - 1))); }
  int rank0(int k) const { return k - rank(k); }
 };
 int len, lg;
 std::vector<SuccinctIndexableDictionary> mat;
 std::vector<T> vec;
public:
 WaveletMatrix(const std::vector<T> &v): len(v.size()), lg(len ? 32 - __builtin_clz(len) : 1), mat(lg, SuccinctIndexableDictionary(len)), vec(v) {
  std::sort(vec.begin(), vec.end()), vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
  std::vector<unsigned> cur(len), nex(len);
  for (int i= len; i--;) cur[i]= std::lower_bound(vec.begin(), vec.end(), v[i]) - vec.begin();
  for (auto h= lg; h--; cur.swap(nex)) {
   for (int i= 0; i < len; ++i)
    if ((cur[i] >> h) & 1) mat[h].set(i);
   mat[h].build();
   std::array it{nex.begin(), nex.begin() + mat[h].zeros};
   for (int i= 0; i < len; ++i) *it[mat[h][i]]++= cur[i];
  }
 }
 // k-th(0-indexed) smallest number in v[l,r)
 T kth_smallest(int l, int r, int k) const {
  assert(k < r - l);
  int ret= 0;
  for (auto h= lg; h--;)
   if (auto l0= mat[h].rank0(l), r0= mat[h].rank0(r); k >= r0 - l0) k-= r0 - l0, ret|= 1 << h, l+= mat[h].zeros - l0, r+= mat[h].zeros - r0;
   else l= l0, r= r0;
  return vec[ret];
 }
 // k-th(0-indexed) largest number in v[l,r)
 T kth_largest(int l, int r, int k) const { return kth_smallest(l, r, r - l - k - 1); }
 // count i s.t. (l <= i < r) && (v[i] < ub)
 int count(int l, int r, T ub) const {
  unsigned x= std::lower_bound(vec.begin(), vec.end(), ub) - vec.begin();
  if (x >= 1u << lg) return r - l;
  if (x == 0) return 0;
  int ret= 0;
  for (auto h= lg; h--;)
   if (auto l0= mat[h].rank0(l), r0= mat[h].rank0(r); (x >> h) & 1) ret+= r0 - l0, l+= mat[h].zeros - l0, r+= mat[h].zeros - r0;
   else l= l0, r= r0;
  return ret;
 }
 // count i s.t. (l <= i < r) && (lb <= v[i] < ub)
 int count(int l, int r, T lb, T ub) const { return count(l, r, ub) - count(l, r, lb); }
};
class RangeCountDistinct {
 WaveletMatrix<int> wm;
 template <class T> std::vector<int> build(const std::vector<T>& v) {
  std::vector<int> next(v.size(), -1);
  std::map<T, int> mp;
  for(int i= v.size(); i--; mp[v[i]]= i)
   if(mp.count(v[i])) next[mp[v[i]]]= i;
  return next;
 }
public:
 template <class T> RangeCountDistinct(const std::vector<T>& v): wm(build(v)) {}
 size_t count(int l, int r) const { return wm.count(l, r, l); }
};

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, Q;
 cin >> N >> Q;
 vector<int> c(N);
 for(int i= 0; i < N; i++) cin >> c[i];
 RangeCountDistinct dq(c);
 while(Q--) {
  int l, r;
  cin >> l >> r, l--;
  cout << dq.count(l, r) << '\n';
 }
 return 0;
}
