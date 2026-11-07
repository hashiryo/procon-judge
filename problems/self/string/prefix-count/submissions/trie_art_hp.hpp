#pragma once
// trie_art の頂点の列の置き方を変えたもの。頂点の大きさを子の数で変えるトライ (adaptive radix tree の Node4 と Node256 の考え方)。子が 4 つまでの頂点は 32 byte
// (文字 4 つを 1 語に、子 4 つ、通る要素の数) で、子の文字は語の中の byte の一致で探す。5 つ目の子が来たら 26 個の子の欄を
// 持つ 128 byte の頂点に替え、親の欄を書き換える。根は初めから 26 個の欄。頂点の参照は、正なら小さい頂点、負なら大きい頂点の
// 番号で、0 は子なし。深い頂点はほとんど子が 1 つなので、trie26 より表が小さくなる。trie_art_hp は、頂点の列を 2 MB を超えるまでは
// vector で持ち、超えたら Linux では 64 GB の仮想の領域 (MAP_NORESERVE) に移して huge page を頼み、写し直さずに先へ伸ばす
// (trie_art は huge page の vector を倍々に伸ばして写し直す)。
#include "pj.hpp"
#include <cstring>
#ifdef __linux__
#include <sys/mman.h>
#endif

// 先へ伸ばすだけの列。2 MB を超えたら大きい仮想の領域に移す。
template <class T> struct Arena {
  T *p;
  size_t n = 0, cap, mapped = 0;
  vector<T> vec;
  Arena() : vec(64) { p = vec.data(), cap = vec.size(); }
  ~Arena() {
#ifdef __linux__
    if (mapped) munmap(p, mapped);
#endif
  }
  Arena(const Arena &) = delete;
  Arena &operator=(const Arena &) = delete;
  void grow() {
#ifdef __linux__
    if (!mapped && cap * sizeof(T) >= (size_t(1) << 21)) {
      const size_t bytes = (size_t)1 << 36;
      void *m = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
      if (m != MAP_FAILED) {
        madvise(m, bytes, MADV_HUGEPAGE);
        memcpy(m, p, n * sizeof(T));
        p = (T *)m, mapped = bytes, cap = bytes / sizeof(T);
        vector<T>().swap(vec);
        return;
      }
    }
#endif
    vec.resize(cap * 2);
    p = vec.data(), cap = vec.size();
  }
  void push_back(const T &x) {
    if (n == cap) grow();
    p[n++] = x;
  }
  size_t size() const { return n; }
  T &operator[](size_t i) { return p[i]; }
  const T &operator[](size_t i) const { return p[i]; }
};


struct Solver {
  struct Small {
    u32 keys;  // 使っていない byte は 0xff (英小文字と一致しない)
    int cnt, ch[4];
    u32 num, pad;
  };
  struct Big {
    int ch[26], cnt, pad[5];
  };
  Arena<Small> sm;
  Arena<Big> bg;
  Solver() {
    sm.push_back(Small{});
    bg.push_back(Big{});
  }
  // 参照 r の頂点の数
  int &cnt(int r) { return r > 0 ? sm[r].cnt : bg[-r - 1].cnt; }
  // 頂点 r の文字 c の子 (なければ 0)
  int child(int r, int c) const {
    if (r < 0) return bg[-r - 1].ch[c];
    const Small &v = sm[r];
    const u32 x = v.keys ^ (0x01010101u * (u32)c);
    const u32 z = (x - 0x01010101u) & ~x & 0x80808080u;  // 一致した byte の最上位 bit
    return z ? v.ch[__builtin_ctz(z) >> 3] : 0;
  }
  // 頂点 r に文字 c の子を足して、その参照を返す。r が 4 つの子を持つ小さい頂点なら大きい頂点に替え、r と、r を指す親の欄
  // (親の参照 pr と欄の番号 pk) を書き換える。vector が伸びると場所が変わるので、欄はポインタでなく番号で持つ。
  int add_child(int &r, int c, int pr, int pk) {
    const int w = (int)sm.size();
    sm.push_back(Small{0xffffffffu, 0, {0, 0, 0, 0}, 0, 0});
    if (r < 0) return bg[-r - 1].ch[c] = w;
    Small &v = sm[r];
    if (v.num < 4) {
      v.keys = (v.keys & ~(0xffu << (8 * v.num))) | ((u32)c << (8 * v.num));
      v.ch[v.num++] = w;
      return w;
    }
    Big b{};
    for (int k = 0; k < 4; ++k) b.ch[(v.keys >> (8 * k)) & 0xff] = v.ch[k];
    b.cnt = v.cnt, b.ch[c] = w;
    bg.push_back(b);
    r = -(int)bg.size();
    (pr < 0 ? bg[-pr - 1].ch[pk] : sm[pr].ch[pk]) = r;
    return w;
  }
  void add(const string &s, int d) {
    int r = -1, pr = 0, pk = 0;
    bg[0].cnt += d;
    for (unsigned char ch : s) {
      const int c = ch - 'a';
      int w = child(r, c);
      if (!w) w = add_child(r, c, pr, pk);
      pr = r;
      if (r < 0) pk = c;
      else {
        const Small &v = sm[r];
        pk = 0;
        while (v.ch[pk] != w) ++pk;
      }
      r = w;
      cnt(r) += d;
    }
  }
  void insert(const string &s) { add(s, 1); }
  void erase(const string &s) { add(s, -1); }
  int count_prefix(const string &p) {
    int r = -1;
    for (unsigned char ch : p)
      if (!(r = child(r, ch - 'a'))) return 0;
    return cnt(r);
  }
};
