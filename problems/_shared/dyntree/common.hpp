#pragma once
// 動的木の下書きの共通部品。モノイド・作用の判定と、節の置き場。
#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif

namespace dyntree_internal {
struct Empty {};
template <class M>
concept Monoid = requires {
 typename M::T;
 &M::op;
 &M::ti;
};
template <class M>
concept Action = requires {
 typename M::T;
 typename M::E;
 &M::mp;
 &M::cp;
};
template <class M>
concept Commutative = requires { typename M::commute; };
// 作用の mp が区間の長さを受ける形 (辺の節が混ざる Euler Tour Tree 用)。
template <class M, bool = Action<M>>
struct ActionLenV: std::false_type {};
template <class M>
struct ActionLenV<M, true>: std::integral_constant<bool, requires(typename M::T& v, const typename M::E& e, int len) { M::mp(v, e, len); }> {};
template <class M>
concept ActionLen = ActionLenV<M>::value;
template <class M, bool = Action<M>>
struct ActionOf {
 struct type {};
};
template <class M>
struct ActionOf<M, true> {
 using type = typename M::E;
};
// 1 MB 以上の確保は 2 MB 境界に寄せ、Linux では huge page を頼む。
template <class T>
struct Alloc {
 using value_type = T;
 Alloc() = default;
 template <class U>
 Alloc(const Alloc<U>&) {}
 T* allocate(std::size_t n) {
  const std::size_t want = n * sizeof(T);
  if (want < (std::size_t(1) << 20)) return static_cast<T*>(::operator new(want, std::align_val_t(64)));
  const std::size_t H = std::size_t(1) << 21, bytes = (want + H - 1) & ~(H - 1);
  T* p = static_cast<T*>(::operator new(bytes, std::align_val_t(H)));
#ifdef __linux__
  madvise(p, bytes, MADV_HUGEPAGE);
#endif
  return p;
 }
 void deallocate(T* p, std::size_t n) {
  if (n * sizeof(T) < (std::size_t(1) << 20)) ::operator delete(p, std::align_val_t(64));
  else ::operator delete(p, std::align_val_t(std::size_t(1) << 21));
 }
 template <class U>
 bool operator==(const Alloc<U>&) const {
  return true;
 }
 template <class U>
 void construct(U* p) noexcept {
  ::new (static_cast<void*>(p)) U;
 }
 template <class U, class... A>
 void construct(U* p, A&&... a) {
  ::new (static_cast<void*>(p)) U(std::forward<A>(a)...);
 }
};
template <class T>
using Vec = std::vector<T, Alloc<T>>;
// 2 の冪の大きさの開番地法の表。V は trivially copyable で、値 0 が空の印になること。
// 埋まりが半分を超えたら広げ、消した跡は墓標にして広げ直すときに落とす。キーに 0 は使えない。
template <class V>
class OpenTable {
 struct Slot {
  unsigned long long k;
  V v;
 };
 std::vector<Slot> t;
 std::size_t mask, shift, live, used;
 static constexpr unsigned long long TOMB = ~0ull;
 std::size_t home(unsigned long long k) const { return std::size_t((k * 0x9E3779B97F4A7C15ull) >> shift); }
 void put(unsigned long long k, const V& v) {
  std::size_t h = home(k);
  while (t[h].k && t[h].k != TOMB) h = (h + 1) & mask;
  if (!t[h].k) ++used;
  t[h].k = k, t[h].v = v, ++live;
 }
 void rehash(std::size_t cap, std::size_t new_shift) {
  std::vector<Slot> old;
  old.swap(t);
  t.assign(cap, Slot{});
  mask = cap - 1, shift = new_shift, live = 0, used = 0;
  for (const Slot& s : old)
   if (s.k && s.k != TOMB) put(s.k, s.v);
 }

 public:
 OpenTable(): t(16), mask(15), shift(60), live(0), used(0) {}
 void insert(unsigned long long k, const V& v) {
  if ((used + 1) * 2 > mask + 1) {
   if ((live + 1) * 2 <= mask + 1) rehash(mask + 1, shift);
   else rehash((mask + 1) * 2, shift - 1);
  }
  put(k, v);
 }
 // 無ければ nullptr。
 const V* find(unsigned long long k) const {
  std::size_t h = home(k);
  while (t[h].k) {
   if (t[h].k == k) return &t[h].v;
   h = (h + 1) & mask;
  }
  return nullptr;
 }
 bool contains(unsigned long long k) const { return find(k) != nullptr; }
 void erase(unsigned long long k) {
  std::size_t h = home(k);
  while (t[h].k != k) h = (h + 1) & mask;
  t[h].k = TOMB, --live;
 }
};

}  // namespace dyntree_internal
