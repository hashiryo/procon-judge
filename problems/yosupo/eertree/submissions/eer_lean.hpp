#pragma once
// eer_remap で、確保した領域に触る量を、できた頂点の数に比べた分だけにした版。頂点の長さと suffix link の列、出力の親と suffix link の
// 列は、上限の n + 2 個分の容量だけを先に取り (0 で埋めない)、頂点を作るたびに足す。一様な文字列 (26 文字) では頂点が 3000 個
// ほどしかないのに、eer_remap は 0 で埋めた 8 MB の出力の列に触っていた。遷移の表の幅は種類の数を 2 の冪に切り上げ、添字を
// 掛け算でなくシフトで求める。
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

namespace eer_lean {
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
}  // namespace eer_lean

struct Solver {
  string s;
  EertreeAnswer ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    using namespace eer_lean;
    const int n = int(s.size());
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    array<unsigned char, 256> code{};
    array<bool, 256> used{};
    for (int i = 0; i < n; ++i) used[t[i]] = true;
    int k = 0;
    for (int c = 0; c < 256; ++c)
      if (used[c]) code[c] = (unsigned char)k++;
    int sh = 0;
    while ((1 << sh) < k) ++sh;
    const int cap = n + 2;
    Zeroed tab((size_t(cap) << sh) * sizeof(int));
    int *nx = static_cast<int *>(tab.get());
    vector<Node> nd;
    nd.reserve(cap);
    nd.push_back({-1, 0}), nd.push_back({0, 0});
    ans.par.reserve(cap - 1), ans.link.reserve(cap - 1), ans.last.resize(n);
    ans.par.push_back(0), ans.link.push_back(0);  // 0 番は使わない
    int last = 1;
    for (int i = 0; i < n; ++i) {
      const unsigned char x = t[i];
      const int c = code[x];
      int u = last;
      while (true) {
        const int p = i - nd[u].len - 1;
        if (p >= 0 && t[p] == x) break;
        u = nd[u].link;
      }
      int &e = nx[(size_t(u) << sh) + c];
      if (!e) {
        const int w = int(nd.size()), L = nd[u].len + 2;
        int l = 1;
        if (L > 1) {
          int v = nd[u].link;
          while (true) {
            const int p = i - nd[v].len - 1;
            if (p >= 0 && t[p] == x) break;
            v = nd[v].link;
          }
          l = nx[(size_t(v) << sh) + c];
        }
        nd.push_back({L, l});
        ans.par.push_back(u - 1), ans.link.push_back(l - 1);
        e = w;
      }
      last = e;
      ans.last[i] = last - 1;
    }
  }

  const EertreeAnswer &answer() const { return ans; }
};
