#pragma once
// ac_hybrid と ac_mix が共有する中身 (説明は ac_hybrid.hpp)。子が 1 つの間は (文字, 子) を頂点に直接持ち、枝分かれする頂点にだけ
// 文字で直接引く行を割り当て、suffix link は深さの順にたどって求める。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <array>
#include <memory>
#include <string>
#include <vector>
#include "_ac_common.hpp"

namespace ac_hybrid_body {
inline void solve(const std::vector<std::string> &ss, AhoCorasickAnswer &ans) {
  using namespace std;
  array<unsigned char, 256> code;
  const int k = ac_common::remap(ss, code);
  size_t total = 0;
  for (const auto &s : ss) total += s.size();
  const int cap = int(total) + 1;
  const size_t rows_cap = (ss.size() + 2) * size_t(k);  // 行を持つ頂点は、根と、枝分かれする頂点 (文字列の本数未満)
  ac_common::Zeroed rowbuf(rows_cap * sizeof(int));
  int *rows = static_cast<int *>(rowbuf.get());
  vector<int> &par = ans.par, &link = ans.link;  // link には、suffix link を書くまで深さを置く
  par.assign(cap, 0), link.assign(cap, 0), ans.word.resize(ss.size());
  constexpr unsigned char NONE = 255;
  unique_ptr<unsigned char[]> oc(new unsigned char[cap]), lc(new unsigned char[cap]);
  unique_ptr<int[]> ow(new int[cap]), row(new int[cap]);
  oc[0] = NONE, row[0] = 0;
  int n = 1, nrow = 1, maxd = 0;
  auto child = [&](int v, int c) -> int {
    if (oc[v] == c) return ow[v];
    return row[v] >= 0 ? rows[size_t(row[v]) * k + c] : 0;
  };
  for (size_t i = 0; i < ss.size(); ++i) {
    int v = 0;
    for (const char x : ss[i]) {
      const int c = code[(unsigned char)x];
      int w = child(v, c);
      if (!w) {
        w = n++;
        oc[w] = NONE, row[w] = -1, lc[w] = (unsigned char)c, par[w] = v, link[w] = link[v] + 1, maxd = max(maxd, link[w]);
        if (oc[v] == NONE && row[v] < 0) {
          oc[v] = (unsigned char)c, ow[v] = w;
        } else {
          if (row[v] < 0) row[v] = nrow++, rows[size_t(row[v]) * k + oc[v]] = ow[v];
          rows[size_t(row[v]) * k + c] = w;
        }
      }
      v = w;
    }
    ans.word[i] = v;
  }
  vector<int> cnt(maxd + 2, 0), ord(n);
  for (int v = 1; v < n; ++v) ++cnt[link[v]];
  for (int d = 1, acc = 0; d <= maxd + 1; ++d) {
    const int c = cnt[d];
    cnt[d] = acc, acc += c;
  }
  for (int v = 1; v < n; ++v) ord[cnt[link[v]]++] = v;
  for (int t = 0; t < n - 1; ++t) {
    const int v = ord[t];
    if (par[v] == 0) {
      link[v] = 0;
      continue;
    }
    const int c = lc[v];
    int u = link[par[v]], w;
    while (!(w = child(u, c)) && u) u = link[u];
    link[v] = w;
  }
  par.resize(n), link.resize(n);
}
}  // namespace ac_hybrid_body
