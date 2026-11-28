#pragma once
// 両端に伸ばせるリングバッファで解く。容量は Q から決めて構築で確保する (計測区間の外)。
// 容量を 2 の冪にして、添字は (head + i) & mask で引く。除算が要らず、添字は O(1)。
// 列の長さは Q を超えないので、容量を Q より大きく取れば溢れない。
#include <array>
#include <vector>

struct Solver {
  const std::vector<std::array<int, 2>> &queries;
  std::vector<int> buf;  // リングバッファ。容量は 2 の冪
  std::vector<int> ans;  // タイプ 4 の答え
  int mask = 0;          // 容量 - 1
  int head = 0;          // 先頭の位置
  int size = 0;          // 列の長さ

  explicit Solver(const std::vector<std::array<int, 2>> &queries_)
      : queries(queries_) {
    int cap = 1;
    while (cap <= (int)queries.size()) cap <<= 1;
    buf.assign(cap, 0);
    mask = cap - 1;
    ans.reserve(queries.size());
  }

  void run() {
    for (const auto &e : queries) {
      switch (e[0]) {
        case 0:  // 先頭に x を足す
          head = (head - 1) & mask;
          buf[head] = e[1];
          ++size;
          break;
        case 1:  // 末尾に x を足す
          buf[(head + size) & mask] = e[1];
          ++size;
          break;
        case 2:  // 先頭を消す
          head = (head + 1) & mask;
          --size;
          break;
        case 3:  // 末尾を消す
          --size;
          break;
        default:  // a_i を出す
          ans.push_back(buf[(head + e[1]) & mask]);
          break;
      }
    }
  }

  const std::vector<int> &answers() const { return ans; }
};
