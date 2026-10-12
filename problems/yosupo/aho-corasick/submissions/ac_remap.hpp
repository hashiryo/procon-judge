#pragma once
// ac_dense で、使われている文字だけに番号を振り直して子の表の幅を文字の種類の数 k にし (2 文字の入力なら 1 頂点 8 バイト)、幅優先の
// 順を、子の表を見る代わりに深さで数え分けて作る版。suffix link は ac_dense と同じく、親の suffix link から始めて文字 c の子を持つ
// 頂点まで suffix link をたどって求める (たどる回数は全体で文字の数の和以下)。中身の確保は _ac_common.hpp。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <array>
#include <string>
#include <vector>
#include "pj.hpp"
#include "_ac_common.hpp"

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() {
    array<unsigned char, 256> code;
    const int k = ac_common::remap(ss, code);
    size_t total = 0;
    for (const auto &s : ss) total += s.size();
    const int cap = int(total) + 1;
    ac_common::Zeroed tab(size_t(cap) * k * sizeof(int));
    int *ch = static_cast<int *>(tab.get());
    vector<int> &par = ans.par, &link = ans.link;
    par.assign(cap, 0), link.assign(cap, 0), ans.word.resize(ss.size());
    vector<unsigned char> last(cap, 0);
    vector<int> depth(cap, 0);
    int n = 1, maxd = 0;
    for (size_t i = 0; i < ss.size(); ++i) {
      int v = 0;
      for (const char x : ss[i]) {
        const int c = code[(unsigned char)x];
        int &w = ch[size_t(v) * k + c];
        if (!w) w = n, par[n] = v, last[n] = (unsigned char)c, depth[n] = depth[v] + 1, maxd = max(maxd, depth[n]), ++n;
        v = w;
      }
      ans.word[i] = v;
    }
    // 深さで数え分けて幅優先の順にする。
    vector<int> cnt(maxd + 2, 0), ord(n);
    for (int v = 1; v < n; ++v) ++cnt[depth[v]];
    for (int d = 1, acc = 0; d <= maxd + 1; ++d) {
      const int c = cnt[d];
      cnt[d] = acc, acc += c;
    }
    for (int v = 1; v < n; ++v) ord[cnt[depth[v]]++] = v;
    for (int t = 0; t < n - 1; ++t) {
      const int v = ord[t];
      if (depth[v] == 1) continue;
      const int c = last[v];
      int u = link[par[v]];
      while (u && !ch[size_t(u) * k + c]) u = link[u];
      link[v] = ch[size_t(u) * k + c];
    }
    par.resize(n), link.resize(n);
  }

  const AhoCorasickAnswer &answer() const { return ans; }
};
