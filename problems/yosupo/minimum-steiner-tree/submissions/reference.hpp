#pragma once
// 最小 Steiner 木。Dreyfus-Wagner のビット DP。
//   dp[S][v] = ターミナル集合 S を v で結ぶ部分木の最小重み
// S を空でない 2 つに分けて同じ v で併合し、そのあと辺をたどって v を動かす。
// 各 S の緩めは、N が小さいのでヒープを使わない O(N^2 + M) のダイクストラで行う。
// K <= 12 なので、分割の列挙は全体で 3^K 通り、状態は 2^K * N 個に収まる。
#include <algorithm>
#include <utility>
#include <vector>

struct Solver {
  int n;
  std::vector<int> u, v, xs;
  std::vector<long long> w;
  std::vector<std::vector<std::pair<int, int>>> g;  // (行き先, 辺の番号)
  std::vector<int> chosen;
  long long ans_w = 0;

  Solver(int n_, const std::vector<int> &u_, const std::vector<int> &v_,
         const std::vector<long long> &w_, const std::vector<int> &xs_)
      : n(n_), u(u_), v(v_), xs(xs_), w(w_) {}

  void run() {
    const int k = (int)xs.size();
    const int m = (int)u.size();
    const long long INF = 1LL << 62;

    g.assign(n, {});
    for (int i = 0; i < m; ++i) {
      g[u[i]].push_back({v[i], i});
      g[v[i]].push_back({u[i], i});
    }

    const int full = (1 << k) - 1;
    const size_t sz = (size_t)(full + 1) * n;
    std::vector<long long> dp(sz, INF);
    // pa, pb は復元用。(A, -1) なら S を A と S^A に分けた、(頂点, 辺) なら辺で来た。
    std::vector<int> pa(sz, -1), pb(sz, -1);
    std::vector<char> done(n, 0);

    for (int S = 1; S <= full; ++S) {
      long long *d = &dp[(size_t)S * n];
      int *ea = &pa[(size_t)S * n];
      int *eb = &pb[(size_t)S * n];
      if ((S & (S - 1)) == 0) {
        const int i = __builtin_ctz((unsigned)S);
        d[xs[i]] = 0;
      } else {
        const int lsb = S & -S;
        const int rest = S ^ lsb;
        // 最小のビットを含む側を A に固定して、同じ分け方を 2 回見ない。
        for (int t = rest;; t = (t - 1) & rest) {
          const int A = t | lsb;
          if (A != S) {
            const long long *da = &dp[(size_t)A * n];
            const long long *db = &dp[(size_t)(S ^ A) * n];
            for (int x = 0; x < n; ++x) {
              if (da[x] == INF || db[x] == INF) continue;
              const long long c = da[x] + db[x];
              if (c < d[x]) d[x] = c, ea[x] = A, eb[x] = -1;
            }
          }
          if (t == 0) break;
        }
      }
      std::fill(done.begin(), done.end(), 0);
      for (int it = 0; it < n; ++it) {
        int bv = -1;
        long long bd = INF;
        for (int x = 0; x < n; ++x)
          if (!done[x] && d[x] < bd) bd = d[x], bv = x;
        if (bv < 0) break;
        done[bv] = 1;
        for (const auto &e : g[bv]) {
          const long long nd = d[bv] + w[e.second];
          if (nd < d[e.first]) d[e.first] = nd, ea[e.first] = bv, eb[e.first] = e.second;
        }
      }
    }

    const long long *dfull = &dp[(size_t)full * n];
    int best = 0;
    for (int x = 1; x < n; ++x)
      if (dfull[x] < dfull[best]) best = x;

    std::vector<char> used(m, 0);
    std::vector<std::pair<int, int>> st;
    st.reserve((size_t)1 << (k + 1));
    st.push_back({full, best});
    while (!st.empty()) {
      const std::pair<int, int> cur = st.back();
      st.pop_back();
      const int S = cur.first, r = cur.second;
      const size_t idx = (size_t)S * n + r;
      const int a = pa[idx], b = pb[idx];
      if (a < 0) continue;  // ターミナル 1 個の基点
      if (b < 0) {
        st.push_back({a, r});
        st.push_back({S ^ a, r});
      } else {
        used[b] = 1;
        st.push_back({S, a});
      }
    }

    chosen.clear();
    ans_w = 0;
    for (int i = 0; i < m; ++i)
      if (used[i]) chosen.push_back(i), ans_w += w[i];
  }

  long long weight() const { return ans_w; }
  const std::vector<int> &edges() const { return chosen; }
};
