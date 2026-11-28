#pragma once
// 愚直解。交差確認 (pj testdata crosscheck) 用。
#include <algorithm>
#include <vector>

using namespace std;

struct Solver {
  vector<int> a;

  explicit Solver(const vector<int> &a_) : a(a_) {}

  void shift(int l, int r) {
    int x = a[r - 1];
    a.erase(a.begin() + (r - 1));
    a.insert(a.begin() + l, x);
  }

  int count(int l, int r, int k) const {
    int c = 0;
    for (int i = l; i < r; ++i) c += a[i] == k;
    return c;
  }
};
