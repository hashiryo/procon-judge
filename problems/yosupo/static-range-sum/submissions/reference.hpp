#pragma once
// 累積和で解く。s[0] = 0、s[i + 1] = s[i] + a[i] として、sum a[l, r) = s[r] - s[l]。
// 構築は O(N)、クエリは O(1)。和は 5 * 10^5 * 10^9 = 5 * 10^14 までなので i64 で足りる。
#include <array>
#include <vector>

struct Solver {
  const std::vector<long long> &a;
  const std::vector<std::array<int, 2>> &queries;
  std::vector<long long> sum;  // 長さ N + 1 の累積和
  std::vector<long long> ans;  // クエリの答え

  Solver(const std::vector<long long> &a_,
         const std::vector<std::array<int, 2>> &queries_)
      : a(a_), queries(queries_) {
    sum.resize(a.size() + 1);
    ans.resize(queries.size());
  }

  void run() {
    const int n = (int)a.size();
    for (int i = 0; i < n; ++i) sum[i + 1] = sum[i] + a[i];
    const int q = (int)queries.size();
    for (int i = 0; i < q; ++i) {
      ans[i] = sum[queries[i][1]] - sum[queries[i][0]];
    }
  }

  const std::vector<long long> &answers() const { return ans; }
};
