#pragma once
// 傾きが単調に減る順 (同じ傾きが続いてもよい) に直線 y = a x + b を足す、直線の集合の最小。傾きと切片を 1 つの
// 構造体 (a, b) に持つ形で、中身は cht_deque.hpp と同じ。傾きと切片を別の配列に持つ形との差を測るための核。
#include <cstddef>
#include <vector>

struct MonotoneCHTAos {
  struct Line {
    long long a, b;
  };
  std::vector<Line> h;
  size_t head = 0;  // query_inc がこれより前を見ない

  static bool bad(const Line &l1, const Line &l2, const Line &l3) {
    return (__int128)(l3.b - l1.b) * (l1.a - l2.a) <= (__int128)(l2.b - l1.b) * (l1.a - l3.a);
  }

  void add_line(long long a, long long b) {
    if (!h.empty() && h.back().a == a) {
      if (h.back().b <= b) return;
      h.pop_back();
    }
    const Line l{a, b};
    while (h.size() >= 2 && bad(h[h.size() - 2], h.back(), l)) h.pop_back();
    h.push_back(l);
    if (head + 2 > h.size()) head = h.size() >= 2 ? h.size() - 2 : 0;
  }

  static long long f(const Line &e, long long x) { return e.a * x + e.b; }

  long long query_inc(long long x) {
    while (head + 1 < h.size() && f(h[head + 1], x) <= f(h[head], x)) ++head;
    return f(h[head], x);
  }
};
