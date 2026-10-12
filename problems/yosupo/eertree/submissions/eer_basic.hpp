#pragma once
// 回文木 (Rubinchik と Shur の eertree、2015) を左から作る素直な形。各頂点に 26 個の遷移の表を持つ。中の番号は 0 が ODD (長さ -1)、
// 1 が EVEN (長さ 0)、2 から回文で、出力の番号は中の番号から 1 を引いたもの。位置 i の文字 c について、直前の最長の回文の接尾辞
// から suffix link をたどり、s[i - len - 1] = c となる回文 u を見つけ (ODD では必ず止まる)、u の c の遷移が無ければ頂点を作る。
// 新しい頂点の suffix link は、長さ 1 なら EVEN、そうでなければ u の suffix link から同じようにたどった回文の c の遷移。たどる回数は
// 全体で 2n 以下。遷移の表は 10^6 頂点で 104 MB になるので、2 MB 境界に寄せた領域に madvise(MADV_HUGEPAGE) を呼んで置く (Linux のみ)。
// 回文木の実装を比べた記録は algo-notes の notes/eertree.md にある。
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "pj.hpp"

namespace eer_basic {
// bytes byte の 0 で埋まった領域。2 MB 以上なら、Linux では 2 MB 境界の mmap に置いて huge page を頼む。
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
}  // namespace eer_basic

struct Solver {
  string s;
  EertreeAnswer ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    const int cap = n + 2;
    eer_basic::Zeroed tab(size_t(cap) * 26 * sizeof(int));
    int(*nx)[26] = static_cast<int(*)[26]>(tab.get());
    vector<int> len(cap), link(cap), par(cap);
    len[0] = -1, link[0] = 0, len[1] = 0, link[1] = 0;
    int m = 2, last = 1;
    ans.last.resize(n);
    auto fits = [&](int u, int i, int c) { return i - len[u] - 1 >= 0 && t[i - len[u] - 1] == c; };
    for (int i = 0; i < n; ++i) {
      const int c = t[i] - 'a';
      int u = last;
      while (!fits(u, i, t[i])) u = link[u];
      if (!nx[u][c]) {
        const int w = m++;
        len[w] = len[u] + 2, par[w] = u;
        if (len[w] == 1) {
          link[w] = 1;
        } else {
          int v = link[u];
          while (!fits(v, i, t[i])) v = link[v];
          link[w] = nx[v][c];
        }
        nx[u][c] = w;
      }
      last = nx[u][c];
      ans.last[i] = last - 1;
    }
    ans.par.assign(m - 1, 0), ans.link.assign(m - 1, 0);
    for (int w = 2; w < m; ++w) ans.par[w - 1] = par[w] - 1, ans.link[w - 1] = link[w] - 1;
  }

  const EertreeAnswer &answer() const { return ans; }
};
