// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <iostream>
#include <set>
#include <iterator>
#include <limits>
#include <cassert>
template <class Int, bool merge= true> class RangeSet {
 struct ClosedSection {
  Int l, r;
  Int length() const { return r - l + 1; }
  bool operator<(const ClosedSection &cs) const { return l < cs.l || (l == cs.l && r > cs.r); }
  operator bool() const { return l <= r; }
  friend std::ostream &operator<<(std::ostream &os, const ClosedSection &cs) { return cs ? os << "[" << cs.l << "," << cs.r << "]" : os << "∅"; }
 };
 std::set<ClosedSection> mp;
public:
 RangeSet() {
  constexpr Int INF= std::numeric_limits<Int>::max() / 2;
  mp.insert({INF, INF}), mp.insert({-INF, -INF});
 }
 ClosedSection covered_by(Int l, Int r) const {
  assert(l <= r);
  if (auto it= std::prev(mp.upper_bound(ClosedSection{l, l})); it->l <= l && r <= it->r) return *it;
  return {1, 0};
 }
 ClosedSection covered_by(Int x) const { return covered_by(x, x); }
 ClosedSection covered_by(const ClosedSection &cs) const { return covered_by(cs.l, cs.r); }
 size_t size() const { return mp.size() - 2; }
 auto begin() const { return std::next(mp.begin()); }
 auto end() const { return std::prev(mp.end()); }
 Int insert(Int l, Int r) {
  assert(l <= r);
  auto it= std::prev(mp.upper_bound(ClosedSection{l, l}));
  Int sum= 0, x= it->l, y= it->r;
  if (x <= l && r <= y) return sum;
  if (x <= l && l <= y + merge) sum+= y - (l= x) + 1, it= mp.erase(it);
  else std::advance(it, 1);
  for (; it->r < r; it= mp.erase(it)) sum+= it->r - it->l + 1;
  if (x= it->l, y= it->r; x - merge <= r && r <= y) sum+= (r= y) - x + 1, mp.erase(it);
  return mp.insert({l, r}), r - l + 1 - sum;
 }
 Int insert(Int x) { return insert(x, x); }
 Int insert(const ClosedSection &cs) { return insert(cs.l, cs.r); }
 Int erase(Int l, Int r) {
  assert(l <= r);
  auto it= std::prev(mp.upper_bound(ClosedSection{l, l}));
  Int sum= 0, x= it->l, y= it->r;
  if (x <= l && r <= y) {
   if (mp.erase(it); x < l) mp.insert({x, l - 1});
   if (r < y) mp.insert({r + 1, y});
   return r - l + 1;
  }
  if (x <= l && l <= y) {
   if (x < l) mp.insert({x, l - 1});
   sum+= y - l + 1, it= mp.erase(it);
  } else std::advance(it, 1);
  for (; it->r <= r; it= mp.erase(it)) sum+= it->r - it->l + 1;
  if (x= it->l, y= it->r; x <= r && r <= y)
   if (sum+= r - x + 1, mp.erase(it); r < y) mp.insert({r + 1, y});
  return sum;
 }
 Int erase(Int x) { return erase(x, x); }
 Int erase(const ClosedSection &cs) { return erase(cs.l, cs.r); }
 Int mex(Int x) const {
  auto cs= covered_by(x);
  return cs ? cs.r + 1 : x;
 }
 friend std::ostream &operator<<(std::ostream &os, const RangeSet &rs) {
  os << "[";
  for (auto it= rs.begin(); it != rs.end(); ++it) os << (it == rs.begin() ? "" : ",") << *it;
  return os << "]";
 }
};
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int H, W, N, M;
 cin >> H >> W >> N >> M;
 int A[N], B[N], C[M], D[M];
 for(int i= 0; i < N; ++i) cin >> A[i] >> B[i], --A[i], --B[i];
 for(int i= 0; i < M; ++i) cin >> C[i] >> D[i], --C[i], --D[i];
 bool g[H][W];
 for(int i= H; i--;) fill_n(g[i], W, 0);
 {
  RangeSet<int> rs[H], res[H];
  for(int i= H; i--;) rs[i].insert(0, W - 1);
  for(int i= M; i--;) rs[C[i]].erase(D[i]);
  for(int i= N; i--;) res[A[i]].insert(rs[A[i]].covered_by(B[i]));
  for(int i= H; i--;)
   for(int j= W; j--;) g[i][j]|= res[i].covered_by(j);
 }
 {
  RangeSet<int> rs[W], res[W];
  for(int i= W; i--;) rs[i].insert(0, H - 1);
  for(int i= M; i--;) rs[D[i]].erase(C[i]);
  for(int i= N; i--;) res[B[i]].insert(rs[B[i]].covered_by(A[i]));
  for(int j= W; j--;)
   for(int i= H; i--;) g[i][j]|= res[j].covered_by(i);
 }
 int ans= 0;
 for(int i= H; i--;)
  for(int j= W; j--;) ans+= g[i][j];
 cout << ans << '\n';
 return 0;
}
