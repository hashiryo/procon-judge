#pragma once
// ac_hybrid5 の中身 (説明は ac_hybrid5.hpp)。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <array>
#include <string>
#include <vector>
#include "_ac_common.hpp"

namespace ac_hybrid5_body {
// 頂点の 1 つ目の子 ow とその文字 oc、行の先頭 rb (0 なら行が無く、rows[0, k) の 0 の行を指す)、作業用の suffix link lk。
struct Node {
  int ow, rb, lk;
  unsigned char oc, pad[3];
};
inline void solve(const std::vector<std::string> &ss, const ac_common::Coded &cd, AhoCorasickAnswer &ans) {
  using namespace std;
  const auto &code = cd.code;
  const int k = cd.k;
  const size_t cap = cd.total + 1;
  const size_t rows_cap = (ss.size() + 3) * size_t(k);  // 0 の行、根の行、枝分かれする頂点 (文字列の本数未満) の行
  // 頂点 (cap 個)、行 (rows_cap 個)、幅優先の待ち行列 q (cap 個) を、huge page を頼んだ 1 つの領域に置く
  ac_common::Zeroed buf(cap * sizeof(Node) + (rows_cap + cap) * sizeof(int));
  Node *nd = static_cast<Node *>(buf.get());
  int *rows = reinterpret_cast<int *>(nd + cap), *q = rows + rows_cap;
  vector<int> &par = ans.par, &link = ans.link, &word = ans.word;
  ac_common::reserve_huge(par, cap), ac_common::reserve_huge(word, ss.size());
  par.resize(cap), word.resize(ss.size());
  constexpr unsigned char NONE = 255;
  nd[0].oc = NONE, nd[0].rb = k;
  int n = 1, nrb = 2 * k;  // 次に割り当てる行の先頭
  for (size_t i = 0; i < ss.size(); ++i) {
    int v = 0;
    for (const char x : ss[i]) {
      const int c = code[(unsigned char)x];
      const int a = nd[v].ow, b = rows[nd[v].rb + c];
      int w = nd[v].oc == c ? a : b;  // 行が無ければ 0 の行を読むので、分岐なしで引ける
      if (!w) {
        w = n++;
        nd[w].oc = NONE, par[w] = v;
        if (nd[v].oc == NONE && !nd[v].rb) {
          nd[v].oc = (unsigned char)c, nd[v].ow = w;
        } else {
          if (!nd[v].rb) nd[v].rb = nrb, nrb += k, rows[nd[v].rb + nd[v].oc] = nd[v].ow;
          rows[nd[v].rb + c] = w;
        }
      }
      v = w;
    }
    word[i] = v;
  }
  par.resize(n);
  // 幅優先の順に頂点を取り出す。行を持つ頂点 v は、取り出したときに行の空いた欄を遷移 delta(v, c) で埋めるので、v より浅い頂点の
  // 行はすべて遷移の行になっている。delta(u, c) は、u に行があればその欄、無ければ 1 つ目の子の文字が c ならその子、そうでなければ
  // delta(lk[u], c)。根は初めから行を持つ。
  auto delta = [&](int u, int c) {
    while (true) {
      const Node &e = nd[u];
      if (e.rb) return rows[e.rb + c];
      if (e.oc == c) return e.ow;
      u = e.lk;
    }
  };
  int head = 0, tail = 0;
  for (int c = 0; c < k; ++c) {  // 根の子の suffix link は 0 のまま
    const int w = rows[k + c];
    q[tail] = w, tail += w != 0;
  }
  while (head < tail) {
    const int v = q[head++], lv = nd[v].lk;
    if (const int rb = nd[v].rb) {
      int *r = rows + rb;
      // 子の有無で分岐しないよう、子の無い欄 (w = 0) でも nd[0].lk と待ち行列の末尾に書く。根の lk は読まない
      if (const int rl = nd[lv].rb) {
        for (int c = 0; c < k; ++c) {
          const int w = r[c], d = rows[rl + c];
          nd[w].lk = d;
          q[tail] = w, tail += w != 0;
          r[c] = w | (d & -int(w == 0));
        }
      } else {
        for (int c = 0; c < k; ++c) {
          const int w = r[c], d = delta(lv, c);
          nd[w].lk = d;
          q[tail] = w, tail += w != 0;
          r[c] = w | (d & -int(w == 0));
        }
      }
    } else if (nd[v].oc != NONE) {
      const int w = nd[v].ow;
      nd[w].lk = delta(lv, nd[v].oc), q[tail++] = w;
    }
  }
  ac_common::reserve_huge(link, n);
  link.resize(n);
  link[0] = 0;
  for (int v = 1; v < n; ++v) link[v] = nd[v].lk;
}
}  // namespace ac_hybrid5_body
