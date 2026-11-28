#pragma once
// Static Range LIS Query。順列 P の区間 P[l, r) の最長増加部分列の長さを Q 個答える。
//
// 半局所 LCS の seaweed を使う。順列 P から seaweed と呼ばれる順列 row を作ると
//   LIS(P[l, r)) = (r - l) - #{i in [l, N) : row[i] < r}
// が成り立つ (Tiskin 2008)。row は値の半分での分割統治と unit-Monge 行列の積で
// O(N log N) に作れ、右辺の数え上げは wavelet matrix で O(log N) で答える。
// 構築 (seaweed と wavelet matrix) は計測区間の外、run() はクエリの数え上げだけ。
//
// 参考文献: A. Tiskin, "Semi-local string comparison: algorithmic techniques and
// applications", Mathematics in Computer Science 1(4), 2008.
#include <algorithm>
#include <array>
#include <cstddef>
#include <numeric>
#include <vector>

#include "pj.hpp"

namespace static_range_lis_query_impl {

using vi = std::vector<int>;

// seaweed の「糸が無い」印。
constexpr int absent = -1;

// 置換 p の逆置換。p に無い値の位置は absent。
inline vi inverse(const vi &p) {
  const int n = (int)p.size();
  vi q(n, absent);
  for (int i = 0; i < n; ++i) {
    if (p[i] != absent) q[p[i]] = i;
  }
  return q;
}

// unit_monge_dmul の作業領域の大きさ。
inline int dmul_stack_size(int n) {
  int ret = 0;
  while (n > 1) {
    ret += 2 * n;  // 答えの行と列
    n = (n + 1) / 2;
    ret += 4 * n;  // 半分に分けた配列と、その積の作業領域
  }
  return ret + 1;
}

// a, b は [0, n) の順列。unit-Monge 行列の積の置換を stack の先頭 2n 個に書く。
// 値の半分で 2 つの部分順列に分けて再帰し、残りを seaweed の走査で埋める。
inline void unit_monge_dmul(const int n, int *stack, const int *a, const int *b) {
  if (n == 1) {
    stack[0] = 0;
    return;
  }
  int *c_row = stack;      // 行 -> 列
  int *c_col = stack + n;  // 列 -> 行
  int *work = stack + 2 * n;

  // 値が [base, base + len) の要素だけを取り出し、その部分順列どうしの積を再帰で解き、
  // 元の行と列の番号に戻して書く。
  auto solve_half = [&](const int len, const int base) {
    int *a_val = work;
    int *a_pos = work + len;
    int *b_val = work + 2 * len;
    int *b_pos = work + 3 * len;
    int na = 0, nb = 0;
    for (int i = 0; i < n; ++i) {
      if (a[i] >= base && a[i] < base + len) {
        a_val[na] = a[i] - base;
        a_pos[na] = i;
        ++na;
      }
    }
    for (int i = 0; i < n; ++i) {
      if (b[i] >= base && b[i] < base + len) {
        b_val[nb] = b[i] - base;
        b_pos[nb] = i;
        ++nb;
      }
    }
    int *c = work + 4 * len;
    unit_monge_dmul(len, c, a_val, b_val);
    for (int i = 0; i < len; ++i) {
      const int row = a_pos[i];
      const int col = b_pos[c[i]];
      c_row[row] = col;
      c_col[col] = row;
    }
  };
  const int mid = n / 2;
  solve_half(mid, 0);        // 値が mid 未満
  solve_half(n - mid, mid);  // 値が mid 以上

  // seaweed の走査。行 n-1 から 0 へ見て、残った行を埋める。
  struct cursor {
    int delta = 0;
    int col = 0;
  };
  int cur_row = n;
  auto right = [&](cursor &c) {
    if (b[c.col] < mid) {
      if (c_col[c.col] >= cur_row) ++c.delta;
    } else {
      if (c_col[c.col] < cur_row) ++c.delta;
    }
    ++c.col;
  };
  auto up = [&](cursor &c) {
    if (a[cur_row] < mid) {
      if (c_row[cur_row] >= c.col) --c.delta;
    } else {
      if (c_row[cur_row] < c.col) --c.delta;
    }
  };
  cursor neg, pos;
  while (cur_row != 0) {
    while (pos.col != n) {
      cursor tmp = pos;
      right(tmp);
      if (tmp.delta == 0) {
        pos = tmp;
      } else {
        break;
      }
    }
    --cur_row;
    up(neg);
    up(pos);
    while (neg.delta != 0) right(neg);
    if (neg.col > pos.col) c_row[cur_row] = pos.col;
  }
}

// a, b は長さ n で、値は [0, n) か absent。unit-Monge の subunit 版の積を返す。
inline vi subunit_monge_dmul(vi a, vi b) {
  const int n = (int)a.size();
  vi a_inv = inverse(a);
  vi b_inv = inverse(b);
  std::swap(b, b_inv);  // b は値 -> 位置になる

  // a は present の値を後ろへ (位置の昇順)、absent の値を前へ (値の昇順) に詰める。
  vi a_map;
  for (int i = n - 1; i >= 0; --i) {
    if (a[i] != absent) {
      a_map.push_back(i);
      a[n - (int)a_map.size()] = a[i];
    }
  }
  std::reverse(a_map.begin(), a_map.end());
  for (int i = 0, cnt = 0; i < n; ++i) {
    if (a_inv[i] == absent) a[cnt++] = i;
  }

  // b は present の値の位置を前へ (値の昇順)、absent の位置を後ろへ (位置の昇順) に詰める。
  vi b_map;
  for (int i = 0; i < n; ++i) {
    if (b[i] != absent) {
      b[b_map.size()] = b[i];
      b_map.push_back(i);
    }
  }
  for (int i = 0, cnt = (int)b_map.size(); i < n; ++i) {
    if (b_inv[i] == absent) b[cnt++] = i;
  }

  vi c(dmul_stack_size(n));
  unit_monge_dmul(n, c.data(), a.data(), b.data());

  vi c_pad(n, absent);
  for (int i = 0; i < (int)a_map.size(); ++i) {
    const int t = c[n - (int)a_map.size() + i];
    if (t < (int)b_map.size()) c_pad[a_map[i]] = b_map[t];
  }
  return c_pad;
}

// 順列 p の seaweed を返す。値は p での位置か absent。
inline vi seaweed_doubling(const vi &p) {
  const int n = (int)p.size();
  if (n == 1) return vi({absent});
  const int mid = n / 2;
  vi lo, hi, lo_map, hi_map;
  for (int i = 0; i < n; ++i) {
    if (p[i] < mid) {
      lo.push_back(p[i]);
      lo_map.push_back(i);
    } else {
      hi.push_back(p[i] - mid);
      hi_map.push_back(i);
    }
  }
  lo = seaweed_doubling(lo);
  hi = seaweed_doubling(hi);
  vi lo_pad(n), hi_pad(n);
  std::iota(lo_pad.begin(), lo_pad.end(), 0);
  std::iota(hi_pad.begin(), hi_pad.end(), 0);
  for (int i = 0; i < mid; ++i) {
    lo_pad[lo_map[i]] = (lo[i] == absent) ? absent : lo_map[lo[i]];
  }
  for (int i = 0; i < n - mid; ++i) {
    hi_pad[hi_map[i]] = (hi[i] == absent) ? absent : hi_map[hi[i]];
  }
  return subunit_monge_dmul(std::move(lo_pad), std::move(hi_pad));
}

// 長さ n の列を bit_length bit の値として持ち、[l, r) の key 未満の数を数える。
class wavelet_matrix {
  int n_ = 0;
  int bit_length_ = 0;
  std::vector<std::vector<int>> ones_;  // ones_[p][i] = 段 p で先頭 i 個の 1 の数

 public:
  wavelet_matrix() = default;

  wavelet_matrix(const int bit_length, std::vector<int> a)
      : n_((int)a.size()), bit_length_(bit_length), ones_(bit_length) {
    for (int p = bit_length - 1; p >= 0; --p) {
      ones_[p].assign(n_ + 1, 0);
      std::vector<int> zero, one;
      zero.reserve(n_);
      one.reserve(n_);
      for (int i = 0; i < n_; ++i) {
        const int b = (a[i] >> p) & 1;
        ones_[p][i + 1] = ones_[p][i] + b;
        (b ? one : zero).push_back(a[i]);
      }
      std::copy(zero.begin(), zero.end(), a.begin());
      std::copy(one.begin(), one.end(), a.begin() + zero.size());
    }
  }

  // a[l, r) のうち key 未満の値の数。
  int count_less(const int l, const int r, const int key) const {
    int res = 0, lo = l, hi = r;
    for (int p = bit_length_ - 1; p >= 0; --p) {
      const int ones_l = ones_[p][lo], ones_r = ones_[p][hi];
      const int zeros_l = lo - ones_l, zeros_r = hi - ones_r;
      if ((key >> p) & 1) {
        res += zeros_r - zeros_l;  // この桁が 0 の要素は key より小さい
        const int z = n_ - ones_[p][n_];
        lo = z + ones_l;
        hi = z + ones_r;
      } else {
        lo = zeros_l;
        hi = zeros_r;
      }
    }
    return res;
  }
};

// 順列 p から seaweed の行を作り、wavelet matrix にする。
inline wavelet_matrix convert(const std::vector<int> &p) {
  const int n = (int)p.size();
  std::vector<int> row;
  if (n != 0) row = seaweed_doubling(p);
  for (int &e : row) {
    if (e == absent) e = n;
  }
  int bit_length = 0;
  for (int t = n; t > 0; t /= 2) ++bit_length;
  return wavelet_matrix(bit_length, std::move(row));
}

}  // namespace static_range_lis_query_impl

struct Solver {
  int n;
  std::vector<std::array<int, 2>> queries;
  static_range_lis_query_impl::wavelet_matrix wm;
  std::vector<int> ans;

  Solver(const std::vector<int> &p, const std::vector<std::array<int, 2>> &qs)
      : n((int)p.size()),
        queries(qs),
        wm(static_range_lis_query_impl::convert(p)),
        ans(qs.size(), 0) {}

  void run() {
    for (std::size_t i = 0; i < queries.size(); ++i) {
      const int l = queries[i][0], r = queries[i][1];
      ans[i] = (r - l) - wm.count_less(l, n, r);
    }
  }

  const std::vector<int> &answers() const { return ans; }
};
