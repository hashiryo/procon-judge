#pragma once
// 平方分割の参照実装。位置を B 個ずつの塊に分け、塊ごとに生の列と昇順の列、昇順の累積和を持つ。
// 問いは、値 x より大きい要素の和と個数を塊ごとの二分探索で数え、和が k 以上になる境目の値 v を
// 二分探索と最小値で求め、v の要素を何個足すかで答える。
#include <algorithm>
#include <vector>

using namespace std;

struct Solver {
  static constexpr int B = 300;
  static constexpr int VMAX = 1000000000;

  vector<vector<int>> blk;
  vector<vector<int>> srt;
  vector<vector<long long>> pre;
  vector<long long> bsum;
  int n = 0;

  explicit Solver(const vector<int> &a) {
    n = (int)a.size();
    for (int i = 0; i < n; i += B) {
      int j = min(n, i + B);
      blk.emplace_back(a.begin() + i, a.begin() + j);
      srt.emplace_back(a.begin() + i, a.begin() + j);
      pre.emplace_back();
      bsum.push_back(0);
      rebuild((int)blk.size() - 1);
    }
  }

  void set(int c, int x) {
    int b, off;
    find(c, b, off);
    blk[b][off] = x;
    rebuild(b);
  }

  int query(int l, int r, long long k) const {
    if (range_sum(l, r) < k) return -1;
    int lo = 0, hi = VMAX + 1;
    while (hi - lo > 1) {
      int mid = lo + (hi - lo) / 2;
      if (sum_gt(l, r, mid) >= k) lo = mid;
      else hi = mid;
    }
    int v = min_gt(l, r, lo);
    long long s1 = sum_gt(l, r, v);
    int m1 = cnt_gt(l, r, v);
    long long need = k - s1;
    return m1 + (int)((need + v - 1) / v);
  }

 private:
  void rebuild(int b) {
    srt[b] = blk[b];
    sort(srt[b].begin(), srt[b].end());
    int m = (int)srt[b].size();
    pre[b].assign(m + 1, 0);
    for (int i = 0; i < m; ++i) pre[b][i + 1] = pre[b][i] + srt[b][i];
    bsum[b] = pre[b][m];
  }

  void find(int c, int &b, int &off) const {
    for (b = 0; b < (int)blk.size(); ++b) {
      if (c < (int)blk[b].size()) {
        off = c;
        return;
      }
      c -= (int)blk[b].size();
    }
    b = (int)blk.size() - 1;
    off = (int)blk[b].size();
  }


  long long range_sum(int l, int r) const {
    int bl, ol, br, orr;
    find(l, bl, ol);
    find(r - 1, br, orr);
    ++orr;
    if (bl == br) {
      long long s = 0;
      for (int i = ol; i < orr; ++i) s += blk[bl][i];
      return s;
    }
    long long s = 0;
    for (int i = ol; i < (int)blk[bl].size(); ++i) s += blk[bl][i];
    for (int b = bl + 1; b < br; ++b) s += bsum[b];
    for (int i = 0; i < orr; ++i) s += blk[br][i];
    return s;
  }

  long long sum_gt(int l, int r, int x) const {
    int bl, ol, br, orr;
    find(l, bl, ol);
    find(r - 1, br, orr);
    ++orr;
    if (bl == br) {
      long long s = 0;
      for (int i = ol; i < orr; ++i) s += blk[bl][i] > x ? blk[bl][i] : 0;
      return s;
    }
    long long s = 0;
    for (int i = ol; i < (int)blk[bl].size(); ++i) s += blk[bl][i] > x ? blk[bl][i] : 0;
    for (int b = bl + 1; b < br; ++b) {
      int idx = (int)(upper_bound(srt[b].begin(), srt[b].end(), x) - srt[b].begin());
      s += pre[b].back() - pre[b][idx];
    }
    for (int i = 0; i < orr; ++i) s += blk[br][i] > x ? blk[br][i] : 0;
    return s;
  }

  int cnt_gt(int l, int r, int x) const {
    int bl, ol, br, orr;
    find(l, bl, ol);
    find(r - 1, br, orr);
    ++orr;
    if (bl == br) {
      int c = 0;
      for (int i = ol; i < orr; ++i) c += blk[bl][i] > x;
      return c;
    }
    int c = 0;
    for (int i = ol; i < (int)blk[bl].size(); ++i) c += blk[bl][i] > x;
    for (int b = bl + 1; b < br; ++b) {
      int idx = (int)(upper_bound(srt[b].begin(), srt[b].end(), x) - srt[b].begin());
      c += (int)srt[b].size() - idx;
    }
    for (int i = 0; i < orr; ++i) c += blk[br][i] > x;
    return c;
  }

  int min_gt(int l, int r, int x) const {
    int bl, ol, br, orr;
    find(l, bl, ol);
    find(r - 1, br, orr);
    ++orr;
    int res = VMAX + 1;
    if (bl == br) {
      for (int i = ol; i < orr; ++i)
        if (blk[bl][i] > x && blk[bl][i] < res) res = blk[bl][i];
      return res;
    }
    for (int i = ol; i < (int)blk[bl].size(); ++i)
      if (blk[bl][i] > x && blk[bl][i] < res) res = blk[bl][i];
    for (int b = bl + 1; b < br; ++b) {
      int idx = (int)(upper_bound(srt[b].begin(), srt[b].end(), x) - srt[b].begin());
      if (idx < (int)srt[b].size() && srt[b][idx] < res) res = srt[b][idx];
    }
    for (int i = 0; i < orr; ++i)
      if (blk[br][i] > x && blk[br][i] < res) res = blk[br][i];
    return res;
  }
};
