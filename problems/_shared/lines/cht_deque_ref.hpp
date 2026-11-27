#pragma once
// 傾きが単調に減る順 (同じ傾きが続いてもよい) に直線 y = a x + b を足す、直線の集合の最小。問いは x が前の問いより
// 小さくない順だけ (query_inc)。atcoder-dp-z の参照実装 (submissions/reference.hpp) と同じく、除くのは頭が指す直線より
// 後ろだけにして、頭を戻さない。頭が指す直線は古い問いの最良で、x が増える問いではそれより前の直線は最良にならない。
// 同じ傾きは、切片の小さいほうだけを残す。
#include <cstddef>
#include <vector>

struct MonotoneCHTRef {
  std::vector<long long> as, bs;  // 包絡に出る直線の傾きと切片
  size_t head = 0;                // これより前の直線は除かず、問いでも見ない

  // a1 > a2 > a3 のとき、l2 が包絡に出ないか。
  static bool bad(long long a1, long long b1, long long a2, long long b2, long long a3, long long b3) {
    return (__int128)(b3 - b1) * (a1 - a2) <= (__int128)(b2 - b1) * (a1 - a3);
  }

  void add_line(long long a, long long b) {
    if (!as.empty() && as.back() == a) {
      if (bs.back() <= b) return;
      as.pop_back(), bs.pop_back();
      // 頭が指す直線を除いたら、頭は新しい直線 (足したあとで添字 head) を指す。
      if (head > as.size()) head = as.size();
    }
    // 頭より後ろだけを除く。頭が指す直線は残すので、頭を戻さなくてよい。
    while (as.size() >= head + 2 && bad(as[as.size() - 2], bs[bs.size() - 2], as.back(), bs.back(), a, b)) {
      as.pop_back(), bs.pop_back();
    }
    as.push_back(a), bs.push_back(b);
  }

  // 前の問いより小さくない x での最小。
  long long query_inc(long long x) {
    while (head + 1 < as.size() && as[head + 1] * x + bs[head + 1] <= as[head] * x + bs[head]) ++head;
    return as[head] * x + bs[head];
  }
};
