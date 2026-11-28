#pragma once
// 最小側と最大側に 1 本ずつヒープを持ち、片方から取り出した要素を逆側では遅延削除で飛ばす。
// 値は座標圧縮して添字で持ち、遅延削除の残り数は添字ごとの配列に置く。
#include <algorithm>
#include <functional>
#include <queue>
#include <vector>

struct Solver {
  int n;
  std::vector<long long> s;
  std::vector<int> type;
  std::vector<long long> x;
  std::vector<long long> ans;

  Solver(int n_, std::vector<long long> s_, std::vector<int> type_,
         std::vector<long long> x_)
      : n(n_), s(std::move(s_)), type(std::move(type_)), x(std::move(x_)) {}

  void run() {
    std::vector<long long> vals;
    vals.reserve(s.size() + type.size());
    for (long long v : s) vals.push_back(v);
    for (size_t i = 0; i < type.size(); ++i)
      if (type[i] == 0) vals.push_back(x[i]);
    std::sort(vals.begin(), vals.end());
    vals.erase(std::unique(vals.begin(), vals.end()), vals.end());
    const int m = (int)vals.size();

    std::vector<int> skip_min(m, 0), skip_max(m, 0);
    std::priority_queue<int, std::vector<int>, std::greater<int>> minh;
    std::priority_queue<int> maxh;
    auto index_of = [&](long long v) {
      return (int)(std::lower_bound(vals.begin(), vals.end(), v) - vals.begin());
    };

    for (long long v : s) {
      const int k = index_of(v);
      minh.push(k);
      maxh.push(k);
    }

    ans.clear();
    ans.reserve(type.size());
    for (size_t i = 0; i < type.size(); ++i) {
      if (type[i] == 0) {
        const int k = index_of(x[i]);
        minh.push(k);
        maxh.push(k);
      } else if (type[i] == 1) {
        while (skip_min[minh.top()] > 0) {
          --skip_min[minh.top()];
          minh.pop();
        }
        const int k = minh.top();
        minh.pop();
        ++skip_max[k];
        ans.push_back(vals[k]);
      } else {
        while (skip_max[maxh.top()] > 0) {
          --skip_max[maxh.top()];
          maxh.pop();
        }
        const int k = maxh.top();
        maxh.pop();
        ++skip_min[k];
        ans.push_back(vals[k]);
      }
    }
  }

  const std::vector<long long> &answer() const { return ans; }
};
