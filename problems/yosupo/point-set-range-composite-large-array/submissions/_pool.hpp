#pragma once
#include "pj.hpp"
#include <cstdint>
#include <new>
#ifdef __linux__
#include <sys/mman.h>
#endif

// 節の置き場に使う vector の確保。1 MB 以上は 2 MB 境界に寄せて huge page を頼む (Linux)。
// NeoLibrary の segment_tree_internal::Alloc と同じ形。
template <class T> struct HugeAlloc {
  using value_type = T;
  HugeAlloc() = default;
  template <class U> HugeAlloc(const HugeAlloc<U> &) {}
  T *allocate(size_t n) {
    const size_t want = n * sizeof(T);
    if (want < (size_t(1) << 20)) return static_cast<T *>(::operator new(want, align_val_t(64)));
    const size_t H = size_t(1) << 21, bytes = (want + H - 1) & ~(H - 1);
    T *p = static_cast<T *>(::operator new(bytes, align_val_t(H)));
#ifdef __linux__
    madvise(p, bytes, MADV_HUGEPAGE);
#endif
    return p;
  }
  void deallocate(T *p, size_t n) {
    if (n * sizeof(T) < (size_t(1) << 20)) ::operator delete(p, align_val_t(64));
    else ::operator delete(p, align_val_t(size_t(1) << 21));
  }
  template <class U> bool operator==(const HugeAlloc<U> &) const { return true; }
};
template <class T> using PoolVec = vector<T, HugeAlloc<T>>;
