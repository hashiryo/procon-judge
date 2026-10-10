#pragma once
// NeoLibrary の suffix_array (lib-neo と同じ中身) で、出力の配列のページを MADV_POPULATE_WRITE でまとめて用意させてから解く版。
// lib-neo は vector を 0 で埋めるときに 4 KB のページごと (最大 489 回) にページフォールトを起こす。この版は reserve で確保だけして、
// 触る前に領域のページを 1 回の madvise で用意させ、それから 0 で埋める。ページの大きさは 4 KB のままなので、TLB の外れは変わらない。
// 同じ回の diag_neo_p0 (出力の配列の確保を計測区間の外に出した版) と比べて、ページフォールトの手間のうちどれだけを減らせるかを見る。
// 接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <cstdint>
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const size_t n = s.size();
    sa.reserve(n);
#ifdef __linux__
    const uintptr_t b = (uintptr_t(sa.data()) + 4095) & ~uintptr_t(4095), e = uintptr_t(sa.data() + n) & ~uintptr_t(4095);
    if (b < e) madvise(reinterpret_cast<void *>(b), e - b, 23);  // MADV_POPULATE_WRITE
#endif
    sa.resize(n);
    suffix_array_internal::sa_is(reinterpret_cast<const unsigned char *>(s.data()), int(n), 256, sa.data());
  }

  const vector<int> &answer() const { return sa; }
};
