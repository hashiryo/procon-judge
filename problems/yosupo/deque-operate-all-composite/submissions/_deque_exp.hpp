#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <new>
#include <type_traits>
#ifdef __linux__
#include <sys/mman.h>
#endif

// NeoLibrary ddd9aa3 の neo/data_structure/FoldableDeque.hpp の写しで、Intel の 4 機種で swag より 2 割遅い理由を切り分ける
// ため、配列の取り方を替えられるようにしたもの。遅いのは列が大きく伸びるケース (max_random、random、large_max) だけで、
// 写し元は計測区間の中で配列を伸ばす。伸ばすときは倍の大きさの vector を作って全部を T() で埋め、生きている要素を移す。
// この写しは配列を生のポインタで持ち (要素は自明に写せる型に限る)、次の 2 つを選べる。
//   Fill: 真なら写し元と同じく、新しい配列を全部 T() で埋めてから移す。偽なら埋めずに、生きている要素だけを写す。
//   Huge: 真なら写し元と同じく、1 MB 以上の配列を 2 MB 境界に置いて huge page を頼む。偽なら 64 byte 境界の operator new
//         だけにする (4 KB のページ)。
// reserve(c) は、空のときに容量 c の配列を取って真ん中から使う (写し元には無い)。
template <class M, bool Fill, bool Huge> class FoldableDequeExp {
  using T = typename M::T;
  struct Node {
    T x, s;
  };
  static_assert(std::is_trivially_copyable_v<Node>);
  Node *a = nullptr;
  int cap = 0, hd = 0, md = 0, tl = 0;
  static bool huge(size_t want) { return Huge && want >= (size_t(1) << 20); }
  static Node *alloc(int c) {
    const size_t want = size_t(c) * sizeof(Node);
    Node *p;
    if (huge(want)) {
      const size_t H = size_t(1) << 21, bytes = (want + H - 1) & ~(H - 1);
      p = static_cast<Node *>(::operator new(bytes, std::align_val_t(H)));
#ifdef __linux__
      madvise(p, bytes, MADV_HUGEPAGE);
#endif
    } else {
      p = static_cast<Node *>(::operator new(want, std::align_val_t(64)));
    }
    // 写し元の vector(c) と同じく、全部を値で埋める。
    if constexpr (Fill) std::uninitialized_value_construct_n(p, c);
    return p;
  }
  static void release(Node *p, int c) {
    if (!p) return;
    ::operator delete(p, std::align_val_t(huge(size_t(c) * sizeof(Node)) ? size_t(1) << 21 : 64));
  }
  // 境を m に移し、前と後ろの積を数え直す。
  void rebuild(int m) {
    md = m;
    if (hd < md) {
      a[md - 1].s = a[md - 1].x;
      for (int i = md - 1; --i >= hd;) a[i].s = M::op(a[i].x, a[i + 1].s);
    }
    if (md < tl) {
      a[md].s = a[md].x;
      for (int i = md + 1; i < tl; ++i) a[i].s = M::op(a[i - 1].s, a[i].x);
    }
  }
  // 端に空きが無いとき。生きている要素が半分以下なら同じ配列の真ん中へ、そうでなければ倍の配列の真ん中へ移す。
  void recenter() {
    const int k = tl - hd, c = cap, nc = c && 2 * k <= c ? c : std::max(16, 2 * c), nh = (nc - k) / 2;
    if (nc == c) {
      std::memmove(a + nh, a + hd, sizeof(Node) * k);
    } else {
      Node *b = alloc(nc);
      if (k) std::memcpy(b + nh, a + hd, sizeof(Node) * k);
      release(a, c);
      a = b, cap = nc;
    }
    md += nh - hd, tl += nh - hd, hd = nh;
  }

 public:
  FoldableDequeExp() = default;
  FoldableDequeExp(const FoldableDequeExp &) = delete;
  FoldableDequeExp &operator=(const FoldableDequeExp &) = delete;
  ~FoldableDequeExp() { release(a, cap); }
  void reserve(int c) {
    assert(hd == tl);
    if (c <= cap) return;
    release(a, cap);
    a = alloc(c), cap = c, hd = md = tl = c / 2;
  }
  void push_front(const T &x) {
    if (hd == 0) recenter();
    --hd;
    a[hd].x = x, a[hd].s = hd + 1 == md ? x : M::op(x, a[hd + 1].s);
  }
  void push_back(const T &x) {
    if (tl == cap) recenter();
    a[tl].x = x, a[tl].s = tl == md ? x : M::op(a[tl - 1].s, x);
    ++tl;
  }
  void pop_front() {
    assert(hd < tl);
    if (hd == md) rebuild(hd + (tl - hd + 1) / 2);
    ++hd;
  }
  void pop_back() {
    assert(hd < tl);
    if (md == tl) rebuild(hd + (tl - hd) / 2);
    --tl;
  }
  // op(先頭, ..., 末尾)。空なら ti()。
  T all_prod() const {
    if (hd == md) return md == tl ? M::ti() : a[tl - 1].s;
    return md == tl ? a[hd].s : M::op(a[hd].s, a[tl - 1].s);
  }
};
