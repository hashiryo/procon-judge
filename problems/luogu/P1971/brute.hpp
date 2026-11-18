#pragma once
// 愚直解 (pj testdata crosscheck 用)。マスが 12 個ほどまでの盤で、マッチングへの言い換えを使わず、盤の上で手を全部試して
// 必勝かを求める (盤の文字列と手番で覚える)。兔兔の手ごとに、指す前の局面が兔兔の必勝で、指したあとの局面が蛋蛋の必勝
// なら犯错误とする。
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

struct Solver {
  int n, m;
  const vector<string> &b;
  const vector<array<int, 2>> &mv;
  map<string, bool> memo;
  Solver(int n, int m, const vector<string> &board, const vector<array<int, 2>> &moves) : n(n), m(m), b(board), mv(moves) {}
  // s の末尾の文字は手番 (0 が兔兔、1 が蛋蛋)。手番の人が勝てるか。
  bool win(const string &s) {
    if (auto it = memo.find(s); it != memo.end()) return it->second;
    const int e = s.find('.'), x = e / m, y = e % m;
    const char col = s.back() == '0' ? 'O' : 'X';
    const int dx[4] = {1, -1, 0, 0}, dy[4] = {0, 0, 1, -1};
    bool res = false;
    for (int d = 0; d < 4 && !res; ++d) {
      const int nx = x + dx[d], ny = y + dy[d];
      if (nx < 0 || nx >= n || ny < 0 || ny >= m || s[nx * m + ny] != col) continue;
      string t = s;
      t[e] = col, t[nx * m + ny] = '.', t.back() ^= 1;
      res = !win(t);
    }
    return memo[s] = res;
  }
  vector<int> run() {
    if (n * m > 16) abort();
    string s;
    for (auto &r : b) s += r;
    s += '0';
    vector<int> res;
    for (size_t i = 0; i < mv.size(); ++i) {
      const bool before = win(s);
      const int e = s.find('.'), c = mv[i][0] * m + mv[i][1];
      s[e] = s[c], s[c] = '.', s.back() ^= 1;
      if (i % 2 == 0 && before && win(s)) res.push_back(i / 2 + 1);
    }
    return res;
  }
};
