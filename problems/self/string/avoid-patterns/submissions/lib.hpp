#pragma once
// 今の Library の AhoCorasick (mylib/string/AhoCorasick.hpp) を遷移に使い、Automaton (mylib/counting/Automaton.hpp) で状態に番号を
// 振って長さ n の文字列を数える。パターンに当たった状態は捨てる状態 (-1) に送る。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/string/AhoCorasick.hpp"
#include "mylib/counting/Automaton.hpp"

struct Solver {
  const vector<string> &ps;
  int sigma;
  Solver(const vector<string> &ps, int sigma) : ps(ps), sigma(sigma) {}
  u32 count(int n) {
    using Mint = ModInt<998244353>;
    AhoCorasick<string> ac(ps);
    vector<char> alp(sigma);
    for (int c = 0; c < sigma; ++c) alp[c] = char('a' + c);
    auto tr = [&](int s, char c) {
      const int t = ac.transition(s, c);
      return ac.is_accept(t) ? -1 : t;
    };
    Automaton dfa(alp, 0, tr, [](int) { return true; }, -1);
    return dfa.num<Mint>(n).val();
  }
};
