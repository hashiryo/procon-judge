#pragma once
// 愚直解。交差確認 (pj testdata crosscheck) 用。k 番目は区間を写して nth_element。
#include <algorithm>
#include <vector>

using namespace std;

struct Solver {
  vector<int> a;

  explicit Solver(const vector<int> &a_) : a(a_) {}

  void insert(int k, int x) { a.insert(a.begin() + k, x); }
  void set(int k, int x) { a[k] = x; }

  int kth(int l, int r, int k) const {
    vector<int> b(a.begin() + l, a.begin() + r);
    nth_element(b.begin(), b.begin() + k, b.end());
    return b[k];
  }
};
