#pragma once
// 今の Library (mylib) の AhoCorasick (mylib/string/AhoCorasick.hpp) を使う提出。AhoCorasick は失敗のリンクを公開しておらず、状態の
// 番号も幅優先の順なので、公開の transition から組み直す。各文字列を初めの状態から読んで、トライの頂点 (状態) に問題の番号を
// できた順に振り、親と最後の文字を覚える。suffix link は、状態の番号の順 (幅優先なので親が先に来る) に、深さ 1 なら根、そうでなければ
// transition(親の suffix link, 最後の文字) で求める。AhoCorasick の構築 (パターンを並べて列ごとにトライを作り、照合したパターンの列を
// 失敗のリンクに沿って足す) もそのまま測る。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "mylib/string/AhoCorasick.hpp"

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() {
    AhoCorasick<string> ac(ss);
    const int S = ac.state_size();
    vector<int> id(S, -1), lpar(S, 0), lk(S, 0);
    vector<char> lch(S, 0);
    id[0] = 0;
    int n = 1;
    ans.par.assign(S, -1), ans.link.assign(S, -1), ans.word.resize(ss.size());
    for (size_t i = 0; i < ss.size(); ++i) {
      int st = 0;
      for (const char c : ss[i]) {
        const int nx = ac.transition(st, c);
        if (id[nx] < 0) id[nx] = n++, lpar[nx] = st, lch[nx] = c;
        st = nx;
      }
      ans.word[i] = id[st];
    }
    for (int s = 1; s < S; ++s) lk[s] = lpar[s] == 0 ? 0 : ac.transition(lk[lpar[s]], lch[s]);
    for (int s = 1; s < S; ++s) ans.par[id[s]] = id[lpar[s]], ans.link[id[s]] = id[lk[s]];
    ans.par.resize(n), ans.link.resize(n);
  }

  const AhoCorasickAnswer &answer() const { return ans; }
};
