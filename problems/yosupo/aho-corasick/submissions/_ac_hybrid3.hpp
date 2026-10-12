#pragma once
// ac_hybrid3 と ac_mix3 が共有する中身 (説明は ac_hybrid3.hpp)。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <array>
#include <string>
#include <vector>
#include "_ac_common.hpp"

namespace ac_hybrid3_body {
inline void solve(const std::vector<std::string> &ss, const ac_common::Coded &cd, AhoCorasickAnswer &ans) {
  using namespace std;
  const auto &code = cd.code;
  const int k = cd.k;
  const size_t cap = cd.total + 1;
  const size_t rows_cap = (ss.size() + 2) * size_t(k);  // 行を持つ頂点は、根と、枝分かれする頂点 (文字列の本数未満)
  // 行 (rows_cap 個)、1 つ目の子 ow、行の番号 + 1 の rid (0 なら行が無い)、幅優先の待ち行列 q、1 つ目の子の文字 oc を、huge page を
  // 頼んだ 1 つの領域に置く
  ac_common::Zeroed buf((rows_cap + 3 * cap) * sizeof(int) + cap);
  int *rows = static_cast<int *>(buf.get()), *ow = rows + rows_cap, *rid = ow + cap, *q = rid + cap;
  unsigned char *oc = reinterpret_cast<unsigned char *>(q + cap);
  vector<int> &par = ans.par, &link = ans.link, &word = ans.word;
  // push_back は末尾の位置を読み書きする待ちが要素ごとに連なるので、先に 0 で埋めて添字で書く
  ac_common::reserve_huge(par, cap), ac_common::reserve_huge(word, ss.size());
  par.resize(cap), word.resize(ss.size());
  constexpr unsigned char NONE = 255;
  oc[0] = NONE, rid[0] = 1;
  int n = 1, nrow = 1;
  auto child = [&](int v, int c) -> int {
    if (oc[v] == c) return ow[v];
    return rid[v] ? rows[size_t(rid[v] - 1) * k + c] : 0;
  };
  for (size_t i = 0; i < ss.size(); ++i) {
    int v = 0;
    for (const char x : ss[i]) {
      const int c = code[(unsigned char)x];
      int w = child(v, c);
      if (!w) {
        w = n++;
        oc[w] = NONE, par[w] = v;
        if (oc[v] == NONE && !rid[v]) {
          oc[v] = (unsigned char)c, ow[v] = w;
        } else {
          if (!rid[v]) rid[v] = ++nrow, rows[size_t(nrow - 1) * k + oc[v]] = ow[v];
          rows[size_t(rid[v] - 1) * k + c] = w;
        }
      }
      v = w;
    }
    word[i] = v;
  }
  par.resize(n);
  ac_common::reserve_huge(link, n);
  link.assign(n, 0);
  // 幅優先の順に頂点を取り出し、子 w (文字 c) の suffix link を、v の suffix link から始めて文字 c の子を持つ頂点まで suffix link を
  // たどって求める。根の子の suffix link は 0 のまま。
  auto walk = [&](int u, int c) {
    int w;
    while (!(w = child(u, c)) && u) u = link[u];
    return w;
  };
  int head = 0, tail = 0;
  for (int c = 0; c < k; ++c)
    if (rows[c]) q[tail++] = rows[c];
  while (head < tail) {
    const int v = q[head++], lv = link[v];
    if (rid[v]) {
      const int *r = rows + size_t(rid[v] - 1) * k;
      for (int c = 0; c < k; ++c)
        if (const int w = r[c]) link[w] = walk(lv, c), q[tail++] = w;
    } else if (oc[v] != NONE) {
      const int w = ow[v];
      link[w] = walk(lv, oc[v]), q[tail++] = w;
    }
  }
}
}  // namespace ac_hybrid3_body
