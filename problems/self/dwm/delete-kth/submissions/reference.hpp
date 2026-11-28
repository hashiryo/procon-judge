#pragma once
// 平方分割の参照実装。値は 1 以上 2^30 未満 (問題の制約)。O(Q log V (N/B log B + B))。
#include <algorithm>
#include <vector>

using namespace std;

struct Solver {
  static constexpr int B = 300;
  static constexpr int V = 1 << 30;

  vector<vector<int>> blk;
  vector<vector<int>> srt;
  int n = 0;

  Solver() = default;

  void push_back(int x) {
    if (blk.empty()) {
      blk.push_back({x});
      srt.push_back({x});
      n = 1;
      return;
    }
    int b = (int)blk.size() - 1;
    blk[b].push_back(x);
    srt[b].insert(lower_bound(srt[b].begin(), srt[b].end(), x), x);
    ++n;
    split_if_needed(b);
  }

  void erase(int k) {
    int b, off;
    find(k, b, off);
    int old = blk[b][off];
    blk[b].erase(blk[b].begin() + off);
    auto it = lower_bound(srt[b].begin(), srt[b].end(), old);
    srt[b].erase(it);
    --n;
    if (blk[b].empty()) {
      blk.erase(blk.begin() + b);
      srt.erase(srt.begin() + b);
    } else if (blk[b].size() < (size_t)B / 2) {
      merge_neighbor(b);
    }
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

  void merge_neighbor(int b) {
    if (b + 1 < (int)blk.size() && blk[b].size() + blk[b + 1].size() <= (size_t)2 * B) {
      for (int x : blk[b + 1]) blk[b].push_back(x);
      blk.erase(blk.begin() + b + 1);
      srt[b].insert(srt[b].end(), srt[b + 1].begin(), srt[b + 1].end());
      sort(srt[b].begin(), srt[b].end());
      srt.erase(srt.begin() + b + 1);
    } else if (b > 0 && blk[b - 1].size() + blk[b].size() <= (size_t)2 * B) {
      for (int x : blk[b]) blk[b - 1].push_back(x);
      blk.erase(blk.begin() + b);
      srt[b - 1].insert(srt[b - 1].end(), srt[b].begin(), srt[b].end());
      sort(srt[b - 1].begin(), srt[b - 1].end());
      srt.erase(srt.begin() + b);
    }
  }
};
