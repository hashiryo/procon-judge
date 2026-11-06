#pragma once
// 参照実装。各頂点に 26 個の子の欄と、その頂点を通る要素の数を持つトライ。
struct Solver {
  vector<array<int, 26>> ch;
  vector<int> cnt;
  Solver() : ch(1), cnt(1, 0) { ch[0].fill(-1); }
  void add(const string &s, int d) {
    int v = 0;
    cnt[0] += d;
    for (char c : s) {
      int &w = ch[v][c - 'a'];
      if (w < 0) {
        w = (int)ch.size();
        ch.emplace_back();
        ch.back().fill(-1);
        cnt.push_back(0);
      }
      v = ch[v][c - 'a'];
      cnt[v] += d;
    }
  }
  void insert(const string &s) { add(s, 1); }
  void erase(const string &s) { add(s, -1); }
  int count_prefix(const string &p) {
    int v = 0;
    for (char c : p) {
      v = ch[v][c - 'a'];
      if (v < 0) return 0;
    }
    return cnt[v];
  }
};
