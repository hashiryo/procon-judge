#pragma once
#include "pj.hpp"
#include "mylib/string/z_algorithm.hpp"

// Z アルゴリズム。Z[i] が「i 文字目から始まる接尾辞と全体の最長共通接頭辞」
// なので、前半 (長さ i) と後半の比較は Z[i] と i の大小と次の 1 文字で決まる。
// 全体で O(N)。
struct Solver {
  vector<string> cases;
  vector<i64> ans;

  explicit Solver(const vector<string> &cases) : cases(cases) {}

  void run() {
    ans.clear();
    for (auto &s : cases) {
      auto z = z_algorithm(s);
      const int n = (int)s.size();
      i64 cnt = 0;
      for (int i = 1; i < n; ++i) {
        if (i < z[i]) ++cnt;                                // 前半が後半の真の接頭辞
        else if (i == z[i] && i < n - i) ++cnt;             // 同上 (後半の方が長い)
        else if (i > z[i] && s[z[i]] < s[i + z[i]]) ++cnt;  // 最初に違う文字で決まる
      }
      ans.push_back(cnt);
    }
  }

  const vector<i64> &answer() const { return ans; }
};
