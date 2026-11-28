#pragma once
// 値ごとに位置の集合を持ち、値 x の位置集合の上で [l, r) の個数を数える。
// 更新で値が入れ替わるので、値ごとに「その値が置かれうる位置」(初期値の位置と、
// 更新でその値が入る位置) を集めて座標圧縮し、値ごとに位置の BIT を持つ。
// 位置の総数は N + (タイプ 0 の個数) で 4 * 10^5 以下。
#include <algorithm>
#include <array>
#include <unordered_map>
#include <vector>

struct Solver {
  std::vector<int> a;
  std::vector<std::array<int, 4>> qs;

  // 値の番号 v の位置の一覧は pos[off[v] + 1 .. off[v] + sz[v]] に昇順で並ぶ。
  // bit も同じ並びで、BIT の添字 1..sz[v] がその一覧に対応する。
  std::unordered_map<int, int> id;
  std::vector<int> off, sz, pos, bit;

  std::vector<int> ans;

  Solver(const std::vector<int> &a_, const std::vector<std::array<int, 4>> &qs_)
      : a(a_), qs(qs_) {
    build();
  }

  void build() {
    std::unordered_map<int, int> idx;
    std::vector<std::vector<int>> lists;
    auto get = [&](int v) -> int {
      auto it = idx.find(v);
      if (it != idx.end()) return it->second;
      int k = (int)lists.size();
      idx.emplace(v, k);
      lists.emplace_back();
      return k;
    };
    for (int i = 0; i < (int)a.size(); ++i) lists[get(a[i])].push_back(i);
    for (const auto &e : qs) {
      if (e[0] == 0) lists[get(e[2])].push_back(e[1]);
    }

    const int vals = (int)lists.size();
    off.assign(vals + 1, 0);
    sz.assign(vals, 0);
    for (int v = 0; v < vals; ++v) {
      auto &l = lists[v];
      std::sort(l.begin(), l.end());
      l.erase(std::unique(l.begin(), l.end()), l.end());
      sz[v] = (int)l.size();
      off[v + 1] = off[v] + sz[v] + 1;
    }
    pos.assign(off[vals], 0);
    bit.assign(off[vals], 0);
    for (int v = 0; v < vals; ++v) {
      std::copy(lists[v].begin(), lists[v].end(), pos.begin() + off[v] + 1);
    }
    id = std::move(idx);

    // 初期値の 1 を立てる。
    for (int i = 0; i < (int)a.size(); ++i) add(id[a[i]], i, +1);
  }

  // 値 v の位置 i に delta を足す。i は v の位置の一覧に必ず入っている。
  void add(int v, int i, int delta) {
    const int base = off[v];
    int p = (int)(std::lower_bound(pos.begin() + base + 1,
                                   pos.begin() + base + 1 + sz[v], i) -
                  (pos.begin() + base + 1)) + 1;
    for (int j = p; j <= sz[v]; j += j & -j) bit[base + j] += delta;
  }

  // 値 v で位置が i 未満のものの個数を BIT の先頭からの和で数える。
  int prefix(int v, int i) const {
    const int base = off[v];
    int p = (int)(std::lower_bound(pos.begin() + base + 1,
                                   pos.begin() + base + 1 + sz[v], i) -
                  (pos.begin() + base + 1));
    int s = 0;
    for (int j = p; j > 0; j -= j & -j) s += bit[base + j];
    return s;
  }

  void run() {
    ans.clear();
    for (const auto &e : qs) {
      if (e[0] == 0) {
        const int k = e[1], v = e[2];
        const int old = a[k];
        if (old == v) continue;
        add(id[old], k, -1);
        add(id[v], k, +1);
        a[k] = v;
      } else {
        auto it = id.find(e[3]);
        if (it == id.end()) {
          ans.push_back(0);
          continue;
        }
        const int v = it->second;
        ans.push_back(prefix(v, e[2]) - prefix(v, e[1]));
      }
    }
  }

  const std::vector<int> &answers() const { return ans; }
};
