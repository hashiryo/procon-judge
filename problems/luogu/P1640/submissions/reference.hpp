#pragma once
// 期待出力を作る参照実装。マッチングを使わず、属性値を頂点、装備を辺とした多重グラフの連結成分で答える (よく知られた
// 別解)。頂点が k 個の成分は、閉路を含めば k 個の値を全部使え、木なら 1 つを除いて全部使える (除く頂点を根にして、
// ほかの頂点に親への辺を割り当てる)。木の成分では、いちばん大きい値を除く。使えない値のうち最小のものの 1 つ手前が答え。
// 装備の無い値は、頂点 1 個の木なので使えない。
#include <numeric>
#include <vector>

struct Solver {
  int n;
  const vector<array<int, 2>> &items;
  Solver(int n, const vector<array<int, 2>> &items) : n(n), items(items) {}
  int run() {
    constexpr int V = 10000;
    vector<int> par(V + 1), mx(V + 1);
    vector<char> cyc(V + 1, 0);
    iota(par.begin(), par.end(), 0);
    iota(mx.begin(), mx.end(), 0);
    auto find = [&](int x) {
      while (par[x] != x) x = par[x] = par[par[x]];
      return x;
    };
    for (auto &[a, b] : items) {
      const int x = find(a), y = find(b);
      if (x == y) cyc[x] = 1;
      else par[x] = y, cyc[y] |= cyc[x], mx[y] = max(mx[y], mx[x]);
    }
    for (int v = 1; v <= V; ++v) {
      const int r = find(v);
      if (!cyc[r] && mx[r] == v) return v - 1;
    }
    return V;
  }
};
