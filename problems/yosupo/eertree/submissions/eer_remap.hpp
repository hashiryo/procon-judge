#pragma once
// eer_basic で、使われている文字だけに 0 から番号を振り直して遷移の表の幅を種類の数 k にし (1 文字の列なら 1 頂点 4 バイト)、各頂点の
// 長さと suffix link を隣り合わせに置き、出力の親と suffix link は頂点を作るときに直接書く版。遷移の表は eer_basic と同じく、
// 2 MB 以上なら 2 MB 境界に寄せて madvise(MADV_HUGEPAGE) を呼んだ領域に置く。
// 回文木の実装を比べた記録は algo-notes の notes/eertree.md にある。
#include <array>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "pj.hpp"

namespace eer_remap {
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
struct Node {
  int len, link;
};
}  // namespace eer_remap

struct Solver {
  string s;
  EertreeAnswer ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    using namespace eer_remap;
    const int n = int(s.size());
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    array<unsigned char, 256> code{};
    array<bool, 256> used{};
    for (int i = 0; i < n; ++i) used[t[i]] = true;
    int k = 0;
    for (int c = 0; c < 256; ++c)
      if (used[c]) code[c] = (unsigned char)k++;
    const int cap = n + 2;
    Zeroed tab(size_t(cap) * k * sizeof(int));
    int *nx = static_cast<int *>(tab.get());
    vector<Node> nd(cap);
    nd[0] = {-1, 0}, nd[1] = {0, 0};
    ans.par.resize(cap - 1), ans.link.resize(cap - 1), ans.last.resize(n);
    int m = 2, last = 1;
    for (int i = 0; i < n; ++i) {
      const unsigned char x = t[i];
      const int c = code[x];
      int u = last;
      while (true) {
        const int p = i - nd[u].len - 1;
        if (p >= 0 && t[p] == x) break;
        u = nd[u].link;
      }
      int &e = nx[size_t(u) * k + c];
      if (!e) {
        const int w = m++, L = nd[u].len + 2;
        int l = 1;
        if (L > 1) {
          int v = nd[u].link;
          while (true) {
            const int p = i - nd[v].len - 1;
            if (p >= 0 && t[p] == x) break;
            v = nd[v].link;
          }
          l = nx[size_t(v) * k + c];
        }
        nd[w] = {L, l};
        ans.par[w - 1] = u - 1, ans.link[w - 1] = l - 1;
        e = w;
      }
      last = e;
      ans.last[i] = last - 1;
    }
    ans.par.resize(m - 1), ans.link.resize(m - 1);
  }

  const EertreeAnswer &answer() const { return ans; }
};
