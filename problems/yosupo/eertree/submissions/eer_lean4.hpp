#pragma once
// eer_lean3 で、頂点の列を vector でなく遷移の表と同じ領域 (0 で埋まった領域、2 MB 以上なら Linux では 2 MB 境界の mmap に置いて
// huge page を頼む) に置き、答えの par、link、last は領域を取ってから huge page を頼む版。2 回目 (d314fa66 の run 38211024539、
// EPYC 9V45) では eer_lean3 が 4 環境のすべてで首位で、x64-gcc の最大は binary の組の 11.55 ms だった。回文が 53 万個できる binary
// では、頂点の列 (4.2 MB)、par と link (2.1 MB ずつ)、last (4 MB) を 4 KB のページに取って初めて書いていて、確保したばかりの領域に
// 初めて書く代価は EPYC 7763 で 1 MB あたり 0.43 ms なので、合わせて 5 ms ほどになる。
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

namespace eer_lean4 {
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
// v に n 個分の領域を取り (中身はまだ書かない)、Linux ではその領域のページに huge page を頼む。vector の領域は 2 MB 境界から
// 始まるとは限らないので、huge page に載るのは領域の内側にある 2 MB の区切りだけになる。
template <class T> inline void reserve_huge(std::vector<T> &v, size_t n) {
  v.reserve(n);
#ifdef __linux__
  const uintptr_t b = reinterpret_cast<uintptr_t>(v.data()) & ~uintptr_t(4095), e = (reinterpret_cast<uintptr_t>(v.data() + n) + 4095) & ~uintptr_t(4095);
  if (b < e) madvise(reinterpret_cast<void *>(b), e - b, MADV_HUGEPAGE);
#endif
}
}  // namespace eer_lean4

struct Solver {
  string s;
  EertreeAnswer ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    using namespace eer_lean4;
    const int n = int(s.size());
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    array<unsigned char, 256> code{};
    array<bool, 256> used{};
    for (int i = 0; i < n; ++i) used[t[i]] = true;
    int k = 0;
    for (int c = 0; c < 256; ++c)
      if (used[c]) code[c] = (unsigned char)k++;
    const int cap = n + 2;
    // 頂点の列 (cap 個) と遷移の表 (cap 行、幅 k) を 1 つの領域に置く
    Zeroed buf(size_t(cap) * sizeof(Node) + size_t(cap) * k * sizeof(int));
    Node *nd = static_cast<Node *>(buf.get());
    int *nx = reinterpret_cast<int *>(nd + cap);
    nd[0] = {-1, 0}, nd[1] = {0, 0};
    int cnt = 2;
    reserve_huge(ans.par, cap - 1), reserve_huge(ans.link, cap - 1), reserve_huge(ans.last, n);
    ans.last.resize(n);
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
      int &e = nx[size_t(u) * k + c];
      if (!e) {
        const int w = cnt++, L = nd[u].len + 2;
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
        ans.par.push_back(u - 1), ans.link.push_back(l - 1);
        e = w;
      }
      last = e;
      ans.last[i] = last - 1;
    }
  }

  const EertreeAnswer &answer() const { return ans; }
};
