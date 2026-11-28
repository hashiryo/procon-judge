#pragma once
// 平方分割の参照実装。位置を B 個ずつの塊に分け、塊ごとに生の列と昇順の写しを持つ。
// 巡回シフトは末尾の値を抜いて先頭に挿す。個数は塊ごとの二分探索と端の走査で数える。
#include <algorithm>
#include <vector>

using namespace std;

struct Solver {
  static constexpr int B = 300;

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

  void shift(int l, int r) {
    int x = get(r - 1);
    erase_at(r - 1);
    insert_at(l, x);
  }

  int count(int l, int r, int k) const {
    int bl, ol, br, orr;
    find(l, bl, ol);
    find(r - 1, br, orr);
    ++orr;
    if (bl == br) {
      int c = 0;
      for (int i = ol; i < orr; ++i) c += blk[bl][i] == k;
      return c;
    }
    int c = 0;
    for (int i = ol; i < (int)blk[bl].size(); ++i) c += blk[bl][i] == k;
    for (int b = bl + 1; b < br; ++b) {
      auto lo = lower_bound(srt[b].begin(), srt[b].end(), k);
      auto hi = upper_bound(srt[b].begin(), srt[b].end(), k);
      c += (int)(hi - lo);
    }
    for (int i = 0; i < orr; ++i) c += blk[br][i] == k;
    return c;
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

  int get(int k) const {
    int b, off;
    find(k, b, off);
    return blk[b][off];
  }

  void erase_at(int k) {
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

  void insert_at(int k, int x) {
    if (blk.empty()) {
      blk.push_back({x});
      srt.push_back({x});
      n = 1;
      return;
    }
    int b, off;
    find(k, b, off);
    blk[b].insert(blk[b].begin() + off, x);
    srt[b].insert(lower_bound(srt[b].begin(), srt[b].end(), x), x);
    ++n;
    split_if_needed(b);
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
