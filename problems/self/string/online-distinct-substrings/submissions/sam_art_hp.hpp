#pragma once
// sam_art の状態の列の置き方を変えたもの。遷移の持ち方を遷移の数で変える接尾辞オートマトン (adaptive radix tree の Node4 と Node256 の考え方)。状態は 32 byte で、
// suffix link、最長の長さ、4 本までの遷移 (文字 4 つを 1 語に、行き先 4 つ) と、大きい表の番号を持つ。5 本目の遷移が来たら、
// 26 個の欄の表を別の列に取ってそこへ移す。状態の遷移はほとんど 1〜2 本なので、26 個の欄を全部の状態に持つ形 (112 byte) より
// 表が小さい。文字を足すと、異なる部分文字列の数は len[cur] - len[link[cur]] だけ増える。
// sam_art_hp は、状態の列を 2 MB を超えるまでは vector で持ち、超えたら Linux では 64 GB の仮想の領域 (MAP_NORESERVE) に移して
// huge page を頼み、そこへ先へ伸ばす (写し直しが無い)。
#include "pj.hpp"
#include <array>
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
  struct St {
    int link, len;
    u32 keys;  // 使っていない byte は 0xff
    int to[4];
    int big;  // 26 個の欄の表の番号 (無ければ -1)
  };
  Arena<St> st;
  Arena<array<int, 26>> bigs;
  int last = 0;
  u64 total = 0;
  Solver() { st.push_back(St{-1, 0, 0xffffffffu, {0, 0, 0, 0}, -1}); }
  static int find(u32 keys, int c) {  // 一致した位置 (無ければ -1)
    const u32 x = keys ^ (0x01010101u * (u32)c);
    const u32 z = (x - 0x01010101u) & ~x & 0x80808080u;
    return z ? __builtin_ctz(z) >> 3 : -1;
  }
  int get(int v, int c) const {
    const St &s = st[v];
    if (s.big >= 0) return bigs[s.big][c];
    const int k = find(s.keys, c);
    return k >= 0 ? s.to[k] : 0;
  }
  void set(int v, int c, int t) {
    St &s = st[v];
    if (s.big >= 0) {
      bigs[s.big][c] = t;
      return;
    }
    int k = find(s.keys, c);
    if (k >= 0) {
      s.to[k] = t;
      return;
    }
    k = find(s.keys, 0xff);
    if (k >= 0) {
      s.keys = (s.keys & ~(0xffu << (8 * k))) | ((u32)c << (8 * k));
      s.to[k] = t;
      return;
    }
    array<int, 26> b{};
    for (int j = 0; j < 4; ++j) b[(s.keys >> (8 * j)) & 0xff] = s.to[j];
    b[c] = t;
    s.big = (int)bigs.size();
    bigs.push_back(b);
  }
  u64 push(char ch) {
    const int c = ch - 'a';
    const int cur = (int)st.size();
    st.push_back(St{0, st[last].len + 1, 0xffffffffu, {0, 0, 0, 0}, -1});
    int p = last;
    while (p != -1 && !get(p, c)) set(p, c, cur), p = st[p].link;
    if (p != -1) {
      const int q = get(p, c);
      if (st[p].len + 1 == st[q].len) {
        st[cur].link = q;
      } else {
        const int cl = (int)st.size();
        St x = st[q];
        x.len = st[p].len + 1;
        if (x.big >= 0) {
          const array<int, 26> b = bigs[x.big];
          x.big = (int)bigs.size();
          bigs.push_back(b);
        }
        st.push_back(x);
        while (p != -1 && get(p, c) == q) set(p, c, cl), p = st[p].link;
        st[q].link = st[cur].link = cl;
      }
    }
    last = cur;
    total += st[cur].len - st[st[cur].link].len;
    return total;
  }
};
