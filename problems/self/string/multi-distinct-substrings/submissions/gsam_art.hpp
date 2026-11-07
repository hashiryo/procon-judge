#pragma once
// 広義の接尾辞オートマトン。文字列ごとに根から足し始め、足す先の状態が既にその文字の遷移を持つときは、新しい状態を作らずに、長さが
// 合えばその先へ進み、合わなければ複製を作ってそこへ進む。状態の持ち方は NeoLibrary の SuffixAutomaton と同じ (4 本までの遷移を
// 32 byte に詰め、5 本目で 26 個の欄の表に移す。列は huge page を頼む vector)。答えは状態ごとの len[v] - len[link[v]] の和で、状態を
// 作るたびに足す (複製は区間を分けるだけなので和を変えない)。
#include "pj.hpp"
#include <array>
#include "_shared/string/_hugealloc.hpp"

struct Solver {
  struct St {
    int link, len;
    u32 keys;
    int to[4];
    int big;
  };
  const vector<string> &ss;
  string_hp::Vec<St> st;
  string_hp::Vec<array<int, 26>> bigs;
  u64 total = 0;
  explicit Solver(const vector<string> &ss) : ss(ss) {}
  static int find(u32 keys, int c) {
    const u32 x = keys ^ (0x01010101u * (u32)c), z = (x - 0x01010101u) & ~x & 0x80808080u;
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
    if (k < 0) k = find(s.keys, 0xff);
    if (k >= 0) {
      s.keys = (s.keys & ~(0xffu << (8 * k))) | ((u32)c << (8 * k)), s.to[k] = t;
      return;
    }
    array<int, 26> b{};
    for (int j = 0; j < 4; ++j) b[(s.keys >> (8 * j)) & 0xff] = s.to[j];
    b[c] = t;
    s.big = (int)bigs.size();
    bigs.push_back(b);
  }
  // 状態 q を長さ l の複製に分け、p から suffix link をたどって q へ c で移る遷移を複製へ向ける。
  int split(int p, int c, int q, int l) {
    const int cl = (int)st.size();
    St x = st[q];
    x.len = l;
    if (x.big >= 0) {
      const array<int, 26> b = bigs[x.big];
      x.big = (int)bigs.size();
      bigs.push_back(b);
    }
    st.push_back(x);
    while (p != -1 && get(p, c) == q) set(p, c, cl), p = st[p].link;
    st[q].link = cl;
    return cl;
  }
  int extend(int last, int c) {
    if (const int q = get(last, c)) return st[last].len + 1 == st[q].len ? q : split(last, c, q, st[last].len + 1);
    const int cur = (int)st.size();
    st.push_back(St{0, st[last].len + 1, 0xffffffffu, {0, 0, 0, 0}, -1});
    int p = last;
    while (p != -1 && !get(p, c)) set(p, c, cur), p = st[p].link;
    if (p != -1) {
      const int q = get(p, c);
      st[cur].link = st[p].len + 1 == st[q].len ? q : split(p, c, q, st[p].len + 1);
    }
    total += st[cur].len - st[st[cur].link].len;
    return cur;
  }
  void run() {
    st.clear(), bigs.clear(), total = 0;
    st.push_back(St{-1, 0, 0xffffffffu, {0, 0, 0, 0}, -1});
    for (const string &s : ss) {
      int last = 0;
      for (unsigned char ch : s) last = extend(last, ch - 'a');
    }
  }
  u64 answer() const { return total; }
};
