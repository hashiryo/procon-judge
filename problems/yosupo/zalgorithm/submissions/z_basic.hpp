#pragma once
// Z 配列の素直な形。右端が最も右にある一致の区間 [l, r) (箱) を持ち、i が箱の中で z[i - l] < r - i なら z[i] = z[i - l] として比べず、
// そうでなければ r - i (箱の外なら 0) から 1 文字ずつ比べて伸ばし、箱を [i, i + z[i]) に替える。今の Library の z_algorithm は、箱の
// 中で値が決まる位置でも比べを 1 回する。Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#include <string>
#include <vector>
#include "pj.hpp"

struct Solver {
  string s;
  vector<int> z;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    z.assign(n, 0);
    if (n == 0) return;
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    z[0] = n;
    for (int i = 1, l = 0, r = 0; i < n; ++i) {
      int k = 0;
      if (i < r) {
        k = z[i - l];
        if (k < r - i) {
          z[i] = k;
          continue;
        }
        k = r - i;
      }
      while (i + k < n && t[k] == t[i + k]) ++k;
      z[i] = k, l = i, r = i + k;
    }
  }

  const vector<int> &answer() const { return z; }
};
