#pragma once
// 頂点の大きさを子の数で変えるトライ (adaptive radix tree の Node4 と Node256 の考え方)。子が 4 つまでの頂点は 32 byte
// (文字 4 つを 1 語に、子 4 つ、通る要素の数) で、子の文字は語の中の byte の一致で探す。5 つ目の子が来たら 26 個の子の欄を
// 持つ 128 byte の頂点に替え、親の欄を書き換える。根は初めから 26 個の欄。頂点の参照は、正なら小さい頂点、負なら大きい頂点の
// 番号で、0 は子なし。深い頂点はほとんど子が 1 つなので、trie26 より表が小さくなる。表は huge page の vector に置く。
#include "pj.hpp"
#include <cstring>
#include "_shared/string/_hugealloc.hpp"

struct Solver {
  struct Small {
    u32 keys;  // 使っていない byte は 0xff (英小文字と一致しない)
    int cnt, ch[4];
    u32 num, pad;
  };
  struct Big {
    int ch[26], cnt, pad[5];
  };
  string_hp::Vec<Small> sm;
  string_hp::Vec<Big> bg;
  Solver() : sm(1), bg(1) { bg[0] = Big{}; }
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
