#pragma once
// failtree の 1 歩の (3) の割り算と、次の歩の (1) の足し込みを、幅優先の逆順の 1 回の走査にまとめたもの。部分木の和は別の配列 acc に
// 足し込み、acc は (2) の走査で 0 に戻す。以下は failtree の説明。
//
// 遷移の表を作らず、suffix link の木の部分木の和で 1 歩を O(S) にする DP (S はトライの頂点の数)。
//
// 状態 q から文字 c で状態 t (親 p、p から c で来る頂点) へ進むのは、p が q の suffix link の木での祖先 (q の接尾辞) で、q と p の
// 間に c の子を持つ頂点が無いときに限る。c の子を持つ頂点 u の部分木は、u の c の子の suffix link が指す頂点へ進むので、
//   新しい値[t] = F(p) - Σ_{suffix link が t を指す頂点 v (親が根でないもの)} F(v の親)
// となる (F(x) は x の suffix link の木での部分木の値の和)。根には、根が子を持たない文字の数 × F(根) から同じく引いたものが来る。
// そこで 1 歩を、(1) 幅優先の逆順に F を suffix link の先へ足し込む、(2) 幅優先の順に、頂点 v に F(v の親) を書き、v の suffix link
// の先から引く、(3) 998244353 で割り、パターンに当たった状態を 0 にする、の 3 回の走査で行う。和は真の整数としては負にならないので
// u64 のまま引いてよい。
//
// トライは最初の子と次の兄弟の連結リストで持ち、suffix link は親の suffix link からたどって求める。パターンに当たった状態の子は
// 作らない (その先へは進まず、引く側にも出てこない)。
#include "pj.hpp"

struct Solver {
  const vector<string> &ps;
  int sigma;
  Solver(const vector<string> &ps, int sigma) : ps(ps), sigma(sigma) {}
  u32 count(int n) {
    constexpr u64 MOD = 998244353;
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
    auto child = [&](int v, unsigned char c) {
      int u = fc[v];
      while (u && ch[u] != c) u = ns[u];
      return u;
    };
    // 幅優先の順に番号を振る。lk はトライの番号での suffix link、id はトライの番号から幅優先の番号へ。
    vector<int> id(nodes, -1), lk(nodes, 0), org, par, link, depth;
    vector<unsigned char> dead;
    org.reserve(nodes), par.reserve(nodes), link.reserve(nodes), depth.reserve(nodes), dead.reserve(nodes);
    id[0] = 0, org.push_back(0), par.push_back(0), link.push_back(0), depth.push_back(0), dead.push_back(0);
    int deg0 = 0;
    for (size_t h = 0; h < org.size(); ++h) {
      if (dead[h]) continue;
      const int v = org[h];
      for (int u = fc[v]; u; u = ns[u]) {
        const unsigned char c = ch[u];
        int l = 0;
        if (v != 0)
          for (int x = lk[v];; x = lk[x]) {
            if (const int y = child(x, c)) {
              l = y;
              break;
            }
            if (x == 0) break;
          }
        else ++deg0;
        lk[u] = l;
        const int j = (int)org.size();
        id[u] = j;
        org.push_back(u), par.push_back((int)h), link.push_back(id[l]), depth.push_back(depth[h] + 1);
        dead.push_back(end[u] | dead[id[l]]);
      }
    }
    const int K = (int)org.size();
    vector<int>().swap(fc), vector<int>().swap(ns), vector<int>().swap(id), vector<int>().swap(lk), vector<int>().swap(org);
    const int D = depth[K - 1];
    vector<int> upto(D + 1, 0);
    for (int i = 0; i < K; ++i) upto[depth[i]] = i + 1;
    vector<u64> mask(K);
    for (int i = 0; i < K; ++i) mask[i] = dead[i] ? 0 : ~0ull;
    const int r1 = upto[min(1, D)];  // 深さ 1 以下の頂点の数 (深さ 1 の頂点の親は根)
    // s[x] は今の値の x の部分木の和 (suffix link の木で)。
    vector<u64> s(K, 0), nw(K, 0), acc(K, 0);
    s[0] = 1;
    for (int i = 1; i <= n; ++i) {
      const int rc = upto[min(i, D)];
      nw[0] = u64(sigma - deg0) * s[0];
      acc[0] = 0;
      for (int v = 1; v < min(r1, rc); ++v) nw[v] = s[0], acc[v] = 0;
      for (int v = r1; v < rc; ++v) {
        const u64 g = s[par[v]];
        nw[v] = g;
        nw[link[v]] -= g;
        acc[v] = 0;
      }
      for (int x = rc - 1; x >= 1; --x) {
        const u64 t = ((nw[x] % MOD) & mask[x]) + acc[x];
        s[x] = t;
        acc[link[x]] += t;
      }
      s[0] = nw[0] % MOD + acc[0];
    }
    return u32(s[0] % MOD);
  }
};
