#pragma once
// ライブラリを使う提出が共有するもの。K で割った余りが同じ頂点どうししか結べない
// ので、余りごとに分けて商を座標圧縮し、座標ごとに両側の頂点数を数えるところは
// どちらの形式でも同じなので、ここに置く。
#include <algorithm>
#include <unordered_map>
#include <utility>
#include "pj.hpp"
#include "mylib/misc/compress.hpp"

// 余りが同じ頂点の組。a が少ない側 (全部結ぶ側)、b が多い側。
struct Group {
  vector<i64> xs;    // 商 (昇順、重複なし)
  vector<i64> a, b;  // xs[i] を商に持つ、少ない側と多い側の頂点数
};

// 余りごとに分ける。少ない側の頂点がその余りの多い側より多い組があれば結べない
// ので false を返す。
inline bool group_by_residue(i64 k, const vector<i64> &small, const vector<i64> &large,
                             vector<Group> &out) {
  unordered_map<i64, array<vector<i64>, 2>> mp;
  for (i64 v : small) mp[v % k][0].push_back(v / k);
  for (i64 v : large) mp[v % k][1].push_back(v / k);
  out.clear();
  for (auto &[_, arr] : mp) {
    auto &[s, l] = arr;
    if (s.size() > l.size()) return false;
    Group g;
    g.xs = s;
    g.xs.insert(g.xs.end(), l.begin(), l.end());
    auto id = compress(g.xs);
    g.a.assign(g.xs.size(), 0), g.b.assign(g.xs.size(), 0);
    for (i64 v : s) ++g.a[id(v)];
    for (i64 v : l) ++g.b[id(v)];
    out.push_back(std::move(g));
  }
  return true;
}
