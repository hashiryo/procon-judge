#pragma once
// NeoLibrary の suffix_array (lib-neo と同じ中身) を、2 MB 境界に取って huge page を頼んだ作業用の領域で解き、最後に出力の配列へ写す
// 版。最後の induced sorting と 1 回目の induced sorting は、sa の多くの位置へばらばらに書き、s をばらばらに読む。4 KB のページでは
// sa (最大 2 MB) だけで 489 枚あり、x64 の L1 の TLB (64 から 96 枚) を外す。前回の段ごとの時間で、最後の induced sorting だけが
// 手元の M2 (16 KB のページで TLB が大きい) に比べて x64 で遅かったので、TLB の外れを疑う。作業用の領域を用意する分と、写す分
// (2 MB) の手間が増える。出力の配列は lib-neo と同じく、ふつうの vector で 4 KB のページのまま。s は文字列のまま読む (neo_hp_s が
// s も写す版)。確保の仕方は self-sort-u64 の lsd11_hp と同じ。
// 接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"

namespace neo_hp {
struct FreeDeleter {
  void operator()(void *p) const { std::free(p); }
};
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
inline std::unique_ptr<int, FreeDeleter> alloc_huge(size_t bytes) {
  constexpr size_t H = size_t(1) << 21;
  bytes = (std::max<size_t>(bytes, 1) + H - 1) & ~(H - 1);
  void *p = std::aligned_alloc(H, bytes);
#ifdef __linux__
  madvise(p, bytes, MADV_HUGEPAGE);
  madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
  return std::unique_ptr<int, FreeDeleter>(static_cast<int *>(p));
}
}  // namespace neo_hp

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    auto w = neo_hp::alloc_huge(size_t(n) * sizeof(int));
    suffix_array_internal::sa_is(reinterpret_cast<const unsigned char *>(s.data()), n, 256, w.get());
    sa.assign(w.get(), w.get() + n);
  }

  const vector<int> &answer() const { return sa; }
};
