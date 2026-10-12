#pragma once
// ac_goto3 と ac_mix3 が共有する中身 (説明は ac_goto3.hpp)。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <array>
#include <string>
#include <vector>
#include "_ac_common.hpp"

namespace ac_goto3_body {
inline void solve(const std::vector<std::string> &ss, const ac_common::Coded &cd, AhoCorasickAnswer &ans) {
  using namespace std;
  const auto &code = cd.code;
  const int k = cd.k;
  const size_t cap = cd.total + 1;
  // 遷移の表 (cap 行、幅 k) と幅優先の待ち行列 (cap 個) を、huge page を頼んだ 1 つの領域に置く
  ac_common::Zeroed buf((cap * k + cap) * sizeof(int));
  int *ch = static_cast<int *>(buf.get()), *q = ch + cap * k;
  vector<int> &par = ans.par, &link = ans.link, &word = ans.word;
  // push_back は末尾の位置を読み書きする待ちが要素ごとに連なるので、先に 0 で埋めて添字で書く
  ac_common::reserve_huge(par, cap), ac_common::reserve_huge(word, ss.size());
  par.resize(cap), word.resize(ss.size());
  int n = 1;
  for (size_t i = 0; i < ss.size(); ++i) {
    int v = 0;
    for (const char x : ss[i]) {
      int &w = ch[size_t(v) * k + code[(unsigned char)x]];
      if (!w) w = n++, par[w] = v;
      v = w;
    }
    word[i] = v;
  }
  par.resize(n);
  ac_common::reserve_huge(link, n);
  link.assign(n, 0);
  int head = 0, tail = 0;
  for (int c = 0; c < k; ++c) {  // 根の子の suffix link は 0 のまま
    const int w = ch[c];
    q[tail] = w, tail += w != 0;
  }
  // 頂点 v を取り出したとき、v の suffix link l は v より浅く、l の行は遷移で埋め終えている。v の子 w の suffix link は行 l の
  // 同じ文字の欄で、v の行の空いた欄には行 l の欄を写す。子の有無で分岐しないよう、子の無い欄 (w = 0) でも link[0] と待ち行列の
  // 末尾に書き、末尾は子のときだけ進める。link[0] は使わない。
  while (head < tail) {
    const int v = q[head++];
    int *rv = ch + size_t(v) * k;
    const int *rl = ch + size_t(link[v]) * k;
    for (int c = 0; c < k; ++c) {
      const int w = rv[c], d = rl[c];
      link[w] = d;
      q[tail] = w, tail += w != 0;
      rv[c] = w | (d & -int(w == 0));
    }
  }
  link[0] = 0;
}
}  // namespace ac_goto3_body
