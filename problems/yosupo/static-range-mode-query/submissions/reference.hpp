#pragma once
// Mo's algorithm で解く。構築で値を座標圧縮し、クエリを (l のブロック, r) の順に並べる
// (どちらも計測区間の外)。run() は窓 [curL, curR) を動かしながら各値の頻度を保つ。
//
// 最頻値そのものは、頻度ごとの値の双方向リストで持つ。頻度 f の値のリストの先頭 head[f] を
// 見れば、最頻値は head[best] で O(1) に取れる。窓の移動 1 回はリストの付け替えだけで O(1)、
// 全体で O((N + Q) sqrt N)。
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

struct Solver {
  std::vector<int> a;   // 座標圧縮した列
  std::vector<int> key;  // 圧縮した値 -> 元の値
  // {l, r, 元のクエリ番号} を (l / block, r) の順に並べたもの。
  std::vector<std::array<int, 3>> order;

  std::vector<int> freq;      // 値を添字にした、窓の中の出現回数
  std::vector<int> head;      // 頻度 f の値のリストの先頭。値が無ければ -1
  std::vector<int> prv, nxt;  // 頻度ごとのリストの双方向のつなぎ
  std::vector<std::array<int, 2>> ans;  // {最頻値, 出現回数} をクエリの順に
  int best = 0;               // 窓の中の最大頻度

  Solver(const std::vector<int> &a_,
         const std::vector<std::array<int, 2>> &queries)
      : a(a_), ans(queries.size()) {
    int n = (int)a.size();
    // 座標圧縮。値の大小は問わないので sort + unique で番号を振る。
    key = a;
    std::sort(key.begin(), key.end());
    key.erase(std::unique(key.begin(), key.end()), key.end());
    for (auto &x : a)
      x = (int)(std::lower_bound(key.begin(), key.end(), x) - key.begin());

    // ブロックの幅。l の移動が Q * B、r の移動が (N / B) * N なので B = N / sqrt(Q) が釣り合う。
    int q = (int)queries.size();
    int block = std::max(1, (int)(n / std::max(1.0, std::sqrt((double)q))));
    order.resize(q);
    for (int i = 0; i < q; ++i) order[i] = {queries[i][0], queries[i][1], i};
    std::sort(order.begin(), order.end(),
              [block](const std::array<int, 3> &x, const std::array<int, 3> &y) {
                int bx = x[0] / block, by = y[0] / block;
                if (bx != by) return bx < by;
                // ブロックを 1 つ進むごとに r の向きを変えると、r が端へ戻る回数が減る。
                return (bx & 1) ? x[1] > y[1] : x[1] < y[1];
              });

    int k = (int)key.size();
    freq.assign(k, 0);
    head.assign(n + 1, -1);
    prv.assign(k, -1);
    nxt.assign(k, -1);
  }

  void add(int v) {
    int f = freq[v];
    if (f > 0) {  // 頻度 f のリストから外す
      int p = prv[v], q = nxt[v];
      if (p >= 0) nxt[p] = q; else head[f] = q;
      if (q >= 0) prv[q] = p;
    }
    ++f;
    freq[v] = f;
    // 頻度 f のリストの先頭に繋ぐ
    nxt[v] = head[f];
    prv[v] = -1;
    if (head[f] >= 0) prv[head[f]] = v;
    head[f] = v;
    if (f > best) best = f;
  }

  void remove(int v) {
    int f = freq[v];
    {  // 頻度 f のリストから外す
      int p = prv[v], q = nxt[v];
      if (p >= 0) nxt[p] = q; else head[f] = q;
      if (q >= 0) prv[q] = p;
    }
    --f;
    freq[v] = f;
    if (f > 0) {  // 頻度 f のリストの先頭に繋ぐ
      nxt[v] = head[f];
      prv[v] = -1;
      if (head[f] >= 0) prv[head[f]] = v;
      head[f] = v;
    }
    // 最大頻度の値が居なくなったら 1 下げる。下げた先には今外した値が居る。
    if (f + 1 == best && head[f + 1] < 0) --best;
  }

  void run() {
    int curL = 0, curR = 0;  // 窓は [curL, curR)
    for (const auto &e : order) {
      int l = e[0], r = e[1];
      while (curR < r) add(a[curR++]);
      while (curL > l) add(a[--curL]);
      while (curR > r) remove(a[--curR]);
      while (curL < l) remove(a[curL++]);
      // 窓は空でない (l < r) ので best >= 1 で、head[best] は頻度 best の値。
      ans[e[2]] = {key[head[best]], best};
    }
  }

  const std::vector<std::array<int, 2>> &answers() const { return ans; }
};
