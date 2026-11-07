#pragma once
// 遷移の表を dense と同じく作り、行き先ごとに「来る状態と、その状態から来る文字の数」の列 (来る状態の順) にまとめ直して、行き先ごとに
// 値を集める (引き寄せる) DP。同じ状態から同じ行き先へ行く文字は 1 つにまとめるので、suffix link の先へ戻る遷移が重なる入力では
// σ 個の欄を全部なめるより少ない。値は u64 に足し込み、行き先ごとに 1 回だけ 998244353 で割る。
#include "pj.hpp"

struct Solver {
  const vector<string> &ps;
  int sigma;
  Solver(const vector<string> &ps, int sigma) : ps(ps), sigma(sigma) {}
  u32 count(int n) {
    constexpr u64 MOD = 998244353;
    const int sg = sigma;
    size_t total = 1;
    for (const auto &p : ps) total += p.size();
    vector<int> fc(total, 0), ns(total, 0);
    vector<unsigned char> ch(total, 0), end(total, 0);
    int nodes = 1;
    for (const auto &p : ps) {
      int v = 0;
      for (const char x : p) {
        if (end[v]) break;
        const unsigned char c = (unsigned char)(x - 'a');
        int u = fc[v];
        while (u && ch[u] != c) u = ns[u];
        if (!u) {
          u = nodes++;
          ch[u] = c, ns[u] = fc[v], fc[v] = u;
        }
        v = u;
      }
      end[v] = 1;
    }
    vector<int> tab((size_t)nodes * sg), org, link_of, depth;
    org.reserve(nodes), link_of.reserve(nodes), depth.reserve(nodes);
    org.push_back(0), link_of.push_back(0), depth.push_back(0);
    for (size_t h = 0; h < org.size(); ++h) {
      const int v = org[h];
      int *row = &tab[h * sg];
      if (h == 0) fill(row, row + sg, 0);
      else copy_n(&tab[(size_t)link_of[h] * sg], sg, row);
      for (int u = fc[v]; u; u = ns[u]) {
        const int c = ch[u];
        const int lk = h == 0 ? 0 : row[c];
        if (end[u] || lk < 0) {
          row[c] = -1;
          continue;
        }
        row[c] = (int)org.size();
        org.push_back(u), link_of.push_back(lk), depth.push_back(depth[h] + 1);
      }
    }
    const int A = (int)org.size();
    // 来る辺の列。e = 来る状態 * 32 + 文字の数 (文字の数は 26 以下)。
    vector<int> stamp(A, -1), pos(A);
    vector<u32> deg(A + 1, 0);
    vector<pair<int, u32>> pairs;  // (行き先, e)
    pairs.reserve((size_t)A * 2);
    for (int q = 0; q < A; ++q) {
      const int *row = &tab[(size_t)q * sg];
      for (int c = 0; c < sg; ++c) {
        const int t = row[c];
        if (t < 0) continue;
        if (stamp[t] != q) {
          stamp[t] = q, pos[t] = (int)pairs.size();
          pairs.emplace_back(t, u32(q) * 32 + 1);
          ++deg[t + 1];
        } else ++pairs[pos[t]].second;
      }
    }
    vector<int>().swap(tab);
    for (int t = 0; t < A; ++t) deg[t + 1] += deg[t];
    vector<u32> in(pairs.size());
    {
      vector<u32> fillp(deg.begin(), deg.end() - 1);
      for (const auto &[t, e] : pairs) in[fillp[t]++] = e;  // q の小さい順に並ぶ
    }
    vector<pair<int, u32>>().swap(pairs);
    const int D = depth[A - 1];
    vector<int> upto(D + 1, 0);
    for (int i = 0; i < A; ++i) upto[depth[i]] = i + 1;
    vector<u32> dp(A, 0), nx(A, 0);
    dp[0] = 1;
    for (int i = 1; i <= n; ++i) {
      const int cur = upto[min(i, D)];
      for (int t = 0; t < cur; ++t) {
        u64 s = 0;
        for (u32 j = deg[t]; j < deg[t + 1]; ++j) {
          const u32 e = in[j];
          s += u64(e & 31) * dp[e >> 5];
        }
        nx[t] = u32(s % MOD);
      }
      dp.swap(nx);
    }
    u64 s = 0;
    for (int t = 0; t < A; ++t) s += dp[t];
    return u32(s % MOD);
  }
};
