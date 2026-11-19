#pragma once
// 列の木 (塊の形) の手直しの試作 Seq2。NeoLibrary の neo/data_structure/Sequence.hpp (93da937) からの変更は次のとおり (記録は
// algo-notes の notes/sequence-tree.md)。
//  - 塊の長さを、列の両端を除いて MIN 以上に保つ。併合では、境目の短い塊を MIN 以上になるまで続けてまとめ、1 つに収まらなければ
//    2 つの塊で半分ずつに分け直す (Library Checker の nandhagk の 402977 と同じ決まり)。
//  - 区間の反転で両端の塊の外の部分を入れ替えたあと、塊が CAP を超えるときだけ割る。MIN より短くなった塊は、つなぎ直すときに
//    隣の塊とまとめる。入れ替える 2 つの塊は、反転の印を残したまま外の部分だけを書き換える (塊全体を並べ替えない)。
//  - FA の道 (monoid::AffineSum<ZMod<P>>) は、塊の大きさを S2_CAP (512)、MIN を S2_CAP / S2_MIN_DIV (64)、作るときの塊を
//    S2_CAP * S2_FILL / 8 (128) にする。作用は恒等写像で初めて、印で分けずに合成する。塊を割る、反転で入れ替える、削除、代入、
//    mul では、塊の和を短い側の和の足し引きで求める。
//  - 汎用の道の MIN と作るときの塊は今までと同じ (CAP / 4 と CAP * 3 / 4、S2_GMIN_DIV と S2_GFILL で変えられる)。
//  - 節を 48 byte に詰める (塊の長さを 16 bit、印を 1 byte にし、使わない欄は [[no_unique_address]] で消す)。
//  - 併合で塊をまとめるときは、塊の積を 2 つの塊の積から求める (配列を畳み直さない)。
//  - mul は木を 1 回だけ下りる。
//  - 可換でないモノイドを反転なし (REV = false) で使うと、塊の途中への挿入で塊の積を誤っていた (Sequence.hpp の put が可換を
//    前提にしていた) のを直す。
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/data_structure/segtree/Affine.hpp"
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#ifdef __linux__
#include <sys/mman.h>
#endif
#ifndef S2_CAP
#define S2_CAP 512
#endif
#ifndef S2_MIN_DIV
#define S2_MIN_DIV 8
#endif
#ifndef S2_FILL
#define S2_FILL 2
#endif
#ifndef S2_GMIN_DIV
#define S2_GMIN_DIV 4
#endif
#ifndef S2_GFILL
#define S2_GFILL 6
#endif
namespace seq2_internal {
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
template <int> struct Nil {};
template <class T> struct Alloc {
 using value_type= T;
 Alloc()= default;
 template <class U> Alloc(const Alloc<U>&) {}
 T* allocate(std::size_t n) {
  const std::size_t want= n * sizeof(T);
  if(want < (std::size_t(1) << 20)) return static_cast<T*>(::operator new(want, std::align_val_t(64)));
  const std::size_t H= std::size_t(1) << 21, bytes= (want + H - 1) & ~(H - 1);
  T* p= static_cast<T*>(::operator new(bytes, std::align_val_t(H)));
#ifdef __linux__
  madvise(p, bytes, MADV_HUGEPAGE);
#endif
  return p;
 }
 void deallocate(T* p, std::size_t n) {
  const std::size_t want= n * sizeof(T);
  if(want < (std::size_t(1) << 20)) ::operator delete(p, std::align_val_t(64));
  else ::operator delete(p, std::align_val_t(std::size_t(1) << 21));
 }
 template <class U> bool operator==(const Alloc<U>&) const { return true; }
};
template <class M> struct fast_affine: std::false_type {};
template <unsigned P> struct fast_affine<::monoid::AffineSum<ZMod<P>>>: std::bool_constant<P != 0> {};
using u32= unsigned;
using u64= unsigned long long;
template <unsigned P> struct Kern {
 static constexpr u32 P2= 2 * P;
 struct Act {
  __m256i m, q, c;
 };
 static Act act(const std::array<ZMod<P>, 2>& f) {
  const u32 w= f[0].val();
  return {_mm256_set1_epi32(int(w)), _mm256_set1_epi32(int(u32((u64(w) << 32) / P))), _mm256_set1_epi32(int(f[1].val()))};
 }
 [[gnu::always_inline]] static __m256i mul(__m256i x, __m256i m, __m256i q) {
  const __m256i e= _mm256_mul_epu32(x, q), o= _mm256_mul_epu32(_mm256_srli_epi64(x, 32), q);
  const __m256i h= _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xaa);
  return _mm256_sub_epi32(_mm256_mullo_epi32(x, m), _mm256_mullo_epi32(h, _mm256_set1_epi32(int(P))));
 }
 [[gnu::always_inline]] static __m256i mul_lanes(__m256i x, __m256i m, __m256i q) {
  const __m256i e= _mm256_mul_epu32(x, q), o= _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(q, 32));
  const __m256i h= _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xaa);
  return _mm256_sub_epi32(_mm256_mullo_epi32(x, m), _mm256_mullo_epi32(h, _mm256_set1_epi32(int(P))));
 }
 [[gnu::always_inline]] static __m256i add(__m256i a, __m256i b) {
  const __m256i s= _mm256_add_epi32(a, b);
  return _mm256_min_epu32(s, _mm256_sub_epi32(s, _mm256_set1_epi32(int(P2))));
 }
 static u32 hsum(__m256i s) {
  const __m128i p2= _mm_set1_epi32(int(P2));
  __m128i t= _mm_add_epi32(_mm256_castsi256_si128(s), _mm256_extracti128_si256(s, 1));
  t= _mm_min_epu32(t, _mm_sub_epi32(t, p2));
  t= _mm_add_epi32(t, _mm_shuffle_epi32(t, 0x4e));
  t= _mm_min_epu32(t, _mm_sub_epi32(t, p2));
  t= _mm_add_epi32(t, _mm_shuffle_epi32(t, 0xb1));
  t= _mm_min_epu32(t, _mm_sub_epi32(t, p2));
  return u32(_mm_cvtsi128_si32(t));
 }
 [[gnu::always_inline]] static __m256i iota8() { return _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7); }
 [[gnu::always_inline]] static __m256i ld(const u32* p) { return _mm256_load_si256((const __m256i*)p); }
 [[gnu::always_inline]] static void st(u32* p, __m256i x) { _mm256_store_si256((__m256i*)p, x); }
 static u32 sum_range(const u32* a, int lo, int hi) {
  if(lo >= hi) return 0;
  const __m256i L= _mm256_set1_epi32(lo - 1), H= _mm256_set1_epi32(hi), step= _mm256_set1_epi32(8);
  int k= lo >> 3;
  const int ke= (hi + 7) >> 3;
  __m256i idx= _mm256_add_epi32(iota8(), _mm256_set1_epi32(k << 3)), s0= _mm256_setzero_si256(), s1= s0;
  for(; k + 1 < ke; k+= 2) {
   const __m256i i1= _mm256_add_epi32(idx, step);
   const __m256i k0= _mm256_and_si256(_mm256_cmpgt_epi32(idx, L), _mm256_cmpgt_epi32(H, idx));
   const __m256i k1= _mm256_and_si256(_mm256_cmpgt_epi32(i1, L), _mm256_cmpgt_epi32(H, i1));
   s0= add(s0, _mm256_and_si256(ld(a + (k << 3)), k0));
   s1= add(s1, _mm256_and_si256(ld(a + (k << 3) + 8), k1));
   idx= _mm256_add_epi32(i1, step);
  }
  if(k < ke) s0= add(s0, _mm256_and_si256(ld(a + (k << 3)), _mm256_and_si256(_mm256_cmpgt_epi32(idx, L), _mm256_cmpgt_epi32(H, idx))));
  return hsum(add(s0, s1));
 }
 static void transform_all(u32* a, int len, const Act& f) {
  for(int k= 0; k < len; k+= 8) st(a + k, add(mul(ld(a + k), f.m, f.q), f.c));
 }
 static u32 transform_range(u32* a, int lo, int hi, const Act& f) {
  const __m256i L= _mm256_set1_epi32(lo - 1), H= _mm256_set1_epi32(hi), step= _mm256_set1_epi32(8);
  int k= lo >> 3;
  const int ke= (hi + 7) >> 3;
  __m256i idx= _mm256_add_epi32(iota8(), _mm256_set1_epi32(k << 3)), s= _mm256_setzero_si256();
  for(; k < ke; ++k) {
   const __m256i x= ld(a + (k << 3));
   const __m256i msk= _mm256_and_si256(_mm256_cmpgt_epi32(idx, L), _mm256_cmpgt_epi32(H, idx));
   st(a + (k << 3), _mm256_blendv_epi8(x, add(mul(x, f.m, f.q), f.c), msk));
   s= add(s, _mm256_and_si256(x, msk));
   idx= _mm256_add_epi32(idx, step);
  }
  return hsum(s);
 }
 static u32 transform2(u32* a, int len, int lo, int hi, const Act& in, const Act& out) {
  const __m256i L= _mm256_set1_epi32(lo - 1), H= _mm256_set1_epi32(hi), N= _mm256_set1_epi32(len), step= _mm256_set1_epi32(8);
  __m256i idx= iota8(), s= _mm256_setzero_si256();
  for(int k= 0; k < len; k+= 8) {
   const __m256i msk= _mm256_and_si256(_mm256_cmpgt_epi32(idx, L), _mm256_cmpgt_epi32(H, idx));
   const __m256i m= _mm256_blendv_epi8(out.m, in.m, msk), q= _mm256_blendv_epi8(out.q, in.q, msk), c= _mm256_blendv_epi8(out.c, in.c, msk);
   const __m256i y= add(mul_lanes(ld(a + k), m, q), c);
   st(a + k, y);
   s= add(s, _mm256_and_si256(y, _mm256_cmpgt_epi32(N, idx)));
   idx= _mm256_add_epi32(idx, step);
  }
  return hsum(s);
 }
};
template <class M, bool FA= fast_affine<M>::value> struct kern_of {
 using type= void;
};
template <class M> struct kern_of<M, true> {
 using type= Kern<M::T::mod()>;
};
}  // namespace seq2_internal
template <class M, bool REV= false, int B= 0> class Seq2 {
 using T= typename seq2_internal::value_of<M>::type;
 using E= typename seq2_internal::action_of<M>::type;
 static constexpr bool MO= seq2_internal::monoid<M>::value, AC= seq2_internal::action<M>::value;
 static constexpr bool FA= seq2_internal::fast_affine<M>::value;
 using K= typename seq2_internal::kern_of<M>::type;
 static constexpr bool CM= FA || seq2_internal::commute<M>::value;  // 塊の積を順を問わずに足せる
 static constexpr bool RS= MO && REV && !CM;
 static constexpr int CAP= B ? B : FA ? S2_CAP : std::clamp<int>(int(512 / sizeof(T)) / 16 * 16, 16, 128), MIN= CAP / (FA ? S2_MIN_DIV : S2_GMIN_DIV), FILL= std::max(1, CAP * (FA ? S2_FILL : S2_GFILL) / 8);
 static_assert(!FA || CAP % 8 == 0, "AVX2 の核は塊の大きさが 8 の倍数であること");
 static_assert(0 < CAP && CAP < 65536, "塊の長さは 16 bit に収める");
 template <int I> using Nil= seq2_internal::Nil<I>;
 using S= std::conditional_t<MO, T, Nil<0>>;
 using CF= std::conditional_t<MO, T, Nil<1>>;
 using SR= std::conditional_t<RS, T, Nil<2>>;
 using CB= std::conditional_t<RS, T, Nil<3>>;
 using EA= std::conditional_t<AC, E, Nil<4>>;
 using CE= std::conditional_t<AC, E, Nil<5>>;
 enum : unsigned char { RV= 1, LZ= 2, CRV= 4, CLZ= 8 };  // RV と LZ は子へ配る分、CRV と CLZ は自分の塊の分
 struct Node {
  int l, r, cnt;
  unsigned pri;
  unsigned short len, fr, bk;  // fr と bk は部分木の左端と右端の塊の長さ
  unsigned char fl;
  [[no_unique_address]] S sum;  // 部分木の積
  [[no_unique_address]] CF cf;  // 塊を配列の順に畳んだ積 (どちらも作用を当てたあとの値)
  [[no_unique_address]] SR rsum;  // 部分木の逆順の積
  [[no_unique_address]] CB cb;  // 塊を配列の逆順に畳んだ積
  [[no_unique_address]] EA e;  // 子へ配る作用
  [[no_unique_address]] CE ce;  // 自分の塊へ当てる作用
 };
 static Node blank(unsigned pri) {
  Node n{};
  n.pri= pri;
  if constexpr(MO) n.sum= n.cf= M::ti();
  if constexpr(RS) n.rsum= n.cb= M::ti();
  if constexpr(FA) n.e= n.ce= E{T(1), T(0)};
  return n;
 }
 template <class U> using Vec= std::vector<U, seq2_internal::Alloc<U>>;
 static inline Vec<Node> nd= Vec<Node>(1, blank(0));
 static inline Vec<T> dat;
 static inline std::vector<int> freed;
 static unsigned seed() {
  const auto t= (unsigned long long)std::chrono::steady_clock::now().time_since_epoch().count();
  const unsigned v= unsigned(t ^ (t >> 32)) ^ 2463534242u;
  return v ? v : 2463534242u;
 }
 static inline unsigned rs= seed();
 static inline std::vector<int> path, path2;
 static inline std::vector<T> tu, tv;
 static unsigned rnd() { return rs^= rs << 13, rs^= rs >> 17, rs^= rs << 5; }
 static T* arr(int x) { return dat.data() + std::size_t(x) * CAP; }
 static seq2_internal::u32* raw(T* a) { return reinterpret_cast<seq2_internal::u32*>(a); }
 static const seq2_internal::u32* raw(const T* a) { return reinterpret_cast<const seq2_internal::u32*>(a); }
 static int make(unsigned pri) {
  if(!freed.empty()) {
   const int x= freed.back();
   freed.pop_back();
   nd[x]= blank(pri);
   return x;
  }
  nd.push_back(blank(pri));
  if(dat.size() < nd.size() * CAP) dat.resize(std::max(nd.size() * CAP, dat.size() * 2));
  return (int)nd.size() - 1;
 }
 static void release(int x) { freed.push_back(x); }
 static void map1(T& v, const E& e, int len) {
  if constexpr(std::is_invocable_v<decltype(M::mp), T&, const E&, int>) M::mp(v, e, len);
  else M::mp(v, e);
 }
 static T fold(const T* a, int lo, int hi, bool rv) {
  if constexpr(FA) return T::raw(K::sum_range(raw(a), lo, hi));
  T s= M::ti();
  if(RS && rv)
   for(int i= hi; i-- > lo;) s= M::op(s, a[i]);
  else
   for(int i= lo; i < hi; ++i) s= M::op(s, a[i]);
  return s;
 }
 // FA の道で、塊の作用を当て終えた節 x の配列の [0, k) と [k, len) の和。短い側を足し、長い側は塊の和から引く。
 static T fa_prefix(int x, int k) {
  const auto* a= raw(arr(x));
  const int len= nd[x].len;
  if(2 * k <= len) return T::raw(K::sum_range(a, 0, k));
  return nd[x].cf - T::raw(K::sum_range(a, k, len));
 }
 static T fa_suffix(int x, int k) {
  const auto* a= raw(arr(x));
  const int len= nd[x].len;
  if(2 * k >= len) return T::raw(K::sum_range(a, k, len));
  return nd[x].cf - T::raw(K::sum_range(a, 0, k));
 }
 // FA の道で、塊の作用を当て終えた節 x の配列の [lo, hi) の和。短い側を足す。
 static T fa_range(int x, int lo, int hi) {
  const auto* a= raw(arr(x));
  const int len= nd[x].len;
  if(2 * (hi - lo) <= len) return T::raw(K::sum_range(a, lo, hi));
  return nd[x].cf - (T::raw(K::sum_range(a, 0, lo)) + T::raw(K::sum_range(a, hi, len)));
 }
 static const T& lsum(const Node& n) {
  if constexpr(RS) return n.fl & CRV ? n.cb : n.cf;
  else return n.cf;
 }
 static const T& lrsum(const Node& n) {
  if constexpr(RS) return n.fl & CRV ? n.cf : n.cb;
  else return n.cb;
 }
 static void recalc(int x) {
  if constexpr(MO) {
   Node& n= nd[x];
   const T* a= arr(x);
   n.cf= fold(a, 0, n.len, false);
   if constexpr(RS) n.cb= fold(a, 0, n.len, true);
   if constexpr(AC)
    if(n.fl & CLZ) {
     map1(n.cf, n.ce, n.len);
     if constexpr(RS) map1(n.cb, n.ce, n.len);
    }
  }
 }
 static void pull(int x) {
  Node& n= nd[x];
  const Node &L= nd[n.l], &R= nd[n.r];
  n.cnt= L.cnt + n.len + R.cnt;
  n.fr= n.l ? L.fr : n.len, n.bk= n.r ? R.bk : n.len;
  if constexpr(MO) n.sum= M::op(M::op(L.sum, lsum(n)), R.sum);
  if constexpr(RS) n.rsum= M::op(M::op(R.rsum, lrsum(n)), L.rsum);
 }
 static void aff(int x, const E& e) {
  Node& n= nd[x];
  if constexpr(MO) {
   map1(n.sum, e, n.cnt), map1(n.cf, e, n.len);
   if constexpr(RS) map1(n.rsum, e, n.cnt), map1(n.cb, e, n.len);
  }
  if constexpr(FA) M::cp(n.e, e), M::cp(n.ce, e);
  else {
   if(n.fl & LZ) M::cp(n.e, e);
   else n.e= e;
   if(n.fl & CLZ) M::cp(n.ce, e);
   else n.ce= e;
  }
  n.fl|= LZ | CLZ;
 }
 static void tog(int x) {
  Node& n= nd[x];
  std::swap(n.l, n.r), std::swap(n.fr, n.bk);
  if constexpr(RS) std::swap(n.sum, n.rsum);
  n.fl^= RV | CRV;
 }
 static void push(int x) {
  Node& n= nd[x];
  if constexpr(REV)
   if(n.fl & RV) {
    if(n.l) tog(n.l);
    if(n.r) tog(n.r);
    n.fl&= ~RV;
   }
  if constexpr(AC)
   if(n.fl & LZ) {
    const E e= n.e;
    if(n.l) aff(n.l, e);
    if(n.r) aff(n.r, e);
    n.fl&= ~LZ;
    if constexpr(FA) n.e= E{T(1), T(0)};
   }
 }
 static void materialize(int x) {
  if constexpr(AC) {
   Node& n= nd[x];
   if(!(n.fl & CLZ)) return;
   T* a= arr(x);
   if constexpr(FA) K::transform_all(raw(a), n.len, K::act(n.ce));
   else
    for(int i= 0; i < n.len; ++i) map1(a[i], n.ce, 1);
   n.fl&= ~CLZ;
   if constexpr(FA) n.ce= E{T(1), T(0)};
  }
 }
 static void forward(int x) {
  materialize(x);
  if constexpr(REV) {
   Node& n= nd[x];
   if(n.fl & CRV) {
    std::reverse(arr(x), arr(x) + n.len), n.fl&= ~CRV;
    if constexpr(RS) std::swap(n.cf, n.cb);
   }
  }
 }
 static int phys(const Node& n, int o) { return REV && (n.fl & CRV) ? n.len - 1 - o : o; }
 static T part_sum(int x, int cl, int cr) {
  const Node& n= nd[x];
  const bool rv= REV && (n.fl & CRV);
  const int pl= rv ? n.len - cr : cl, pr= rv ? n.len - cl : cr;
  if constexpr(FA) {
   const int w= pr - pl;
   const auto* a= raw(arr(x));
   const bool lz= n.fl & CLZ;
   if(2 * w <= n.len) {
    const T s= T::raw(K::sum_range(a, pl, pr));
    return lz ? n.ce[0] * s + n.ce[1] * T(w) : s;
   }
   const T s= T::raw(K::sum_range(a, 0, pl)) + T::raw(K::sum_range(a, pr, n.len));
   return n.cf - (lz ? n.ce[0] * s + n.ce[1] * T(n.len - w) : s);
  }
  T s= fold(arr(x), pl, pr, rv);
  if constexpr(AC)
   if(n.fl & CLZ) map1(s, n.ce, cr - cl);
  return s;
 }
 static void part_apply(int x, int cl, int cr, const E& e) {
  if constexpr(FA) {
   Node& n= nd[x];
   const bool rv= REV && (n.fl & CRV);
   const int len= n.len, pl= rv ? len - cr : cl, pr= rv ? len - cl : cr;
   auto* a= raw(arr(x));
   if(n.fl & CLZ) {
    const E in{e[0] * n.ce[0], e[0] * n.ce[1] + e[1]};
    n.cf= T::raw(K::transform2(a, len, pl, pr, K::act(in), K::act(n.ce)));
    n.fl&= ~CLZ;
    n.ce= E{T(1), T(0)};
   } else {
    const T old= T::raw(K::transform_range(a, pl, pr, K::act(e)));
    n.cf+= (e[0] - T(1)) * old + e[1] * T(pr - pl);
   }
   return;
  }
  materialize(x);
  const Node& n= nd[x];
  const bool rv= REV && (n.fl & CRV);
  const int pl= rv ? n.len - cr : cl, pr= rv ? n.len - cl : cr;
  T* a= arr(x);
  for(int i= pl; i < pr; ++i) map1(a[i], e, 1);
  recalc(x);
 }
 static void split(int x, int k, int& a, int& b) {
  if(!x) return void(a= b= 0);
  push(x);
  const int ls= nd[nd[x].l].cnt, len= nd[x].len;
  if(k <= ls) {
   int p, q;
   split(nd[x].l, k, p, q);
   nd[x].l= q, pull(x), a= p, b= x;
  } else if(k >= ls + len) {
   int p, q;
   split(nd[x].r, k - ls - len, p, q);
   nd[x].r= p, pull(x), a= x, b= q;
  } else {
   const int o= k - ls;
   forward(x);
   const int y= make(rnd());
   std::copy(arr(x) + o, arr(x) + len, arr(y));
   if constexpr(FA) {
    const T s= fa_prefix(x, o);
    nd[y].cf= nd[x].cf - s, nd[x].cf= s;
   }
   nd[y].len= len - o, nd[x].len= o;
   if constexpr(!FA) recalc(x), recalc(y);
   pull(y);
   const int r= nd[x].r;
   nd[x].r= 0, pull(x);
   a= x, b= merge_(y, r);
  }
 }
 static int merge_(int a, int b) {
  if(!a || !b) return a | b;
  if(nd[a].pri > nd[b].pri) {
   push(a);
   const int r= merge_(nd[a].r, b);
   nd[a].r= r, pull(a);
   return a;
  }
  push(b);
  const int l= merge_(a, nd[b].l);
  nd[b].l= l, pull(b);
  return b;
 }
 static int take_first(int& b) {
  path.clear();
  int x= b;
  for(;;) {
   push(x);
   if(!nd[x].l) break;
   path.push_back(x), x= nd[x].l;
  }
  const int r= nd[x].r;
  if(path.empty()) b= r;
  else nd[path.back()].l= r;
  for(int k= (int)path.size(); k--;) pull(path[k]);
  forward(x);
  return x;
 }
 // a の末尾の塊に、節 f の塊 (列の順に並べてある) を足して、f を外す。塊の積は 2 つの積から求める。
 static void absorb_last(int a, int f) {
  path.clear();
  int x= a;
  for(;;) {
   push(x), path.push_back(x);
   if(!nd[x].r) break;
   x= nd[x].r;
  }
  forward(x);
  std::copy(arr(f), arr(f) + nd[f].len, arr(x) + nd[x].len);
  nd[x].len+= nd[f].len;
  if constexpr(MO) nd[x].cf= M::op(nd[x].cf, nd[f].cf);
  if constexpr(RS) nd[x].cb= M::op(nd[f].cb, nd[x].cb);
  for(int k= (int)path.size(); k--;) pull(path[k]);
  release(f);
 }
 // a の末尾の塊と b の先頭の塊を、長さが半分ずつになるように移し合う (どちらかが MIN より短く、1 つに収まらないとき)。
 static void rebalance(int a, int b) {
  path.clear(), path2.clear();
  int x= a, y= b;
  for(;;) {
   push(x), path.push_back(x);
   if(!nd[x].r) break;
   x= nd[x].r;
  }
  for(;;) {
   push(y), path2.push_back(y);
   if(!nd[y].l) break;
   y= nd[y].l;
  }
  forward(x), forward(y);
  const int nx= nd[x].len, ny= nd[y].len, tg= (nx + ny) / 2;
  T *X= arr(x), *Y= arr(y);
  if(nx < tg) {
   const int m= tg - nx;
   if constexpr(FA) {
    const T s= fa_prefix(y, m);
    nd[x].cf+= s, nd[y].cf-= s;
   }
   std::copy(Y, Y + m, X + nx), std::move(Y + m, Y + ny, Y);
  } else {
   const int m= nx - tg;
   if constexpr(FA) {
    const T s= fa_suffix(x, tg);
    nd[x].cf-= s, nd[y].cf+= s;
   }
   std::move_backward(Y, Y + ny, Y + ny + m), std::copy(X + tg, X + nx, Y);
  }
  nd[x].len= tg, nd[y].len= nx + ny - tg;
  if constexpr(!FA) recalc(x), recalc(y);
  pull_path(path), pull_path(path2);
 }
 static int merge(int a, int b) {
  while(a && b && (nd[a].bk < MIN || nd[b].fr < MIN)) {
   if(nd[a].bk + nd[b].fr <= CAP) {
    const int f= take_first(b);
    absorb_last(a, f);
   } else {
    rebalance(a, b);
    break;
   }
  }
  return merge_(a, b);
 }
 static T prod(int x, int l, int r) {
  if(!x || l >= r) return M::ti();
  if(l == 0 && r == nd[x].cnt) return nd[x].sum;
  push(x);
  const int ls= nd[nd[x].l].cnt, len= nd[x].len;
  T s= M::ti();
  if(l < ls) s= prod(nd[x].l, l, std::min(r, ls));
  const int cl= std::max(l - ls, 0), cr= std::min(r - ls, len);
  if(cl < cr) s= M::op(s, cl == 0 && cr == len ? lsum(nd[x]) : part_sum(x, cl, cr));
  if(r > ls + len) s= M::op(s, prod(nd[x].r, std::max(l - ls - len, 0), r - ls - len));
  return s;
 }
 static void apply(int x, int l, int r, const E& e) {
  if(!x || l >= r) return;
  if(l == 0 && r == nd[x].cnt) return aff(x, e);
  push(x);
  const int ls= nd[nd[x].l].cnt, len= nd[x].len;
  if(l < ls) apply(nd[x].l, l, std::min(r, ls), e);
  const int cl= std::max(l - ls, 0), cr= std::min(r - ls, len);
  if(cl < cr) {
   if(cl == 0 && cr == len) {
    Node& n= nd[x];
    if constexpr(MO) {
     map1(n.cf, e, len);
     if constexpr(RS) map1(n.cb, e, len);
    }
    if constexpr(FA) M::cp(n.ce, e);
    else if(n.fl & CLZ) M::cp(n.ce, e);
    else n.ce= e;
    n.fl|= CLZ;
   } else part_apply(x, cl, cr, e);
  }
  if(r > ls + len) apply(nd[x].r, std::max(l - ls - len, 0), r - ls - len, e);
  pull(x);
 }
 static int descend(int rt, int i, bool end, std::vector<int>& ps, int& o) {
  ps.clear();
  int x= rt;
  o= i;
  for(;;) {
   push(x), ps.push_back(x);
   const int ls= nd[nd[x].l].cnt;
   if(o < ls) x= nd[x].l;
   else if(o < ls + nd[x].len + (end ? 1 : 0)) {
    o-= ls;
    return x;
   } else o-= ls + nd[x].len, x= nd[x].r;
  }
 }
 static void pull_path(const std::vector<int>& ps) {
  for(int k= (int)ps.size(); k--;) pull(ps[k]);
 }
 static void put(int x, int o, const T& v) {
  materialize(x);
  Node& n= nd[x];
  T* a= arr(x);
  const int p= REV && (n.fl & CRV) ? n.len - o : o;
  std::move_backward(a + p, a + n.len, a + n.len + 1);
  a[p]= v, ++n.len;
  if constexpr(MO) {
   if constexpr(CM) n.cf= M::op(n.cf, v);  // 可換なので順は問わない
   else recalc(x);
  }
 }
 static void collect(int x, std::vector<T>& out) {
  if(!x) return;
  push(x), forward(x);
  collect(nd[x].l, out);
  out.insert(out.end(), arr(x), arr(x) + nd[x].len);
  collect(nd[x].r, out);
 }
 template <class F> static int build(const F& at, int n, int lo, int hi, int depth) {
  if(lo >= hi) return 0;
  constexpr int H= FILL;
  const int m= (lo + hi) / 2;
  const int x= make(~0u - (unsigned(depth) << 24) - (rnd() & 0xffffffu));
  const int s= m * H, e= std::min(s + H, n);
  T* a= arr(x);
  for(int i= s; i < e; ++i) a[i - s]= at(i);
  nd[x].len= e - s, recalc(x);
  const int l= build(at, n, lo, m, depth + 1), r= build(at, n, m + 1, hi, depth + 1);
  nd[x].l= l, nd[x].r= r, pull(x);
  return x;
 }
 template <class F> static int build(const F& at, int n) {
  constexpr int H= FILL;
  return build(at, n, 0, (n + H - 1) / H, 0);
 }
 int root= 0;
 static Seq2 of(int rt) {
  Seq2 s;
  s.root= rt;
  return s;
 }
public:
 Seq2()= default;
 explicit Seq2(int n, const T& v= T()) { root= build([&](int) { return v; }, n); }
 Seq2(const std::vector<T>& v) { root= build([&](int i) { return v[i]; }, (int)v.size()); }
 template <class It, std::enable_if_t<!std::is_integral_v<It>, int> = 0> Seq2(It bg, It ed) {
  const std::vector<T> v(bg, ed);
  root= build([&](int i) { return v[i]; }, (int)v.size());
 }
 int size() const { return nd[root].cnt; }
 bool empty() const { return !root; }
 void clear() { root= 0; }
 std::pair<Seq2, Seq2> split(int k) {
  int a, b;
  split(root, k, a, b);
  return {of(a), of(b)};
 }
 std::tuple<Seq2, Seq2, Seq2> split3(int l, int r) {
  int a, b, c;
  split(root, r, b, c);
  split(b, l, a, b);
  return {of(a), of(b), of(c)};
 }
 Seq2& operator+=(Seq2 rhs) { return root= merge(root, rhs.root), *this; }
 Seq2 operator+(Seq2 rhs) const { return of(merge(root, rhs.root)); }
 void insert(int k, const T& v) {
  if(!root) {
   root= make(rnd());
   arr(root)[0]= v, nd[root].len= 1, recalc(root), pull(root);
   return;
  }
  int o;
  const int x= descend(root, k, true, path2, o);
  if(nd[x].len < CAP) {
   put(x, o, v);
   pull_path(path2);
   return;
  }
  forward(x);
  constexpr int H= CAP / 2;
  const int y= make(rnd());
  std::copy(arr(x) + H, arr(x) + CAP, arr(y));
  if constexpr(FA) {
   const T s= fa_suffix(x, H);
   nd[y].cf= s, nd[x].cf-= s;
  }
  nd[y].len= CAP - H, nd[x].len= H;
  if constexpr(!FA) recalc(x), recalc(y);
  if(o <= H) put(x, o, v);
  else put(y, o - H, v);
  pull(y), pull_path(path2);
  int a, b;
  split(root, k - o + nd[x].len, a, b);
  root= merge_(merge_(a, y), b);
 }
 void push_back(const T& v) { insert(size(), v); }
 void push_front(const T& v) { insert(0, v); }
 T erase(int k) {
  int o;
  const int x= descend(root, k, false, path2, o);
  Node& n= nd[x];
  T* a= arr(x);
  const int p= phys(n, o);
  T v= a[p];
  if constexpr(AC)
   if(n.fl & CLZ) map1(v, n.ce, 1);
  std::move(a + p + 1, a + n.len, a + p);
  --n.len;
  if(!n.len) {
   const int m= merge_(n.l, n.r);
   path2.pop_back();
   if(path2.empty()) root= m;
   else {
    const int par= path2.back();
    (nd[par].l == x ? nd[par].l : nd[par].r)= m;
   }
   release(x);
  } else if constexpr(FA) n.cf-= v;
  else recalc(x);
  pull_path(path2);
  return v;
 }
 T pop_back() { return erase(size() - 1); }
 T pop_front() { return erase(0); }
 T get(int k) {
  int o;
  const int x= descend(root, k, false, path2, o);
  T v= arr(x)[phys(nd[x], o)];
  if constexpr(AC)
   if(nd[x].fl & CLZ) map1(v, nd[x].ce, 1);
  return v;
 }
 T operator[](int k) { return get(k); }
 void set(int k, const T& v) {
  int o;
  const int x= descend(root, k, false, path2, o);
  materialize(x);
  T& w= arr(x)[phys(nd[x], o)];
  if constexpr(FA) nd[x].cf+= v - w, w= v;
  else w= v, recalc(x);
  pull_path(path2);
 }
 void mul(int k, const T& v) {
  static_assert(MO, "mul はモノイドがあるときだけ");
  int o;
  const int x= descend(root, k, false, path2, o);
  materialize(x);
  T& w= arr(x)[phys(nd[x], o)];
  w= M::op(w, v);
  if constexpr(CM) nd[x].cf= M::op(nd[x].cf, v);
  else recalc(x);
  pull_path(path2);
 }
 T prod(int l, int r) {
  static_assert(MO, "prod はモノイドがあるときだけ");
  return prod(root, l, r);
 }
 T all_prod() const {
  static_assert(MO, "all_prod はモノイドがあるときだけ");
  return nd[root].sum;
 }
 void apply(int l, int r, const E& e) {
  static_assert(AC, "apply は作用があるときだけ");
  apply(root, l, r, e);
 }
 void reverse() {
  static_assert(REV, "reverse は REV のときだけ");
  if(root) tog(root);
 }
 void reverse(int l, int r) {
  static_assert(REV, "reverse は REV のときだけ");
  if(r - l < 2) return;
  int i, jj;
  const int u= descend(root, l, false, path, i);
  const int v= descend(root, r - 1, false, path2, jj);
  const int j= jj + 1;
  if(u == v) {
   const Node& n= nd[u];
   const bool rv= n.fl & CRV;
   const int pl= rv ? n.len - j : i, pr= rv ? n.len - i : j;
   std::reverse(arr(u) + pl, arr(u) + pr);
   if constexpr(RS) recalc(u), pull_path(path);
   return;
  }
  const int nu= nd[u].len, nv= nd[v].len, tail= nv - j, newu= tail + nu - i, newv= j + i;
  if(newu > CAP || newv > CAP) {
   int a, b, c;
   split(root, l, a, b);
   split(b, r - l, b, c);
   tog(b);
   root= merge(merge(a, b), c);
   return;
  }
  if(i || tail) {
   // 塊の反転の印は残したまま、外の部分 (u の [0, i) と v の [j, nv)) だけを書き換える。
   materialize(u), materialize(v);
   const bool ru= nd[u].fl & CRV, rv= nd[v].fl & CRV;
   T *U= arr(u), *V= arr(v);
   const int up= ru ? nu - i : 0, vp= rv ? 0 : j;  // 外の部分の配列の上の位置
   if constexpr(FA) {
    const T su= fa_range(u, up, up + i), sv= fa_range(v, vp, vp + tail);
    nd[u].cf+= sv - su, nd[v].cf+= su - sv;
   }
   tu.assign(U + up, U + up + i), tv.assign(V + vp, V + vp + tail);
   if(ru) {  // u の配列は rev(u[i, nu)) + v[j, nv)
    if(rv) std::reverse_copy(tv.begin(), tv.end(), U + nu - i);
    else std::copy(tv.begin(), tv.end(), U + nu - i);
   } else {  // u の配列は rev(v[j, nv)) + u[i, nu)
    if(tail > i) std::move_backward(U + i, U + nu, U + newu);
    else std::move(U + i, U + nu, U + tail);
    if(rv) std::copy(tv.begin(), tv.end(), U);
    else std::reverse_copy(tv.begin(), tv.end(), U);
   }
   if(rv) {  // v の配列は u[0, i) + rev(v[0, j))
    if(i > tail) std::move_backward(V + tail, V + nv, V + newv);
    else std::move(V + tail, V + nv, V + i);
    if(ru) std::reverse_copy(tu.begin(), tu.end(), V);
    else std::copy(tu.begin(), tu.end(), V);
   } else {  // v の配列は v[0, j) + rev(u[0, i))
    if(ru) std::copy(tu.begin(), tu.end(), V + j);
    else std::reverse_copy(tu.begin(), tu.end(), V + j);
   }
   nd[u].len= newu, nd[v].len= newv;
   if constexpr(!FA) recalc(u), recalc(v);
   pull_path(path2), pull_path(path);
  }
  int a, b, c;
  split(root, l - i, a, b);
  split(b, r + tail - (l - i), b, c);
  tog(b);
  root= merge(merge(a, b), c);
 }
 std::vector<T> dump() {
  std::vector<T> out;
  out.reserve(size());
  collect(root, out);
  return out;
 }
};
