#pragma once
// sam26 の状態の置き方を変えたもの。状態が 2 MB 分 (18724 個) になるまでは vector で持ち、超えたら Linux では 64 GB の仮想の領域
// (MAP_NORESERVE) に移して huge page を頼み、そこへ先へ伸ばす (写し直しが無く、触ったページだけ 0 で割り当てられる)。
#include "pj.hpp"
#include <cstring>
#ifdef __linux__
#include <sys/mman.h>
#endif

struct Solver {
  struct St {
    int next[26], link, len;
  };
  static constexpr size_t SMALL = (size_t(1) << 21) / sizeof(St);
  St *st;
  size_t used = 1, cap, mapped = 0;
  vector<St> vec;
  int last = 0;
  u64 total = 0;
  Solver() : vec(64) {
    st = vec.data(), cap = vec.size();
    memset(st, 0, sizeof(St) * cap);
    st[0].link = -1;
  }
  ~Solver() {
#ifdef __linux__
    if (mapped) munmap(st, mapped);
#endif
  }
  void grow() {
#ifdef __linux__
    if (!mapped && cap >= SMALL) {
      const size_t bytes = (size_t)1 << 36;
      void *m = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
      if (m != MAP_FAILED) {
        madvise(m, bytes, MADV_HUGEPAGE);
        memcpy(m, st, used * sizeof(St));
        st = (St *)m, mapped = bytes, cap = bytes / sizeof(St);
        vector<St>().swap(vec);
        return;
      }
    }
#endif
    vec.resize(cap * 2);
    st = vec.data();
    memset(st + cap, 0, sizeof(St) * cap);
    cap *= 2;
  }
  int make(int len, int link) {
    if (used == cap) grow();
    St &v = st[used];
    v.link = link, v.len = len;
    return (int)used++;
  }
  u64 push(char ch) {
    const int c = ch - 'a';
    const int cur = make(st[last].len + 1, 0);
    int p = last;
    while (p != -1 && !st[p].next[c]) st[p].next[c] = cur, p = st[p].link;
    if (p != -1) {
      const int q = st[p].next[c];
      if (st[p].len + 1 == st[q].len) {
        st[cur].link = q;
      } else {
        const int cl = make(st[p].len + 1, st[q].link);
        memcpy(st[cl].next, st[q].next, sizeof(st[q].next));
        while (p != -1 && st[p].next[c] == q) st[p].next[c] = cl, p = st[p].link;
        st[q].link = st[cur].link = cl;
      }
    }
    last = cur;
    total += st[cur].len - st[st[cur].link].len;
    return total;
  }
};
