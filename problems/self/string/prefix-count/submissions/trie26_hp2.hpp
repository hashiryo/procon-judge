#pragma once
// trie26_hp で、頂点が 2 MB 分 (16384 個) になるまでは vector で持ち、超えたら Linux では 64 GB の仮想の領域 (MAP_NORESERVE) に
// 移して huge page を頼むもの。trie26_hp は作るたびに大きい領域を取って 2 MB のページを 0 で埋めるので、小さいトライを 1000 個
// 作るケースで 63 ms かかっていた。頂点は 128 byte (子の欄 26 個と、頂点を通る要素の数)。
#include "pj.hpp"
#include <cstring>
#ifdef __linux__
#include <sys/mman.h>
#endif

struct Solver {
  struct Node {
    int ch[26], cnt, pad[5];
  };
  static constexpr size_t SMALL = 16384;
  Node *nd;
  size_t used = 1, cap, mapped = 0;
  vector<Node> vec;
  Solver() : vec(64, Node{}) { nd = vec.data(), cap = vec.size(); }
  ~Solver() {
#ifdef __linux__
    if (mapped) munmap(nd, mapped);
#endif
  }
  void grow() {
#ifdef __linux__
    if (!mapped && cap >= SMALL) {
      const size_t bytes = (size_t)1 << 36;
      void *m = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
      if (m != MAP_FAILED) {
        madvise(m, bytes, MADV_HUGEPAGE);
        memcpy(m, nd, used * sizeof(Node));
        nd = (Node *)m, mapped = bytes, cap = bytes / sizeof(Node);
        vector<Node>().swap(vec);
        return;
      }
    }
#endif
    vec.resize(cap * 2, Node{});
    nd = vec.data(), cap = vec.size();
  }
  void add(const string &s, int d) {
    int v = 0;
    nd[0].cnt += d;
    for (unsigned char c : s) {
      int w = nd[v].ch[c - 'a'];
      if (!w) {
        if (used == cap) grow();
        w = (int)used++;
        nd[v].ch[c - 'a'] = w;
      }
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
