#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
// 列の木の試作。今の Library の WeightBalancedTree (mylib/data_structure/WeightBalancedTree.hpp) の中身を写し、次の 3 つを足した
// もの (平衡二分木の書き比べ、algo-notes の notes/sequence-tree.md)。
//  - 置き場は伸びる配列 (今の Library は大きさを決めた静的な配列)。節は番号で指す。
//  - 永続 (PERSIST) のときの gc。pool_empty() は、置き場が前の gc のあとに生きていた節の 2 倍 (少なくとも 2^22 個) まで伸びたら
//    真を返す。gc(版の列) は渡した版から届く節に印を付けて置き場の前へ詰め、版の根を書き換える (版どうしで共有する節は共有した
//    まま)。渡さなかった木は使えなくなる。
//  - 永続しないときは、split で外れた節と消した葉を使い回す (今の Library は置き場を使い切るまで捨てる)。
// 区間の積と 1 点の取得は、今の Library と違い、遅延を配らずに根から下りながら合わせて求める (永続のときに節を写さない)。
// 木の作りは今の Library と同じで、値は葉にだけ持ち (葉は負の番号)、左右の大きさの比を 4 以下に保つ。併合は重い方の背を下りて
// 回転 1 回で直す。遅延の印 (作用と反転) は大きさの上位 2 bit に置く。構造体 M の決まりは NeoLibrary の SegmentTree と同じ
// (モノイドは T、ti、op、作用は E、mp、cp)。M の代わりに int などの型を渡すと値を並べるだけの列になる。可換でないモノイドを反転
// するときは逆順の積も持つ (M に using commute = void を書くと省く)。
namespace wbt_seq_internal {
template <class M, class= void> struct monoid: std::false_type {};
template <class M> struct monoid<M, std::void_t<typename M::T, decltype(&M::op), decltype(&M::ti)>>: std::true_type {};
template <class M, class= void> struct action: std::false_type {};
template <class M> struct action<M, std::void_t<typename M::T, typename M::E, decltype(&M::mp), decltype(&M::cp)>>: std::true_type {};
template <class M, class= void> struct commute: std::false_type {};
template <class M> struct commute<M, std::void_t<typename M::commute>>: std::true_type {};
template <class M, class= void> struct value_of {
 using type= M;
};
template <class M> struct value_of<M, std::void_t<typename M::T>> {
 using type= typename M::T;
};
template <class M, class= void> struct action_of {
 using type= char;
};
template <class M> struct action_of<M, std::void_t<typename M::E>> {
 using type= typename M::E;
};
}  // namespace wbt_seq_internal
template <class M, bool REV= false, bool PERSIST= true> class WbtSeq {
 using T= typename wbt_seq_internal::value_of<M>::type;
 using E= typename wbt_seq_internal::action_of<M>::type;
 static constexpr bool MO= wbt_seq_internal::monoid<M>::value, AC= wbt_seq_internal::action<M>::value;
 static constexpr bool RS= MO && REV && !wbt_seq_internal::commute<M>::value;  // 逆順の積も持つ
 static constexpr bool FL= AC || REV;  // 大きさの上位 bit に遅延の印を置く
 static constexpr unsigned LZ= 0x80000000u, TG= 0x40000000u, SZ= 0x3fffffffu;
 using S= std::conditional_t<MO, T, char>;
 using SR= std::conditional_t<RS, T, char>;
 using EA= std::conditional_t<AC, E, char>;
 struct Node {
  int ch[2];
  unsigned sz;  // 葉の数。FL なら上位 2 bit が作用 (LZ) と反転 (TG) の印 (子へ配る分で、自分の sum と rsum には当ててある)
  S sum;
  SR rsum;
  EA laz;
 };
 static inline std::vector<Node> nm= std::vector<Node>(1);  // 0 は空の木
 static inline std::vector<T> nl= std::vector<T>(1);        // 葉 -i の値は nl[i]
 static inline std::vector<int> free_nm, free_nl;
 static inline std::size_t gc_at= std::size_t(1) << 22;
 static void map1(T& v, const E& e, int len) {
  if constexpr(std::is_invocable_v<decltype(M::mp), T&, const E&, int>) M::mp(v, e, len);
  else M::mp(v, e);
 }
 static unsigned msize(int i) {
  if constexpr(FL) return nm[i].sz & SZ;
  else return nm[i].sz;
 }
 static unsigned size(int i) { return i < 0 ? 1 : msize(i); }
 static T sum(int i) { return i < 0 ? nl[-i] : T(nm[i].sum); }
 static T rsum(int i) {
  if constexpr(RS) return i < 0 ? nl[-i] : T(nm[i].rsum);
  else return sum(i);
 }
 static int new_node() {
  if constexpr(!PERSIST)
   if(!free_nm.empty()) {
    const int i= free_nm.back();
    free_nm.pop_back();
    return i;
   }
  nm.emplace_back();
  return int(nm.size() - 1);
 }
 static int copy_node(int i) {
  const Node n= nm[i];
  const int j= new_node();
  nm[j]= n;
  return j;
 }
 static int new_leaf(T v) {
  if constexpr(!PERSIST)
   if(!free_nl.empty()) {
    const int i= free_nl.back();
    free_nl.pop_back();
    nl[i]= v;
    return -i;
   }
  nl.push_back(v);
  return -int(nl.size() - 1);
 }
 static void release(int c) {
  if constexpr(!PERSIST) {
   if(c > 0) free_nm.push_back(c);
   else if(c < 0) free_nl.push_back(-c);
  }
 }
 // 子から求め直す。sz が真なら大きさも求め直す (印は消えるので、遅延を配ったあとの節だけ)。
 template <bool sz= true> static void update(int i) {
  const int l= nm[i].ch[0], r= nm[i].ch[1];
  if constexpr(sz) nm[i].sz= size(l) + size(r);
  if constexpr(MO) {
   nm[i].sum= M::op(sum(l), sum(r));
   if constexpr(RS) nm[i].rsum= M::op(rsum(r), rsum(l));
  }
 }
 static void propagate(int i, const E& x) {
  Node& t= nm[i];
  if(t.sz & LZ) M::cp(t.laz, x);
  else t.laz= x;
  t.sz|= LZ;
  if constexpr(MO) {
   map1(t.sum, x, t.sz & SZ);
   if constexpr(RS) map1(t.rsum, x, t.sz & SZ);
  }
 }
 static void toggle(int i) {
  Node& t= nm[i];
  if constexpr(RS) std::swap(t.sum, t.rsum);
  std::swap(t.ch[0], t.ch[1]), t.sz^= TG;
 }
 // 子 c へ印 f と作用 lz を配る。永続なら c を写してから当てる。
 static void push_child(int& c, unsigned f, const EA& lz) {
  if(c > 0) {
   if constexpr(PERSIST) c= copy_node(c);
   if constexpr(AC)
    if(f & LZ) propagate(c, lz);
   if constexpr(REV)
    if(f & TG) toggle(c);
  } else if constexpr(AC) {
   if(c < 0 && (f & LZ)) {
    if constexpr(PERSIST) c= new_leaf(nl[-c]);
    map1(nl[-c], lz, 1);
   }
  }
 }
 // 節 i の印を子へ配る。i の部分木の表す列は変わらないので、永続でも i 自身はその場で書き換えてよい (今の Library と同じ)。
 static void push(int i) {
  if constexpr(FL) {
   const unsigned f= nm[i].sz;
   if(!(f >> 30)) return;
   EA lz{};
   if constexpr(AC) lz= nm[i].laz;
   int l= nm[i].ch[0], r= nm[i].ch[1];
   push_child(l, f, lz), push_child(r, f, lz);
   Node& t= nm[i];
   t.ch[0]= l, t.ch[1]= r, t.sz= f & SZ;
  }
 }
 // a が重い側。R が偽なら a の右の背を下りて b をつなぎ、真なら a の左の背を下りて b をつなぐ。t はつなぎ目に使う節。
 template <bool R> static int join_(int t, int a, int b) {
  if constexpr(FL) push(a);
  if constexpr(R) b= join(b, t, nm[a].ch[0]);
  else b= join(nm[a].ch[1], t, b);
  if constexpr(PERSIST) a= copy_node(a);
  if(size(nm[a].ch[R]) * 4 >= msize(b)) return nm[a].ch[!R]= b, update(a), a;
  return nm[a].ch[!R]= nm[b].ch[R], update(a), nm[b].ch[R]= a, update(b), b;
 }
 static int join(int l, int t, int r) {
  const unsigned lsz= size(l), rsz= size(r);
  if(lsz > rsz * 4) return join_<0>(t, l, r);
  if(rsz > lsz * 4) return join_<1>(t, r, l);
  nm[t].ch[0]= l, nm[t].ch[1]= r, update(t);
  return t;
 }
 static int merge(int l, int r) { return !l ? r : !r ? l : join(l, new_node(), r); }
 // 0 < k < size(i) のとき、k の位置で割った左と右と、割れ目にあった節を返す。
 static std::array<int, 3> split_(int i, unsigned k) {
  if constexpr(FL) push(i);
  const int l= nm[i].ch[0], r= nm[i].ch[1];
  const unsigned lsz= size(l);
  if(k == lsz) return {l, i, r};
  if constexpr(PERSIST) i= new_node();
  if(k < lsz) {
   const auto [a, b, c]= split_(l, k);
   return {a, b, join(c, i, r)};
  }
  const auto [a, b, c]= split_(r, k - lsz);
  return {join(l, i, a), b, c};
 }
 static std::pair<int, int> split(int i, unsigned k) {
  if(k == 0) return {0, i};
  if(k >= size(i)) return {i, 0};
  const auto [l, c, r]= split_(i, k);
  release(c);
  return {l, r};
 }
 // i の部分木の列の順の [l, r) の積を、遅延を配らずに求める。fl は祖先から受けた反転の印。
 static T prod_(int i, unsigned l, unsigned r, bool fl) {
  if(i < 0) return nl[-i];
  const unsigned f= nm[i].sz, s= f & SZ;
  if(l == 0 && r == s) return fl ? rsum(i) : sum(i);
  const int a= nm[i].ch[fl], b= nm[i].ch[!fl];
  const bool cf= fl ^ bool(f & TG);
  const unsigned asz= size(a);
  T res;
  if(r <= asz) res= prod_(a, l, r, cf);
  else if(asz <= l) res= prod_(b, l - asz, r - asz, cf);
  else res= M::op(prod_(a, l, asz, cf), prod_(b, 0, r - asz, cf));
  if constexpr(AC)
   if(f & LZ) map1(res, nm[i].laz, r - l);
  return res;
 }
 static T get_(int i, unsigned k) {
  static std::vector<int> ps;
  ps.clear();
  bool fl= false;
  while(i > 0) {
   ps.push_back(i);
   const unsigned f= nm[i].sz;
   const int a= nm[i].ch[fl], b= nm[i].ch[!fl];
   fl^= bool(f & TG);
   const unsigned asz= size(a);
   if(k < asz) i= a;
   else k-= asz, i= b;
  }
  T v= nl[-i];
  if constexpr(AC)
   for(int j= (int)ps.size(); j--;)
    if(nm[ps[j]].sz & LZ) map1(v, nm[ps[j]].laz, 1);
  return v;
 }
 static int apply_(int i, unsigned l, unsigned r, const E& x) {
  if(i < 0) {
   if constexpr(PERSIST) i= new_leaf(nl[-i]);
   map1(nl[-i], x, 1);
   return i;
  }
  if constexpr(PERSIST) i= copy_node(i);
  if(l == 0 && msize(i) <= r) return propagate(i, x), i;
  push(i);
  int a= nm[i].ch[0], b= nm[i].ch[1];
  const unsigned asz= size(a);
  if(r <= asz) a= apply_(a, l, r, x);
  else if(asz <= l) b= apply_(b, l - asz, r - asz, x);
  else a= apply_(a, l, asz, x), b= apply_(b, 0, r - asz, x);
  nm[i].ch[0]= a, nm[i].ch[1]= b;
  if constexpr(MO) update<false>(i);
  return i;
 }
 static int set_(int i, unsigned k, const T& v) {
  if(i < 0) {
   if constexpr(PERSIST) return new_leaf(v);
   nl[-i]= v;
   return i;
  }
  push(i);
  if constexpr(PERSIST) i= copy_node(i);
  int a= nm[i].ch[0], b= nm[i].ch[1];
  const unsigned asz= size(a);
  if(k < asz) a= set_(a, k, v);
  else b= set_(b, k - asz, v);
  nm[i].ch[0]= a, nm[i].ch[1]= b;
  if constexpr(MO) update<false>(i);
  return i;
 }
 template <class F> static int build(const F& at, int lo, int hi) {
  if(hi - lo == 1) return new_leaf(at(lo));
  const int m= (lo + hi) / 2;
  const int a= build(at, lo, m), b= build(at, m, hi);
  const int i= new_node();
  nm[i].ch[0]= a, nm[i].ch[1]= b, update(i);
  return i;
 }
 static void dump_(int i, bool fl, std::vector<T>& out, std::vector<E>& acts) {
  if(i < 0) {
   T v= nl[-i];
   if constexpr(AC)
    for(int j= (int)acts.size(); j--;) map1(v, acts[j], 1);
   out.push_back(v);
   return;
  }
  const unsigned f= nm[i].sz;
  const int a= nm[i].ch[fl], b= nm[i].ch[!fl];
  const bool cf= fl ^ bool(f & TG);
  if constexpr(AC)
   if(f & LZ) acts.push_back(nm[i].laz);
  dump_(a, cf, out, acts), dump_(b, cf, out, acts);
  if constexpr(AC)
   if(f & LZ) acts.pop_back();
 }
 // 渡した根から届く節に印を付け、置き場の前へ詰める。根の番号は書き換える。
 static void gc_roots(const std::vector<int*>& roots) {
  std::vector<char> mi(nm.size()), ml(nl.size());
  std::vector<int> st;
  for(int* r: roots) st.push_back(*r);
  while(!st.empty()) {
   const int c= st.back();
   st.pop_back();
   if(c > 0) {
    if(mi[c]) continue;
    mi[c]= 1, st.push_back(nm[c].ch[0]), st.push_back(nm[c].ch[1]);
   } else if(c < 0) ml[-c]= 1;
  }
  std::vector<int> ni(nm.size()), nj(nl.size());
  int k= 1, kl= 1;
  for(std::size_t i= 1; i < nm.size(); ++i)
   if(mi[i]) ni[i]= k++;
  for(std::size_t i= 1; i < nl.size(); ++i)
   if(ml[i]) nj[i]= kl++;
  auto remap= [&](int c) { return c > 0 ? ni[c] : c < 0 ? -nj[-c] : 0; };
  for(std::size_t i= 1; i < nm.size(); ++i)
   if(mi[i]) {
    Node n= nm[i];
    n.ch[0]= remap(n.ch[0]), n.ch[1]= remap(n.ch[1]);
    nm[ni[i]]= n;
   }
  for(std::size_t i= 1; i < nl.size(); ++i)
   if(ml[i]) nl[nj[i]]= nl[i];
  nm.resize(k), nl.resize(kl);
  free_nm.clear(), free_nl.clear();
  for(int* r: roots) *r= remap(*r);
  gc_at= std::max<std::size_t>(std::size_t(1) << 22, 2 * (nm.size() + nl.size()));
 }
 int root= 0;
 static WbtSeq of(int rt) {
  WbtSeq s;
  s.root= rt;
  return s;
 }
public:
 WbtSeq()= default;
 explicit WbtSeq(int n, const T& v= T()) {
  if(n) root= build([&](int) { return v; }, 0, n);
 }
 WbtSeq(const std::vector<T>& v) {
  if(!v.empty()) root= build([&](int i) { return v[i]; }, 0, (int)v.size());
 }
 template <class It, std::enable_if_t<!std::is_integral_v<It>, int> = 0> WbtSeq(It bg, It ed) {
  const std::vector<T> v(bg, ed);
  if(!v.empty()) root= build([&](int i) { return v[i]; }, 0, (int)v.size());
 }
 int size() const { return root ? (int)size(root) : 0; }
 bool empty() const { return !root; }
 void clear() { root= 0; }
 std::pair<WbtSeq, WbtSeq> split(int k) const {
  const auto [l, r]= split(root, k);
  return {of(l), of(r)};
 }
 std::tuple<WbtSeq, WbtSeq, WbtSeq> split3(int a, int b) const {
  const auto [tmp, r]= split(root, b);
  const auto [l, c]= split(tmp, a);
  return {of(l), of(c), of(r)};
 }
 WbtSeq& operator+=(const WbtSeq& rhs) { return root= merge(root, rhs.root), *this; }
 WbtSeq operator+(const WbtSeq& rhs) const { return of(merge(root, rhs.root)); }
 void insert(int k, const T& v) {
  const auto [l, r]= split(root, k);
  root= merge(merge(l, new_leaf(v)), r);
 }
 void push_back(const T& v) { root= merge(root, new_leaf(v)); }
 void push_front(const T& v) { root= merge(new_leaf(v), root); }
 T erase(int k) {
  const auto [l, tmp]= split(root, k);
  const auto [t, r]= split(tmp, 1);
  const T v= nl[-t];
  release(t);
  root= merge(l, r);
  return v;
 }
 T pop_back() { return erase(size() - 1); }
 T pop_front() { return erase(0); }
 T get(int k) const { return get_(root, k); }
 T operator[](int k) const { return get_(root, k); }
 void set(int k, const T& v) { root= set_(root, k, v); }
 void mul(int k, const T& v) {
  static_assert(MO, "mul はモノイドがあるときだけ");
  set(k, M::op(get(k), v));
 }
 T prod(int l, int r) const {
  static_assert(MO, "prod はモノイドがあるときだけ");
  if(l >= r) return M::ti();
  return prod_(root, l, r, false);
 }
 T all_prod() const {
  static_assert(MO, "all_prod はモノイドがあるときだけ");
  return root ? sum(root) : M::ti();
 }
 void apply(int l, int r, const E& x) {
  static_assert(AC, "apply は作用があるときだけ");
  if(l < r) root= apply_(root, l, r, x);
 }
 void reverse() {
  static_assert(REV, "reverse は REV のときだけ");
  if(root > 0) {
   if constexpr(PERSIST) root= copy_node(root);
   toggle(root);
  }
 }
 void reverse(int a, int b) {
  static_assert(REV, "reverse は REV のときだけ");
  if(b - a < 2) return;
  const auto [tmp, r]= split(root, b);
  auto [l, c]= split(tmp, a);
  if constexpr(PERSIST) c= copy_node(c);
  toggle(c);
  root= merge(merge(l, c), r);
 }
 std::vector<T> dump() const {
  std::vector<T> out;
  std::vector<E> acts;
  out.reserve(size());
  if(root) dump_(root, false, out, acts);
  return out;
 }
 static bool pool_empty() { return PERSIST && nm.size() + nl.size() >= gc_at; }
 static void gc(std::vector<WbtSeq>& vs) {
  std::vector<int*> rs;
  rs.reserve(vs.size());
  for(auto& v: vs) rs.push_back(&v.root);
  gc_roots(rs);
 }
 template <class... Ps> static void gc(WbtSeq& v, Ps&... vs) {
  std::vector<int*> rs{&v.root, &vs.root...};
  gc_roots(rs);
 }
 // 置き場を n 個ぶん先に取る (伸びる配列を作り直して写す手間を省く)。
 static void reserve(std::size_t n) { nm.reserve(n), nl.reserve(n); }
 // 試し用。置き場の節の数、gc を始める大きさ。
 static std::size_t pool_nodes() { return nm.size() + nl.size() - free_nm.size() - free_nl.size(); }
 static void set_gc_at(std::size_t n) { gc_at= n; }
};
