#pragma once
// 平方分割の参照実装。位置を B 個ずつの塊に分け、塊ごとに生の列と昇順の写しを持つ。
// 挿入と変更は塊の中の vector をずらし、k 番目は値で二分探索して x 未満の個数を数える。
// 値は 0 以上 70000 以下 (問題の制約)。O(Q log V (N/B log B + B))。
#include <algorithm>
#include <vector>

using namespace std;

struct Solver {
  static constexpr int B = 300;
  static constexpr int V = 70001;  // 値は [0, V)

  vector<vector<int>> blk;
  vector<vector<int>> srt;
  int n = 0;

  explicit Solver(const vector<int> &a) {
    n = (int)a.size();
    for (int i = 0; i < n; i += B) {
      int j = min(n, i + B);
      blk.emplace_back(a.begin() + i, a.begin() + j);
      srt.emplace_back(a.begin() + i, a.begin() + j);
      sort(srt.back().begin(), srt.back().end());
    }
  }

  void insert(int k, int x) {
    if (n == 0) {
      blk.push_back({x});
      srt.push_back({x});
      n = 1;
      return;
    }
    if (k == n) {
      int b = (int)blk.size() - 1;
      blk[b].push_back(x);
      srt[b].insert(lower_bound(srt[b].begin(), srt[b].end(), x), x);
      ++n;
      split_if_needed(b);
      return;
    }
    int b, off;
    find(k, b, off);
    blk[b].insert(blk[b].begin() + off, x);
    srt[b].insert(lower_bound(srt[b].begin(), srt[b].end(), x), x);
    ++n;
    split_if_needed(b);
  }

  void set(int k, int x) {
    int b, off;
    find(k, b, off);
    int old = blk[b][off];
    if (old == x) return;
    blk[b][off] = x;
    auto it = lower_bound(srt[b].begin(), srt[b].end(), old);
    srt[b].erase(it);
    srt[b].insert(lower_bound(srt[b].begin(), srt[b].end(), x), x);
  }

  int kth(int l, int r, int k) const {
    int bl, ol, br, orr;
    find(l, bl, ol);
    find(r - 1, br, orr);
    ++orr;
    auto count_lt = [&](int x) {
      if (bl == br) {
        int c = 0;
        for (int i = ol; i < orr; ++i) c += blk[bl][i] < x;
        return c;
      }
      int c = 0;
      for (int i = ol; i < (int)blk[bl].size(); ++i) c += blk[bl][i] < x;
      for (int b = bl + 1; b < br; ++b)
        c += (int)(lower_bound(srt[b].begin(), srt[b].end(), x) - srt[b].begin());
      for (int i = 0; i < orr; ++i) c += blk[br][i] < x;
      return c;
    };
    int ng = -1, ok = V;
    while (ok - ng > 1) {
      int mid = (ng + ok) / 2;
      if (count_lt(mid) <= k) ng = mid;
      else ok = mid;
    }
    return ng;
  }

 private:
  // k 番目 (0 <= k < n) を含む塊と、その塊の中の位置。
  void find(int k, int &b, int &off) const {
    for (b = 0; b < (int)blk.size(); ++b) {
      if (k < (int)blk[b].size()) {
        off = k;
        return;
      }
      k -= (int)blk[b].size();
    }
    b = (int)blk.size() - 1;
    off = (int)blk[b].size();
  }

  void split_if_needed(int b) {
    if ((int)blk[b].size() <= 2 * B) return;
    int m = (int)blk[b].size() / 2;
    vector<int> lo(blk[b].begin(), blk[b].begin() + m);
    vector<int> hi(blk[b].begin() + m, blk[b].end());
    vector<int> slo = lo, shi = hi;
    sort(slo.begin(), slo.end());
    sort(shi.begin(), shi.end());
    blk[b] = move(lo);
    srt[b] = move(slo);
    blk.insert(blk.begin() + b + 1, move(hi));
    srt.insert(srt.begin() + b + 1, move(shi));
  }
};
