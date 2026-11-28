#pragma once
// 期待出力を作る参照実装。永続 binary trie を NeoLibrary を使わずに素直に書いたもの。
// 愚直解 (brute.hpp) と小さい入力で突き合わせる (pj testdata crosscheck)。
#include <vector>

struct Solver {
  static constexpr int BITS = 30;
  struct Node {
    int ch[2];
    int c;
  };
  std::vector<Node> t;
  std::vector<int> root;
  explicit Solver(const std::vector<int> &a) {
    t.push_back(Node{{0, 0}, 0});
    root.reserve(a.size() + 1);
    root.push_back(0);
    for (int x : a) root.push_back(insert(root.back(), BITS - 1, x));
  }
  int insert(int p, int b, int x) {
    int q = t.size();
    t.push_back(t[p]);
    ++t[q].c;
    if (b < 0) return q;
    const int bit = (x >> b) & 1;
    t[q].ch[bit] = insert(t[p].ch[bit], b - 1, x);
    return q;
  }
  int max_xor(int l, int r, int x) const {
    int pa = root[l], pb = root[r], res = 0;
    for (int b = BITS - 1; b >= 0; --b) {
      const int xb = (x >> b) & 1, want = xb ^ 1;
      const int ca = t[pa].ch[want], cb = t[pb].ch[want];
      if (t[cb].c - t[ca].c > 0) {
        res |= 1 << b;
        pa = ca, pb = cb;
      } else {
        pa = t[pa].ch[xb], pb = t[pb].ch[xb];
      }
    }
    return res;
  }
};
