#pragma once
// 新しく確保した領域に初めて触る代価を CI で測る診断用の中身。run() では 4 MB の領域を 1 で埋めるだけで、run の列挙は answer() で
// lyndon_lean と同じ計算をするので、出力は正しいまま。MODE 0 は run() の中で普通に確保して埋める (4 KB のページごとにフォールト)、
// MODE 1 は run() の中で 2 MB 境界に寄せた領域に huge page を頼んでから埋める (Linux のみ)、MODE 2 は構築で確保して触っておいた領域を
// run() で埋め直す (フォールトのない書き込みだけの代価)。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "_lyndon_lean.hpp"

template <int MODE> struct DiagTouchSolver {
  static constexpr std::size_t BYTES = std::size_t(4) << 20, H = std::size_t(2) << 20;
  std::string s;
  std::unique_ptr<char[]> raw;
  char *p = nullptr;
  mutable std::vector<std::array<int, 3>> res;
  explicit DiagTouchSolver(const std::string &s) : s(s) {
    if (MODE == 2) {
      raw.reset(new char[BYTES]), p = raw.get();
      std::memset(p, 0, BYTES);
      asm volatile("" : : "r"(p) : "memory");
    }
  }
  void run() {
    if (MODE == 0) raw.reset(new char[BYTES]), p = raw.get();
    if (MODE == 1) {
      raw.reset(new char[BYTES + H]);
      p = reinterpret_cast<char *>((reinterpret_cast<std::uintptr_t>(raw.get()) + H - 1) & ~std::uintptr_t(H - 1));
#ifdef __linux__
      madvise(p, BYTES, MADV_HUGEPAGE);
#endif
    }
    std::memset(p, 1, BYTES);
    asm volatile("" : : "r"(p) : "memory");
  }
  const std::vector<std::array<int, 3>> &answer() const {
    if (res.empty()) res = lyndon_lean::runs<false>(s);
    return res;
  }
};
