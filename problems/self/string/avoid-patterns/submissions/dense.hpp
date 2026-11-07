#pragma once
// 遷移の表を全部持ち、状態ごとに σ 個の行き先へ値を足す (押し出す) DP。パターンは最初の子と次の兄弟の連結リストのトライに入れ、
// 幅優先の順に生きた状態 (今の文字列の接尾辞にパターンが無い状態) だけに番号を振りながら、行を suffix link の先の行の写しから作って
// 子の欄を上書きする。死んだ状態への遷移は捨てる欄 (番号 A) に送り、死んだ状態の子は作らない。値は u64 に足し込み、1 歩の終わりに
// 1 回だけ 998244353 で割る。i 歩目に値があるのは深さ i - 1 以下の状態 (幅優先の順の先頭) だけなので、そこまでを回す。
#include "pj.hpp"

struct Solver {
  const vector<string> &ps;
  int sigma;
  Solver(const vector<string> &ps, int sigma) : ps(ps), sigma(sigma) {}
  u32 count(int n) {
    constexpr u64 MOD = 998244353;
    const int sg = sigma;
    // トライ。0 が根。fc は最初の子、ns は次の兄弟、ch は文字、end はパターンの終わり。
    size_t total = 1;
    for (const auto &p : ps) total += p.size();
    vector<int> fc(total, 0), ns(total, 0);
    vector<unsigned char> ch(total, 0), end(total, 0);
    int nodes = 1;
    for (const auto &p : ps) {
      int v = 0;
      for (const char x : p) {
        if (end[v]) break;  // 接頭辞がパターンなら、その先は死んだ状態の子なので要らない
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
    // 生きた状態の遷移の表。行 i は幅優先の順で i 番目の生きた状態。
    const int SINK_GUESS = nodes;  // 生きた状態の数の上限
    vector<int> tab((size_t)SINK_GUESS * sg), org;
    vector<int> link_of;  // 生きた状態の suffix link (番号は幅優先の順)
    vector<int> depth;
    org.reserve(nodes), link_of.reserve(nodes), depth.reserve(nodes);
    // 死んだ状態への欄は、表を作り終えたあとで A に書き換える。それまでは -1 にしておく。
    org.push_back(0), link_of.push_back(0), depth.push_back(0);
    for (size_t h = 0; h < org.size(); ++h) {
      const int v = org[h];
      int *row = &tab[h * sg];
      if (h == 0) fill(row, row + sg, 0);
      else copy_n(&tab[(size_t)link_of[h] * sg], sg, row);
      for (int u = fc[v]; u; u = ns[u]) {
        const int c = ch[u];
        const int lk = h == 0 ? 0 : row[c];  // u の suffix link (row[c] は上書き前なので suffix link の先の行の欄)
        if (end[u] || lk < 0) {
          row[c] = -1;
          continue;
        }
        row[c] = (int)org.size();
        org.push_back(u), link_of.push_back(lk), depth.push_back(depth[h] + 1);
      }
    }
    const int A = (int)org.size();
    for (size_t i = 0; i < (size_t)A * sg; ++i)
      if (tab[i] < 0) tab[i] = A;
    const int D = depth[A - 1];
    vector<int> upto(D + 1, 0);  // upto[d] は深さ d 以下の生きた状態の数
    for (int i = 0; i < A; ++i) upto[depth[i]] = i + 1;
    vector<u32> dp(A + 1, 0);
    vector<u64> acc(A + 1, 0);
    dp[0] = 1;
    for (int i = 1; i <= n; ++i) {
      const int lim = upto[min(i - 1, D)], cur = upto[min(i, D)];
      fill(acc.begin(), acc.begin() + cur, 0);
      for (int q = 0; q < lim; ++q) {
        const u64 x = dp[q];
        if (!x) continue;
        const int *row = &tab[(size_t)q * sg];
        for (int c = 0; c < sg; ++c) acc[row[c]] += x;
      }
      for (int t = 0; t < cur; ++t) dp[t] = u32(acc[t] % MOD);
    }
    u64 s = 0;
    for (int t = 0; t < A; ++t) s += dp[t];
    return u32(s % MOD);
  }
};
