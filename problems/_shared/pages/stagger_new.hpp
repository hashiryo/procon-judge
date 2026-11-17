#pragma once
// 確かめ用。整列付きの operator new と delete を差し替えて、2 MB 以上の整列を求める確保 (NeoLibrary の 1 MB 以上の配列) の
// 先頭を、取るたびに 2 KB と 64 byte ずつずらす (k 回目の確保は k mod 16 倍)。同じ添字で引く配列がどれも 2 MB 境界から
// 始まると、要素が同じ大きさなら同じ添字の要素の番地の下位 21 bit がそろう。Xeon 6975P-C では汎用の遅延セグ木がこれで
// 2 割遅かった (self-segtree-range-assign-range-sum の neo-generic-stagger)。速い核 (AddSum、AddMin、Assign、Affine) も
// 値の配列と塊の配列を別々に 2 MB 境界に置くので、ずらすと速くなるかを見る。huge page を頼むのはここで 2 MB 境界の領域に
// 対して行う (呼んだ側がずれた番地に頼む madvise は境界から外れて失敗するが、害はない)。
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace stagger_internal {
constexpr std::size_t H = std::size_t(1) << 21, STEP = 2048 + 64, K = 16;
inline unsigned cnt = 0;
}  // namespace stagger_internal
void *operator new(std::size_t n, std::align_val_t al) {
  using namespace stagger_internal;
  const std::size_t a = std::size_t(al);
  if (a >= H) {
    const std::size_t off = (cnt++ % K) * STEP, bytes = (n + off + H - 1) & ~(H - 1);
    if (void *p = std::aligned_alloc(H, bytes)) {
#ifdef __linux__
      madvise(p, bytes, MADV_HUGEPAGE);
#endif
      return static_cast<char *>(p) + off;
    }
    throw std::bad_alloc();
  }
  if (void *p = std::aligned_alloc(a, (n + a - 1) / a * a)) return p;
  throw std::bad_alloc();
}
void operator delete(void *p, std::align_val_t al) noexcept {
  using namespace stagger_internal;
  // ずらした分は 2 MB 未満なので、2 MB 境界へ切り下げれば取った先頭に戻る。
  if (std::size_t(al) >= H) p = reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(p) & ~std::uintptr_t(H - 1));
  std::free(p);
}
void operator delete(void *p, std::size_t, std::align_val_t al) noexcept { operator delete(p, al); }
