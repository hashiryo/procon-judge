#pragma once
// 直線 y = a x + b の集合の最小を、上側の包絡 (符号を反転した直線の最大) を平衡二分探索木に持って求める、傾きの順を問わない
// Convex Hull Trick。KACTL の LineContainer と同じ作りで、各直線に「この直線が最大である x の右端」を持たせ、問いは
// lower_bound で引く。POOL が true なら、木の節を std::pmr::unsynchronized_pool_resource から取り、消した節を使い回す。
#include <climits>
#include <memory_resource>
#include <set>

template <bool POOL> struct SetCHT {
  struct Line {
    mutable long long k, m, p;
    bool operator<(const Line &o) const { return k < o.k; }
    bool operator<(long long x) const { return p < x; }
  };
  using Set = std::conditional_t<POOL, std::pmr::multiset<Line, std::less<>>, std::multiset<Line, std::less<>>>;
  std::pmr::unsynchronized_pool_resource pool;
  Set s;
  using It = typename Set::iterator;

  SetCHT() : s(make()) {}
  Set make() {
    if constexpr (POOL) return Set(&pool);
    else return Set();
  }

  static long long div(long long a, long long b) { return a / b - ((a ^ b) < 0 && a % b); }
  bool isect(It x, It y) {
    if (y == s.end()) return x->p = LLONG_MAX, false;
    if (x->k == y->k) x->p = x->m > y->m ? LLONG_MAX : LLONG_MIN;
    else x->p = div(y->m - x->m, x->k - y->k);
    return x->p >= y->p;
  }

  void add_line(long long a, long long b) {
    auto z = s.insert({-a, -b, 0}), y = z++, x = y;
    while (isect(y, z)) z = s.erase(z);
    if (x != s.begin() && isect(--x, y)) isect(x, y = s.erase(y));
    while ((y = x) != s.begin() && (--x)->p >= y->p) isect(x, s.erase(y));
  }

  long long query(long long x) const {
    const auto &l = *s.lower_bound(x);
    return -(l.k * x + l.m);
  }
};
