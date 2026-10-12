#pragma once
// ac_remap と ac_goto が共有する中身。0 で埋まった領域 (2 MB 以上なら Linux では 2 MB 境界の mmap に置いて huge page を頼む) と、
// 使われている文字だけに 0 から番号を振り直す表。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <array>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif

namespace ac_common {
class Zeroed {
  void *p = nullptr;
#ifdef __linux__
  size_t mapped = 0;
#endif

 public:
  explicit Zeroed(size_t bytes) {
#ifdef __linux__
    constexpr size_t H = size_t(1) << 21;
    if (bytes >= H) {
      const size_t b = (bytes + 4095) & ~size_t(4095);
      void *m = mmap(nullptr, b + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (m != MAP_FAILED) {
        char *q = static_cast<char *>(m), *a = reinterpret_cast<char *>((reinterpret_cast<uintptr_t>(q) + H - 1) & ~(H - 1));
        if (a != q) munmap(q, a - q);
        munmap(a + b, q + H - a);
        madvise(a, b, MADV_HUGEPAGE);
        p = a, mapped = b;
        return;
      }
    }
#endif
    p = calloc(bytes ? bytes : 1, 1);
  }
  ~Zeroed() {
#ifdef __linux__
    if (mapped) {
      munmap(p, mapped);
      return;
    }
#endif
    free(p);
  }
  Zeroed(const Zeroed &) = delete;
  Zeroed &operator=(const Zeroed &) = delete;
  void *get() const { return p; }
};
// 文字列の列に現れる byte に、小さい順に 0 から番号を振る。返すのは種類の数。
inline int remap(const std::vector<std::string> &ss, std::array<unsigned char, 256> &code) {
  std::array<bool, 256> used{};
  for (const auto &s : ss)
    for (const char c : s) used[(unsigned char)c] = true;
  int k = 0;
  for (int c = 0; c < 256; ++c) code[c] = used[c] ? (unsigned char)k++ : 0;
  return k;
}
}  // namespace ac_common
