#pragma once
// 参照実装。ベクトルを k 組の線形独立な集合に分ける (GF(2) の線形マトロイドの和) のを、Edmonds のマトロイドの分割の
// 算法で求める。提出の共通部分 (problems/_shared/matroid/) を読まずに書いた。
//
// ベクトルを番号の順に 1 本ずつ入れる。どの組にもそのまま入らなければ、交換の列を幅優先で探す。ベクトル u を組 j に
// 入れると、組 j の中で u を作るのに使うベクトル (u の閉路) のどれか y が押し出され、y をほかの組に入れ直す。どこかの組に
// そのまま入るベクトルに着いたら、列に沿って入れ替える。見つからないベクトルは使わない。見つからなかった探索でたどった
// ベクトルは、以後のどの探索の最短の列にも現れないので、以後は飛ばす。
#include <algorithm>
#include <vector>

struct Solver {
  const vector<u64> &v;
  int k;
  vector<int> own;

  Solver(const vector<u64> &vec, int k) : v(vec), k(k) {}

  void run() {
    const int m = v.size();
    own.assign(m, -1);
    // 組ごとの要素 (64 本まで) と、掃き出した行 (行 b は最上位の bit が b) と、行が組の何番目の要素の xor かの印
    vector<vector<int>> mem(k);
    vector<array<u64, 64>> row(k), how(k);
    vector<char> stale(k, 1), dead(m, 0);
    vector<int> pos(m, -1), from(m), q, seen(m, 0);
    int stamp = 0;
    auto rebuild = [&](int j) {
      if (!stale[j]) return;
      stale[j] = 0;
      row[j].fill(0), how[j].fill(0);
      for (int t = 0; t < (int)mem[j].size(); ++t) {
        u64 a = v[mem[j][t]], h = 1ULL << t;
        while (a) {
          const int b = 63 - __builtin_clzll(a);
          if (!row[j][b]) {
            row[j][b] = a, how[j][b] = h;
            break;
          }
          a ^= row[j][b], h ^= how[j][b];
        }
      }
    };
    // x を組 j の行で消す。消えなければ ~0、消えれば使った要素の印
    auto reduce = [&](int j, int x) {
      u64 a = v[x], h = 0;
      while (a) {
        const int b = 63 - __builtin_clzll(a);
        if (!row[j][b]) return ~0ULL;
        a ^= row[j][b], h ^= how[j][b];
      }
      return h;
    };
    auto put = [&](int x, int j) {
      if (own[x] >= 0) {
        auto &s = mem[own[x]];
        const int last = s.back();
        s[pos[x]] = last, pos[last] = pos[x], s.pop_back();
        stale[own[x]] = 1;
      }
      own[x] = j;
      if (j >= 0) pos[x] = mem[j].size(), mem[j].push_back(x), stale[j] = 1;
    };
    for (int s = 0; s < m; ++s) {
      for (int j = 0; j < k; ++j) rebuild(j);
      ++stamp;
      seen[s] = stamp, from[s] = -1;
      q.assign(1, s);
      int got = -1, got_j = -1;
      for (size_t h = 0; h < q.size() && got < 0; ++h) {
        const int u = q[h];
        for (int j = 0; j < k; ++j) {
          if (j == own[u]) continue;
          const u64 c = reduce(j, u);
          if (c == ~0ULL) {
            got = u, got_j = j;
            break;
          }
          for (u64 t = c; t; t &= t - 1) {
            const int y = mem[j][__builtin_ctzll(t)];
            if (seen[y] != stamp && !dead[y]) seen[y] = stamp, from[y] = u, q.push_back(y);
          }
        }
      }
      if (got < 0) {
        for (int y : q) dead[y] = 1;
        continue;
      }
      for (int x = got, j = got_j; x >= 0;) {
        const int old = own[x], p = from[x];
        put(x, j);
        j = old, x = p;
      }
    }
  }

  vector<int> answer() const { return own; }
};
