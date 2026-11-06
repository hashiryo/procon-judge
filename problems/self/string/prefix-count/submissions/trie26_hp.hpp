#pragma once
// trie26 の頂点の置き方を変えたもの。頂点は 128 byte (子の欄 26 個と、頂点を通る要素の数) で、Linux では仮想の領域を大きく
// (64 GB、MAP_NORESERVE) 取って huge page を頼み、頂点を足すたびに先へ伸ばす。vector で伸ばすと、写し直しと、新しい領域を 0 で
// 埋める手間と、4 KB のページを初めて触る手間が頂点の数だけかかる (長さ 10^5 の文字列 15 本で頂点 150 万個、表 190 MB)。
// 仮想の領域は触ったところだけ 0 のページが割り当てられるので、子の欄の 0 (子なし) を書かなくてよい。取れなければ vector で伸ばす。
#include "pj.hpp"
#ifdef __linux__
#include <sys/mman.h>
#endif

struct Solver {
  struct Node {
    int ch[26], cnt, pad[5];
  };
  Node *nd = nullptr;
  size_t used = 1, cap = 0, mapped = 0;
  vector<Node> fallback;
  Solver() {
#ifdef __linux__
    const size_t bytes = (size_t)1 << 36;
    void *m = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (m != MAP_FAILED) {
      madvise(m, bytes, MADV_HUGEPAGE);
      nd = (Node *)m, mapped = bytes, cap = bytes / sizeof(Node);
    }
#endif
    if (!nd) fallback.assign(1024, Node{}), nd = fallback.data(), cap = fallback.size();
  }
  ~Solver() {
#ifdef __linux__
    if (mapped) munmap(nd, mapped);
#endif
  }
  int new_node() {
    if (used == cap) fallback.resize(cap * 2, Node{}), nd = fallback.data(), cap = fallback.size();
    return (int)used++;
  }
  void add(const string &s, int d) {
    int v = 0;
    nd[0].cnt += d;
    for (unsigned char c : s) {
      int w = nd[v].ch[c - 'a'];
      if (!w) w = new_node(), nd[v].ch[c - 'a'] = w;
      nd[w].cnt += d;
      v = w;
    }
  }
  void insert(const string &s) { add(s, 1); }
  void erase(const string &s) { add(s, -1); }
  int count_prefix(const string &p) {
    int v = 0;
    for (unsigned char c : p)
      if (!(v = nd[v].ch[c - 'a'])) return 0;
    return nd[v].cnt;
  }
};
