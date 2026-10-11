#pragma once
// duval で、s[k] = s[j] が続くところを 8 バイトずつ比べる版。k と j を 1 つずつ進める代わりに、t[k, k + 8) と t[j, j + 8) の排他的
// 論理和の下位から 0 のバイトを数えて、一致した分だけまとめて進める (文字列は写さず、末尾の 8 バイトに満たないところは 1 文字ずつ)。
// 一致が途切れた文字で、duval と同じく s[k] < s[j] なら k を i に戻し、s[k] > s[j] なら止める。周期的な列 (all_same、fib_str、
// binary_carry) で、一致の続く長いところを速く進む。
// Lyndon 分解の実装を比べた記録は algo-notes の notes/lyndon.md にある。
#include <bit>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include "pj.hpp"

namespace duval_w8 {
inline uint64_t load(const unsigned char *p) {
  uint64_t x;
  std::memcpy(&x, p, 8);
  return x;
}
}  // namespace duval_w8

struct Solver {
  string s;
  vector<int> cut;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    using duval_w8::load;
    const int n = int(s.size());
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    cut.clear();
    cut.push_back(0);
    for (int i = 0; i < n;) {
      int j = i + 1, k = i;
      while (j < n) {
        // s[k] = s[j] の続く長さを数え、k と j をまとめて進める
        while (j + 8 <= n) {
          const uint64_t x = load(t + k) ^ load(t + j);
          if (x) {
            const int m = std::countr_zero(x) >> 3;
            k += m, j += m;
            break;
          }
          k += 8, j += 8;
        }
        while (j < n && t[k] == t[j]) ++k, ++j;
        if (j == n || t[k] > t[j]) break;
        k = i, ++j;  // s[k] < s[j]: s[i, j + 1) は Lyndon 語
      }
      for (const int p = j - k; i <= k; i += p) cut.push_back(i + p);
    }
  }

  const vector<int> &answer() const { return cut; }
};
