#pragma once
// 区間並列の併合。各クエリの辺 (a+j, b+j) (0 <= j < k) を、長さ 2^t の区間どうしの
// 併合に分ける。レベル t に「長さ 2^t の区間」の UnionFind を持ち、2 つの区間を
// 併合したら下のレベルへ降りて位置ごとに併合する。レベル t で既に同じ成分なら、
// その区間の位置はすべて併合済みなので降りなくてよい。
//
// X = sum_{i<j, same(i,j)} x_i x_j は、成分の (x の和) を持ち、2 つの成分を併合する
// たびに (和1) * (和2) を足せばよい。
#include <array>
#include <vector>

struct Solver {
  static constexpr int MOD = 998244353;

  int n;
  std::vector<int> x;
  std::vector<std::array<int, 3>> qs;
  std::vector<int> ans;

  // par[t][v] は、レベル t の区間 [v, v+2^t) の DSU。根は -大きさ、それ以外は親。
  // レベル 0 はそのまま位置の DSU。
  std::vector<std::vector<int>> par;
  std::vector<int> sum;  // 成分の x の和 (mod MOD)。根だけ見る。
  long long cur = 0;     // 今の X mod MOD

  Solver(int n_, std::vector<int> x_, std::vector<std::array<int, 3>> qs_)
      : n(n_), x(std::move(x_)), qs(std::move(qs_)) {}

  int find(int t, int v) {
    std::vector<int> &p = par[t];
    int r = v;
    while (p[r] >= 0) r = p[r];
    while (v != r) {
      int u = p[v];
      p[v] = r;
      v = u;
    }
    return r;
  }

  // 位置 a と b の成分を併合し、X を足す。
  void unite_positions(int a, int b) {
    int ra = find(0, a), rb = find(0, b);
    if (ra == rb) return;
    cur += (long long)sum[ra] * sum[rb] % MOD;
    if (cur >= MOD) cur -= MOD;
    int s = sum[ra] + sum[rb];
    if (s >= MOD) s -= MOD;
    std::vector<int> &p = par[0];
    if (p[ra] > p[rb]) std::swap(ra, rb);  // p[根] は -大きさ
    p[ra] += p[rb];
    p[rb] = ra;
    sum[ra] = s;
  }

  // 長さ 2^t の区間 [a, a+2^t) と [b, b+2^t) を位置ごとに併合する。
  void merge_blocks(int t, int a, int b) {
    if (t == 0) {
      unite_positions(a, b);
      return;
    }
    std::vector<int> &p = par[t];
    int ra = find(t, a), rb = find(t, b);
    if (ra == rb) return;
    if (p[ra] > p[rb]) std::swap(ra, rb);
    p[ra] += p[rb];
    p[rb] = ra;
    const int half = 1 << (t - 1);
    merge_blocks(t - 1, a, b);
    merge_blocks(t - 1, a + half, b + half);
  }

  void run() {
    int levels = 1;
    while ((1 << levels) <= n) ++levels;
    par.assign(levels, {});
    for (int t = 0; t < levels; ++t) par[t].assign(n - (1 << t) + 1, -1);
    sum = x;
    cur = 0;
    ans.clear();
    ans.reserve(qs.size());
    for (const auto &e : qs) {
      const int k = e[0], a = e[1], b = e[2];
      // 長さ k の区間 [0, k) を 2 の冪の区間に分ける (高々 popcount(k) 個)。
      int done = 0;
      while (done < k) {
        const int t = 31 - __builtin_clz(k - done);
        merge_blocks(t, a + done, b + done);
        done += 1 << t;
      }
      ans.push_back((int)cur);
    }
  }

  const std::vector<int> &answers() const { return ans; }
};
