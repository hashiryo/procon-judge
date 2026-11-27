#pragma once
// 傾きが単調に減る順 (同じ傾きが続いてもよい) に直線 y = a x + b を足す、直線の集合の最小。下側の包絡を、傾きと切片を
// 別の配列に持つ。足すたびに末尾の要らなくなった直線を除く。問いは、x が単調に増えるなら先頭から進める query_inc、任意の
// 順なら二分探索の query。要らないかの判定は、切片の差と傾きの差の積を __int128 で比べる。傾きと切片を別の配列にするのは、
// 問いと追加で読む値が 8 byte ずつ詰まって並ぶため (atcoder-dp-z の手元の比較で 3〜4 割)。
#include <cstddef>
#include <vector>

struct MonotoneCHT {
  std::vector<long long> as, bs;  // 包絡に出る直線の傾きと切片
  size_t head = 0;                // query_inc がこれより前を見ない

  // a1 > a2 > a3 のとき、l2 が包絡に出ないか。
  static bool bad(long long a1, long long b1, long long a2, long long b2, long long a3, long long b3) {
    return (__int128)(b3 - b1) * (a1 - a2) <= (__int128)(b2 - b1) * (a1 - a3);
  }

  void add_line(long long a, long long b) {
    if (!as.empty() && as.back() == a) {
      if (bs.back() <= b) return;
      as.pop_back(), bs.pop_back();
    }
    while (as.size() >= 2 && bad(as[as.size() - 2], bs[bs.size() - 2], as.back(), bs.back(), a, b)) {
      as.pop_back(), bs.pop_back();
    }
    as.push_back(a), bs.push_back(b);
    // query_inc が指していた直線が除かれたら、残った末尾の 1 つ前から見直す。
    if (head + 2 > as.size()) head = as.size() >= 2 ? as.size() - 2 : 0;
  }

  // 前の問いより小さくない x での最小。
  long long query_inc(long long x) {
    while (head + 1 < as.size() && as[head + 1] * x + bs[head + 1] <= as[head] * x + bs[head]) ++head;
    return as[head] * x + bs[head];
  }

  // 任意の x での最小。
  long long query(long long x) const {
    size_t lo = 0, hi = as.size() - 1;
    while (lo < hi) {
      const size_t mid = (lo + hi) >> 1;
      if (as[mid] * x + bs[mid] <= as[mid + 1] * x + bs[mid + 1]) hi = mid;
      else lo = mid + 1;
    }
    return as[lo] * x + bs[lo];
  }
};
