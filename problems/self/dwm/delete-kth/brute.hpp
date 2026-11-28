#pragma once
// 愚直解。交差確認 (pj testdata crosscheck) 用。
#include <algorithm>
#include <vector>

using namespace std;

struct Solver {
  vector<int> a;

  Solver() = default;

  void push_back(int x) { a.push_back(x); }
  void erase(int k) { a.erase(a.begin() + k); }
  void set(int k, int x) { a[k] = x; }

  int kth(int l, int r, int k) const {
    vector<int> b(a.begin() + l, a.begin() + r);
    nth_element(b.begin(), b.begin() + k, b.end());
    return b[k];
  }
};
