#pragma once
// eer_lean4 で、遷移を頂点ごとの幅 k の表でなく、1 つ目の子を頂点に直接持ち、2 つ目の子ができた頂点にだけ幅 k の行を割り当てる形で
// 持つ版 (yosupo-aho-corasick の ac_hybrid5 と同じ持ち方)。行の無い頂点は 0 で埋まった 0 番の行を指すので、子は分岐なしで引ける。
// 3 回目 (f068eb89 の run 38215267734、Xeon 8573C) の x64-gcc では、eer_lean3 と eer_lean4 が binary の組で 21.5 ms だった。binary の
// 組は 19 文字の abacaba 型の文字列で、回文が 53 万個でき、遷移の表は 10^6 行 x 19 文字の 76 MB になる。一方、子を持つ頂点の
// ほとんどは子が 1 つで、2 つ以上の子を持つ頂点は 1254 個だった (short_period では 1 個)。suffix link をたどる回数は 1 文字あたり
// 1.33 回と少ない。
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

namespace eer_hy {
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
// 頂点の長さと suffix link、1 つ目の子 ow、行の番号と 1 つ目の子の文字 ro (上位 24 bit が行の番号で 0 なら行が無い、下位 8 bit が文字で
// 255 なら子が無い)。行の無い頂点は 0 番の行 (0 で埋まっている) を指すので、子は分岐なしで引ける。
struct Node {
  int len, link, ow;
  unsigned ro;
};
constexpr unsigned NONE = 255;
// v に n 個分の領域を取り (中身はまだ書かない)、Linux ではその領域のページに huge page を頼む。vector の領域は 2 MB 境界から
// 始まるとは限らないので、huge page に載るのは領域の内側にある 2 MB の区切りだけになる。
template <class T> inline void reserve_huge(std::vector<T> &v, size_t n) {
  v.reserve(n);
#ifdef __linux__
  const uintptr_t b = reinterpret_cast<uintptr_t>(v.data()) & ~uintptr_t(4095), e = (reinterpret_cast<uintptr_t>(v.data() + n) + 4095) & ~uintptr_t(4095);
  if (b < e) madvise(reinterpret_cast<void *>(b), e - b, MADV_HUGEPAGE);
#endif
}
}  // namespace eer_hy

struct Solver {
  string s;
  EertreeAnswer ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    using namespace eer_hy;
    const int n = int(s.size());
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    array<unsigned char, 256> code{};
    array<bool, 256> used{};
    for (int i = 0; i < n; ++i) used[t[i]] = true;
    int k = 0;
    for (int c = 0; c < 256; ++c)
      if (used[c]) code[c] = (unsigned char)k++;
    const int cap = n + 2;
    // 頂点の列 (cap 個) と行 (0 番の行と、枝分かれする頂点の行。枝分かれする頂点は辺の数 cap の半分以下) を 1 つの領域に置く
    const size_t rows_cap = (size_t(cap) / 2 + 2) * k;
    Zeroed buf(size_t(cap) * sizeof(Node) + rows_cap * sizeof(int));
    Node *nd = static_cast<Node *>(buf.get());
    int *rows = reinterpret_cast<int *>(nd + cap);
    nd[0] = {-1, 0, 0, NONE}, nd[1] = {0, 0, 0, NONE};
    int cnt = 2;
    unsigned nrow = 1;
    auto child = [&](int u, int c) {
      const Node &e = nd[u];
      const int a = e.ow, b = rows[size_t(e.ro >> 8) * k + c];
      return (e.ro & 255) == unsigned(c) ? a : b;
    };
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
      int e = child(u, c);
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
          l = child(v, c);
        }
        nd[w] = {L, l, 0, NONE};
        ans.par.push_back(u - 1), ans.link.push_back(l - 1);
        Node &f = nd[u];
        if (f.ro == NONE) {
          f.ow = w, f.ro = unsigned(c);
        } else {
          if (!(f.ro >> 8)) rows[size_t(nrow) * k + (f.ro & 255)] = f.ow, f.ro |= nrow++ << 8;
          rows[size_t(f.ro >> 8) * k + c] = w;
        }
        e = w;
      }
      last = e;
      ans.last[i] = last - 1;
    }
  }

  const EertreeAnswer &answer() const { return ans; }
};
