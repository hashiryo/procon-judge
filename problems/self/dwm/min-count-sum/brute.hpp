#pragma once
// 愚直解。交差確認 (pj testdata crosscheck) 用。
#include <algorithm>
#include <vector>

using namespace std;

struct Solver {
  vector<int> a;

  explicit Solver(const vector<int> &a_) : a(a_) {}

  void set(int c, int x) { a[c] = x; }

  int query(int l, int r, long long k) const {
    vector<int> b(a.begin() + l, a.begin() + r);
    sort(b.begin(), b.end(), greater<int>());
    long long s = 0;
    for (int i = 0; i < (int)b.size(); ++i) {
      s += b[i];
      if (s >= k) return i + 1;
    }
    return -1;
  }
};
