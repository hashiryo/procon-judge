#pragma once
// 参照実装。パターンのトライの頂点を状態にし (状態は今の文字列の接尾辞のうちトライにある最長のもの)、幅優先の順に suffix link を求め
// ながら、子の無い欄を suffix link の先の欄で埋めて、文字の種類ぶんの遷移の表を全部作る。自身か suffix link の先がパターンの終わりの
// 状態を「死んだ状態」とし、長さを 1 ずつ伸ばす DP で死んだ状態に進む遷移を捨てる。O(n S σ) (S は頂点の数)。
#include <array>

struct Solver {
  const vector<string> &ps;
  int sigma;
  Solver(const vector<string> &ps, int sigma) : ps(ps), sigma(sigma) {}
  u32 count(int n) {
    constexpr u32 MOD = 998244353;
    vector<array<int, 26>> go(1);
    go[0].fill(-1);
    vector<char> dead(1, 0);
    for (const auto &p : ps) {
      int v = 0;
      for (const char ch : p) {
        const int c = ch - 'a';
        if (go[v][c] < 0) {
          go[v][c] = (int)go.size();
          go.emplace_back();
          go.back().fill(-1);
          dead.push_back(0);
        }
        v = go[v][c];
      }
      dead[v] = 1;
    }
    const int S = (int)go.size();
    vector<int> link(S, 0), que{0};
    que.reserve(S);
    for (size_t h = 0; h < que.size(); ++h) {
      const int v = que[h];
      for (int c = 0; c < sigma; ++c) {
        const int u = go[v][c];
        if (u >= 0) {
          link[u] = v == 0 ? 0 : go[link[v]][c];
          dead[u] |= dead[link[u]];
          que.push_back(u);
        } else go[v][c] = v == 0 ? 0 : go[link[v]][c];
      }
    }
    vector<u32> dp(S, 0), nx(S);
    dp[0] = 1;
    for (int i = 0; i < n; ++i) {
      fill(nx.begin(), nx.end(), 0u);
      for (int v = 0; v < S; ++v) {
        if (!dp[v] || dead[v]) continue;
        for (int c = 0; c < sigma; ++c) {
          const int u = go[v][c];
          if (dead[u]) continue;
          const u32 x = nx[u] + dp[v];
          nx[u] = x >= MOD ? x - MOD : x;
        }
      }
      dp.swap(nx);
    }
    u64 s = 0;
    for (int v = 0; v < S; ++v) s += dp[v];
    return u32(s % MOD);
  }
};
