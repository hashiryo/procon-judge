#pragma once
// NeoLibrary の PalindromeDeque (dq_tk2 を写したもの) を呼ぶ提出。クエリに現れる文字に 0 から番号を振り、種類の数を 2、4、8、26 の
// どれかに切り上げた幅で組む (dq_tk2 と同じ)。
#include <array>
#include <utility>
#include <vector>
#include "pj.hpp"
#include "neo/string/PalindromeDeque.hpp"

struct Solver {
  vector<pair<int, char>> qs;
  vector<array<int, 3>> ans;

  explicit Solver(const vector<pair<int, char>> &qs) : qs(qs) {}

  template <int K> void body(const array<int, 256> &code) {
    int pushes = 0;
    for (const auto &[type, ch] : qs) pushes += type <= 1;
    PalindromeDeque<K> d(pushes);
    ans.resize(qs.size());
    for (size_t t = 0; t < qs.size(); ++t) {
      const auto [type, ch] = qs[t];
      if (type == 0) d.push_front(code[(unsigned char)ch]);
      else if (type == 1) d.push_back(code[(unsigned char)ch]);
      else if (type == 2) d.pop_front();
      else d.pop_back();
      ans[t] = {d.distinct(), d.longest_prefix(), d.longest_suffix()};
    }
  }

  void run() {
    array<int, 256> code;
    code.fill(0);
    array<bool, 256> used{};
    int k = 0;
    for (const auto &[type, ch] : qs)
      if (type <= 1 && !used[(unsigned char)ch]) used[(unsigned char)ch] = true, code[(unsigned char)ch] = k++;
    if (k <= 2) body<2>(code);
    else if (k <= 4) body<4>(code);
    else if (k <= 8) body<8>(code);
    else body<26>(code);
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
