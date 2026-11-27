#pragma once
// 傾きが単調に減る順 (同じ傾きが続いてもよい) に直線 y = a x + b を足す、直線の集合の最小。下側の包絡を配列に持ち、
// 足すたびに末尾の要らなくなった直線を除く。問いは、x が単調に増えるなら先頭から進める query_inc、任意の順なら二分探索の
// query。要らないかの判定は、切片の差と傾きの差の積を __int128 で比べる。
#include <cstddef>
#include <vector>

struct MonotoneCHT {
  struct Line {
    long long a, b;
  };
  std::vector<Line> h;
  size_t head = 0;  // query_inc がこれより前を見ない

  // a1 > a2 > a3 のとき、l2 が包絡に出ないか。
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
    // query_inc が指していた直線が除かれたら、残った末尾の 1 つ前から見直す。
    if (head + 2 > h.size()) head = h.size() >= 2 ? h.size() - 2 : 0;
  }

  static long long f(const Line &e, long long x) { return e.a * x + e.b; }

  // 前の問いより小さくない x での最小。
  long long query_inc(long long x) {
    while (head + 1 < h.size() && f(h[head + 1], x) <= f(h[head], x)) ++head;
    return f(h[head], x);
  }

  // 任意の x での最小。
  long long query(long long x) const {
    size_t lo = 0, hi = h.size() - 1;
    while (lo < hi) {
      const size_t mid = (lo + hi) >> 1;
      if (f(h[mid], x) <= f(h[mid + 1], x)) hi = mid;
      else lo = mid + 1;
    }
    return f(h[lo], x);
  }
};
