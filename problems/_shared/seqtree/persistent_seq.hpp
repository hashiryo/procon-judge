#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
// 永続の列の木の試作 (平衡二分木の書き比べ、algo-notes の notes/sequence-tree.md)。値を葉にだけ持つ重み平衡木で、どの操作も
// 道の上の節を写して新しい根を作るので、木の値 (根の番号) を写せば版の写しになる。構造体 M の決まりは NeoLibrary の SegmentTree
// と同じ (モノイドは T、ti、op、作用は E、mp、cp)。M の代わりに int などの型を渡すと値を並べるだけの列になる。反転は REV で選び、
// 可換でないモノイドを反転するときは逆順の積も持つ (M に using commute = void を書くと省く)。
// - 重みは葉の数で、左右の重みの比を 3 以下に保つ。併合は、重い方の背を下りて 1 回か 2 回の回転で直す (Blelloch らの join を
//   葉に値を持つ木に読み替えたもの。どの節も値を持つ木の空の部分木が、ここでの葉に当たる)。
// - 遅延 (反転の印と作用) は、子へ配るときに子を写してから当てる。区間の積と 1 点の取得は、節を写さずに、根から下りながら
//   遅延を合わせて求める。
// - 置き場は同じ型の木で共有する。pool_empty() は、前の gc のあとに生きていた節の 2 倍 (少なくとも 2^16 個) まで置き場が
//   伸びたら真を返す。gc(版の列) は、渡した版から届く節に印を付けて置き場の前へ詰め、版の根を書き換える。版どうしで共有する
//   節は共有したまま残る。渡さなかった木は使えなくなる。
namespace persistent_seq_internal {
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
}  // namespace persistent_seq_internal
template <class M, bool REV= false> class PersistentSeq {
 using T= typename persistent_seq_internal::value_of<M>::type;
 using E= typename persistent_seq_internal::action_of<M>::type;
 static constexpr bool MO= persistent_seq_internal::monoid<M>::value, AC= persistent_seq_internal::action<M>::value;
 static constexpr bool RS= MO && REV && !persistent_seq_internal::commute<M>::value;  // 逆順の積も持つ
 using S= std::conditional_t<MO, T, char>;
 using SR= std::conditional_t<RS, T, char>;
 using EA= std::conditional_t<AC, E, char>;
 // 子の番号は、正なら内側の節 nd[c]、負なら葉 lv[-c]、0 なら空。
 struct Node {
  int l, r, sz;  // sz は葉の数
  bool rev, lz;  // 子へ配る反転の印と作用 (自分の sum と rsum には当ててある)
  S sum;
  SR rsum;
  EA e;
 };
 static inline std::vector<Node> nd= std::vector<Node>(1);
 static inline std::vector<T> lv= std::vector<T>(1);
 static inline std::size_t gc_at= std::size_t(1) << 16;
 static void map1(T& v, const E& e, int len) {
  if constexpr(std::is_invocable_v<decltype(M::mp), T&, const E&, int>) M::mp(v, e, len);
  else M::mp(v, e);
 }
 static int size(int c) { return c > 0 ? nd[c].sz : c < 0; }
 static T sumv(int c) { return c > 0 ? T(nd[c].sum) : lv[-c]; }
 static T rsumv(int c) {
  if constexpr(RS) return c > 0 ? T(nd[c].rsum) : lv[-c];
  else return sumv(c);
 }
 static int leaf(const T& v) {
  lv.push_back(v);
  return -int(lv.size() - 1);
 }
 // 子 a と b を持つ新しい節。
 static int node(int a, int b) {
  Node n;
  n.l= a, n.r= b, n.sz= size(a) + size(b), n.rev= n.lz= false;
  if constexpr(MO) n.sum= M::op(sumv(a), sumv(b));
  if constexpr(RS) n.rsum= M::op(rsumv(b), rsumv(a));
  nd.push_back(n);
  return int(nd.size() - 1);
 }
 // 節 y (写したばかりで、ほかから指されていない) に反転の印と作用を当てる。
 static void tog(int y) {
  Node& n= nd[y];
  std::swap(n.l, n.r);
  if constexpr(RS) std::swap(n.sum, n.rsum);
  n.rev^= 1;
 }
 static void aff(int y, const E& e) {
  Node& n= nd[y];
  if constexpr(MO) {
   map1(n.sum, e, n.sz);
   if constexpr(RS) map1(n.rsum, e, n.sz);
  }
  if(n.lz) M::cp(n.e, e);
  else n.e= e, n.lz= true;
 }
 // 子 c に反転 (rv) と作用 (lz なら e) を当てたものを返す。c は写して当てる (c 自身は書き換えない)。
 static int push_to(int c, bool rv, bool lz, const EA& e) {
  if(c < 0) {
   if constexpr(AC)
    if(lz) {
     T v= lv[-c];
     map1(v, e, 1);
     return leaf(v);
    }
   return c;
  }
  if(!rv && !lz) return c;
  const Node copy= nd[c];
  nd.push_back(copy);
  const int y= int(nd.size() - 1);
  if(rv) tog(y);
  if constexpr(AC)
   if(lz) aff(y, e);
  return y;
 }
 // 内側の節 c の子を、c の遅延を当てた形で返す (c は書き換えない)。
 static std::pair<int, int> kids(int c) {
  const Node n= nd[c];
  if(!n.rev && !n.lz) return {n.l, n.r};
  const int a= push_to(n.l, n.rev, n.lz, n.e);
  const int b= push_to(n.r, n.rev, n.lz, n.e);
  return {a, b};
 }
 static bool bal(int wa, int wb) { return 3LL * wa >= wb && 3LL * wb >= wa; }
 // a の重さが b の 3 倍を超えるとき。a の右の背を下りて b をつなぐ。
 static int join_right(int a, int b) {
  const auto [x, y]= kids(a);
  const int z= merge(y, b);
  if(bal(size(x), size(z))) return node(x, z);
  const auto [z1, z2]= kids(z);
  if(bal(size(x), size(z1)) && bal(size(x) + size(z1), size(z2))) return node(node(x, z1), z2);  // 1 回の回転
  assert(z1 > 0);
  const auto [u1, u2]= kids(z1);
  return node(node(x, u1), node(u2, z2));  // 2 回の回転
 }
 static int join_left(int a, int b) {
  const auto [x, y]= kids(b);
  const int z= merge(a, x);
  if(bal(size(z), size(y))) return node(z, y);
  const auto [z1, z2]= kids(z);
  if(bal(size(z2), size(y)) && bal(size(z1), size(z2) + size(y))) return node(z1, node(z2, y));
  assert(z2 > 0);
  const auto [u1, u2]= kids(z2);
  return node(node(z1, u1), node(u2, y));
 }
 static int merge(int a, int b) {
  if(!a || !b) return a | b;
  const int wa= size(a), wb= size(b);
  if(wa > 3 * wb) return join_right(a, b);
  if(wb > 3 * wa) return join_left(a, b);
  return node(a, b);
 }
 // 先頭の k 個を a に、残りを b に分ける。
 static void split(int c, int k, int& a, int& b) {
  if(k <= 0) return void((a= 0, b= c));
  if(k >= size(c)) return void((a= c, b= 0));
  const auto [x, y]= kids(c);
  const int wx= size(x);
  if(k == wx) return void((a= x, b= y));
  int p, q;
  if(k < wx) split(x, k, p, q), a= p, b= merge(q, y);
  else split(y, k - wx, p, q), a= merge(x, p), b= q;
 }
 // c の列の順の [l, r) の積を、節を写さずに求める。fl は祖先から受けた反転の印 (c の部分木を逆に読む)。
 static T prod(int c, int l, int r, bool fl) {
  if(c < 0) return lv[-c];
  const Node& n= nd[c];
  if(l == 0 && r == n.sz) return fl ? rsumv(c) : sumv(c);
  const int a= fl ? n.r : n.l, b= fl ? n.l : n.r;
  const bool cf= fl ^ n.rev;
  const int wa= size(a);
  T s;
  if(r <= wa) s= prod(a, l, r, cf);
  else if(l >= wa) s= prod(b, l - wa, r - wa, cf);
  else s= M::op(prod(a, l, wa, cf), prod(b, 0, r - wa, cf));
  if constexpr(AC)
   if(n.lz) map1(s, n.e, r - l);
  return s;
 }
 static T get(int c, int k) {
  // 根から葉まで下り、葉の値に下の節の作用から順に当てる (下の節の作用ほど古い)。
  static std::vector<int> ps;
  ps.clear();
  bool fl= false;
  while(c > 0) {
   const Node& n= nd[c];
   ps.push_back(c);
   const int a= fl ? n.r : n.l, b= fl ? n.l : n.r, wa= size(a);
   fl^= n.rev;
   if(k < wa) c= a;
   else k-= wa, c= b;
  }
  T v= lv[-c];
  if constexpr(AC)
   for(int i= (int)ps.size(); i--;)
    if(nd[ps[i]].lz) map1(v, nd[ps[i]].e, 1);
  return v;
 }
 static int apply(int c, int l, int r, const E& e) {
  if(l >= r) return c;
  if(l == 0 && r == size(c)) return push_to(c, false, true, e);
  const auto [x, y]= kids(c);
  const int wx= size(x);
  const int x2= l < wx ? apply(x, l, std::min(r, wx), e) : x;
  const int y2= r > wx ? apply(y, std::max(l - wx, 0), r - wx, e) : y;
  return node(x2, y2);
 }
 static int set(int c, int k, const T& v) {
  if(c < 0) return leaf(v);
  const auto [x, y]= kids(c);
  const int wx= size(x);
  if(k < wx) return node(set(x, k, v), y);
  return node(x, set(y, k - wx, v));
 }
 static void dump(int c, bool fl, std::vector<T>& out, std::vector<E>& acts) {
  if(c < 0) {
   T v= lv[-c];
   if constexpr(AC)
    for(int i= (int)acts.size(); i--;) map1(v, acts[i], 1);
   out.push_back(v);
   return;
  }
  const Node& n= nd[c];
  const int a= fl ? n.r : n.l, b= fl ? n.l : n.r;
  const bool cf= fl ^ n.rev, lz= n.lz;
  if constexpr(AC)
   if(lz) acts.push_back(n.e);
  dump(a, cf, out, acts), dump(b, cf, out, acts);
  if constexpr(AC)
   if(lz) acts.pop_back();
 }
 template <class F> static int build(const F& at, int lo, int hi) {
  if(hi - lo == 1) return leaf(at(lo));
  const int m= (lo + hi) / 2;
  const int a= build(at, lo, m), b= build(at, m, hi);
  return node(a, b);
 }
 // 渡した根から届く節に印を付け、置き場の前へ詰める。根の番号は書き換える。
 static void gc_roots(const std::vector<int*>& roots) {
  std::vector<char> mi(nd.size()), ml(lv.size());
  std::vector<int> st;
  for(int* r: roots) st.push_back(*r);
  while(!st.empty()) {
   const int c= st.back();
   st.pop_back();
   if(c > 0) {
    if(mi[c]) continue;
    mi[c]= 1, st.push_back(nd[c].l), st.push_back(nd[c].r);
   } else if(c < 0) ml[-c]= 1;
  }
  std::vector<int> ni(nd.size()), nl(lv.size());
  int k= 1, kl= 1;
  for(std::size_t i= 1; i < nd.size(); ++i)
   if(mi[i]) ni[i]= k++;
  for(std::size_t i= 1; i < lv.size(); ++i)
   if(ml[i]) nl[i]= kl++;
  auto remap= [&](int c) { return c > 0 ? ni[c] : c < 0 ? -nl[-c] : 0; };
  for(std::size_t i= 1; i < nd.size(); ++i)
   if(mi[i]) {
    Node n= nd[i];
    n.l= remap(n.l), n.r= remap(n.r);
    nd[ni[i]]= n;
   }
  for(std::size_t i= 1; i < lv.size(); ++i)
   if(ml[i]) lv[nl[i]]= lv[i];
  nd.resize(k), lv.resize(kl);
  for(int* r: roots) *r= remap(*r);
  gc_at= std::max<std::size_t>(std::size_t(1) << 16, 2 * (nd.size() + lv.size()));
 }
 int root= 0;
 static PersistentSeq of(int rt) {
  PersistentSeq s;
  s.root= rt;
  return s;
 }
public:
 PersistentSeq()= default;
 explicit PersistentSeq(int n, const T& v= T()) {
  if(n) root= build([&](int) { return v; }, 0, n);
 }
 PersistentSeq(const std::vector<T>& v) {
  if(!v.empty()) root= build([&](int i) { return v[i]; }, 0, (int)v.size());
 }
 template <class It, std::enable_if_t<!std::is_integral_v<It>, int> = 0> PersistentSeq(It bg, It ed) {
  const std::vector<T> v(bg, ed);
  if(!v.empty()) root= build([&](int i) { return v[i]; }, 0, (int)v.size());
 }
 int size() const { return size(root); }
 bool empty() const { return !root; }
 void clear() { root= 0; }
 std::pair<PersistentSeq, PersistentSeq> split(int k) const {
  int a, b;
  split(root, k, a, b);
  return {of(a), of(b)};
 }
 std::tuple<PersistentSeq, PersistentSeq, PersistentSeq> split3(int l, int r) const {
  int a, b, c;
  split(root, r, b, c);
  split(b, l, a, b);
  return {of(a), of(b), of(c)};
 }
 PersistentSeq& operator+=(const PersistentSeq& rhs) { return root= merge(root, rhs.root), *this; }
 PersistentSeq operator+(const PersistentSeq& rhs) const { return of(merge(root, rhs.root)); }
 void insert(int k, const T& v) {
  int a, b;
  split(root, k, a, b);
  root= merge(merge(a, leaf(v)), b);
 }
 void push_back(const T& v) { root= merge(root, leaf(v)); }
 void push_front(const T& v) { root= merge(leaf(v), root); }
 T erase(int k) {
  const T v= get(k);
  int a, b, c;
  split(root, k + 1, b, c);
  split(b, k, a, b);
  root= merge(a, c);
  return v;
 }
 T pop_back() { return erase(size() - 1); }
 T pop_front() { return erase(0); }
 T get(int k) const { return get(root, k); }
 T operator[](int k) const { return get(root, k); }
 void set(int k, const T& v) { root= set(root, k, v); }
 void mul(int k, const T& v) {
  static_assert(MO, "mul はモノイドがあるときだけ");
  set(k, M::op(get(k), v));
 }
 T prod(int l, int r) const {
  static_assert(MO, "prod はモノイドがあるときだけ");
  if(l >= r) return M::ti();
  return prod(root, l, r, false);
 }
 T all_prod() const {
  static_assert(MO, "all_prod はモノイドがあるときだけ");
  return root ? sumv(root) : M::ti();
 }
 void apply(int l, int r, const E& e) {
  static_assert(AC, "apply は作用があるときだけ");
  root= apply(root, l, r, e);
 }
 void reverse() {
  static_assert(REV, "reverse は REV のときだけ");
  if(root) root= push_to(root, true, false, EA());
 }
 void reverse(int l, int r) {
  static_assert(REV, "reverse は REV のときだけ");
  if(r - l < 2) return;
  int a, b, c;
  split(root, r, b, c);
  split(b, l, a, b);
  b= push_to(b, true, false, EA());
  root= merge(merge(a, b), c);
 }
 std::vector<T> dump() const {
  std::vector<T> out;
  std::vector<E> acts;
  out.reserve(size());
  if(root) dump(root, false, out, acts);
  return out;
 }
 // 置き場が前の gc のあとに生きていた節の 2 倍まで伸びたら真。
 static bool pool_empty() { return nd.size() + lv.size() >= gc_at; }
 // 渡した版から届く節だけを残して置き場を詰める。渡さなかった木は使えなくなる。
 static void gc(std::vector<PersistentSeq>& vs) {
  std::vector<int*> rs;
  rs.reserve(vs.size());
  for(auto& v: vs) rs.push_back(&v.root);
  gc_roots(rs);
 }
 template <class... Ps> static void gc(PersistentSeq& v, Ps&... vs) {
  std::vector<int*> rs{&v.root, &vs.root...};
  gc_roots(rs);
 }
 // 置き場の節の数と、木の高さ、重みの比の条件 (試し用)。
 static std::size_t pool_nodes() { return nd.size() + lv.size(); }
 int height() const { return height(root); }
 bool balanced() const { return balanced(root); }
private:
 static int height(int c) { return c > 0 ? 1 + std::max(height(nd[c].l), height(nd[c].r)) : 0; }
 static bool balanced(int c) { return c <= 0 || (bal(size(nd[c].l), size(nd[c].r)) && balanced(nd[c].l) && balanced(nd[c].r)); }
};
