#pragma once
#include <algorithm>
#include <array>
#include <vector>

// 重心分解で解く。値の加算は点、問い合わせは「p からの距離が [l, r) の頂点の和」。
//
// p の重心木の祖先を A[0] (= 重心木の根), ..., A[k] (= p) と並べる。各 A[i] の成分について
//
//   f[i] = 成分 A[i] の頂点を dist(·, A[i]) で並べた列
//   g[i] = 成分 A[i] の頂点を dist(·, A[i-1]) = dist(·, 親の重心) で並べた列  (i >= 1)
//
// を持つ。クエリの和は
//
//   Σ_i ( f[i] の [l - dist(p, A[i]), r - dist(p, A[i])) の和
//        - g[i+1] の同じ区間の和 )
//
// になる。g[i+1] は「A[i] から見て p 側の枝」を A[i] からの距離で並べた列なので、
// 引くと枝の中だけが落ちる。f と g の区間は同じ [l - dist(p, A[i]), r - dist(p, A[i]))
// で、これを全ての i で足し引きすると、各頂点 u は「u を含む最も深い A[i]」の 1 項だけが
// 残る。そこでは A[i] が p と u の重心木の LCA なので dist(p, u) の式が正しい。
//
// f と g は 1 本の距離列に詰め、区間ごとに独立した Fenwick 木にする。添字の歩幅を
// 区間の長さで止めるので、木の深さはその区間の長さの対数で済む。区間の先頭 1 個は
// Fenwick 木の添字 0 として空ける。
//
// 計算量は構築 O(N log^2 N)、クエリと加算が O(log^2 N)。

namespace varcst {

// 1 本の配列に複数の Fenwick 木を区間で並べる。区間 [base, base + sz] を使い、
// base は添字 0 として空ける。値は base + 1 から base + sz に入る。
struct FlatFenwick {
  std::vector<long long> d;

  void init(int n) { d.assign(n, 0); }

  // slot は base + 1 + (0 起点の順位)。
  void add(int base, int sz, int slot, long long x) {
    for (int k = slot - base; k <= sz; k += k & -k) d[base + k] += x;
  }

  // 順位 [0, r) の和。
  long long prefix(int base, int r) const {
    long long s = 0;
    for (; r > 0; r -= r & -r) s += d[base + r];
    return s;
  }
};

}  // namespace varcst

struct Solver {
  struct Ent {
    int fbase, fsz;  // f の区間
    int gbase, gsz;  // g の区間。重心木の根は 0
    int dist;        // dist(p, A[i])
    int posF, posG;  // 加算で触る位置 (f, g の中の絶対位置)
  };

  int n;
  std::vector<int> adj_start, adj_list;
  std::vector<int> darr;  // 区間ごとの距離列。各要素は 0 起点の順位で並ぶ
  std::vector<std::vector<Ent>> ents;
  const std::vector<std::array<long long, 4>> &queries;
  std::vector<long long> ans;
  varcst::FlatFenwick fw;

  Solver(int n_, const std::vector<long long> &a, const std::vector<int> &eu,
         const std::vector<int> &ev,
         const std::vector<std::array<long long, 4>> &queries_)
      : n(n_), queries(queries_) {
    // 隣接リストを CSR で作る。重心分解は辺を何度もなぞるので、配列 1 本にしておく。
    std::vector<int> cnt(n, 0);
    for (int i = 0; i + 1 < n; ++i) {
      ++cnt[eu[i]];
      ++cnt[ev[i]];
    }
    adj_start.assign(n + 1, 0);
    for (int i = 0; i < n; ++i) adj_start[i + 1] = adj_start[i] + cnt[i];
    adj_list.resize(adj_start[n]);
    std::vector<int> cur(adj_start.begin(), adj_start.end() - 1);
    for (int i = 0; i + 1 < n; ++i) {
      adj_list[cur[eu[i]]++] = ev[i];
      adj_list[cur[ev[i]]++] = eu[i];
    }

    build();
    // 初期値を入れる。計測区間の外なので、ここで足してよい。
    for (int u = 0; u < n; ++u)
      if (a[u] != 0) add_value(u, a[u]);
  }

  // 重心分解。成分を (成分の根, 親の重心) の組で積む。根は親の重心の隣接頂点なので、
  // 親の重心からの距離は「根からの距離 + 1」で作れる。
  void build() {
    ents.assign(n, std::vector<Ent>());
    darr.reserve((size_t)n * 36);

    std::vector<char> removed(n, 0);
    std::vector<int> par(n, -1), sz(n, 0), par2(n, -1);
    std::vector<int> distc(n, 0), distr(n, 0), posF(n, 0), posG(n, 0);
    std::vector<int> comp, comp2;

    std::vector<std::pair<int, int>> stk;
    stk.push_back({0, -1});
    while (!stk.empty()) {
      const int r = stk.back().first, pc = stk.back().second;
      stk.pop_back();

      // 成分を集める。ついでに r を根とする親子関係も入る。
      comp.clear();
      comp.push_back(r);
      par[r] = -1;
      for (size_t i = 0; i < comp.size(); ++i) {
        const int u = comp[i];
        for (int e = adj_start[u]; e < adj_start[u + 1]; ++e) {
          const int v = adj_list[e];
          if (removed[v] || v == par[u]) continue;
          par[v] = u;
          comp.push_back(v);
        }
      }
      const int total = (int)comp.size();

      // 部分木の大きさ。comp は親が先に並ぶので、逆順で足せる。
      for (int i = total - 1; i >= 0; --i) {
        const int u = comp[i];
        int s = 1;
        for (int e = adj_start[u]; e < adj_start[u + 1]; ++e) {
          const int v = adj_list[e];
          if (!removed[v] && par[v] == u) s += sz[v];
        }
        sz[u] = s;
      }

      // 重心。どの隣を切っても残りが半分以下になる頂点。
      int c = r;
      for (int i = 0; i < total; ++i) {
        const int u = comp[i];
        int mx = total - sz[u];
        for (int e = adj_start[u]; e < adj_start[u + 1]; ++e) {
          const int v = adj_list[e];
          if (!removed[v] && par[v] == u && sz[v] > mx) mx = sz[v];
        }
        if (mx * 2 <= total) {
          c = u;
          break;
        }
      }

      // c からの距離。
      comp2.clear();
      comp2.push_back(c);
      par2[c] = -1;
      distc[c] = 0;
      for (size_t i = 0; i < comp2.size(); ++i) {
        const int u = comp2[i];
        for (int e = adj_start[u]; e < adj_start[u + 1]; ++e) {
          const int v = adj_list[e];
          if (removed[v] || v == par2[u]) continue;
          par2[v] = u;
          distc[v] = distc[u] + 1;
          comp2.push_back(v);
        }
      }

      // f。c からの距離で並べる。同じ距離は頂点番号で割って順序を固定する。
      std::sort(comp2.begin(), comp2.end(), [&](int x, int y) {
        return distc[x] != distc[y] ? distc[x] < distc[y] : x < y;
      });
      const int fbase = (int)darr.size();
      darr.resize(fbase + total + 1);
      for (int i = 0; i < total; ++i) {
        const int u = comp2[i];
        darr[fbase + 1 + i] = distc[u];
        posF[u] = fbase + 1 + i;
      }

      // g。親の重心 pc からの距離は、r からの距離 + 1。
      int gbase = 0, gsz = 0;
      if (pc >= 0) {
        distr[r] = 0;
        for (size_t i = 1; i < comp.size(); ++i) {
          const int u = comp[i];
          distr[u] = distr[par[u]] + 1;
        }
        std::sort(comp.begin(), comp.end(), [&](int x, int y) {
          const int dx = distr[x], dy = distr[y];
          return dx != dy ? dx < dy : x < y;
        });
        gbase = (int)darr.size();
        darr.resize(gbase + total + 1);
        for (int i = 0; i < total; ++i) {
          const int u = comp[i];
          darr[gbase + 1 + i] = distr[u] + 1;
          posG[u] = gbase + 1 + i;
        }
        gsz = total;
      }

      // この重心の項を、成分の全頂点の祖先列に足す。祖先列は根から p に向かう順に積まれる。
      for (int i = 0; i < total; ++i) {
        const int u = comp2[i];
        Ent e;
        e.fbase = fbase;
        e.fsz = total;
        e.gbase = gbase;
        e.gsz = gsz;
        e.dist = distc[u];
        e.posF = posF[u];
        e.posG = (pc >= 0) ? posG[u] : 0;
        ents[u].push_back(e);
      }

      removed[c] = 1;
      for (int e = adj_start[c]; e < adj_start[c + 1]; ++e) {
        const int v = adj_list[e];
        if (!removed[v]) stk.push_back({v, c});
      }
    }

    fw.init((int)darr.size());
  }

  void add_value(int p, long long x) {
    const std::vector<Ent> &es = ents[p];
    for (size_t i = 0; i < es.size(); ++i) {
      fw.add(es[i].fbase, es[i].fsz, es[i].posF, x);
      // g[i] は i >= 1 のときだけ意味を持つ (i-1 の項の「枝」を引くため)。
      if (i > 0) fw.add(es[i].gbase, es[i].gsz, es[i].posG, x);
    }
  }

  long long range_sum(int p, int l, int r) const {
    const std::vector<Ent> &es = ents[p];
    long long res = 0;
    for (size_t i = 0; i < es.size(); ++i) {
      const int lo = l - es[i].dist, hi = r - es[i].dist;
      if (hi <= 0) continue;
      res += seg(fw, darr, es[i].fbase, es[i].fsz, lo, hi);
      if (i + 1 < es.size()) {
        const Ent &g = es[i + 1];
        res -= seg(fw, darr, g.gbase, g.gsz, lo, hi);
      }
    }
    return res;
  }

  // 区間 [base, base + sz] の距離列で、値が [lo, hi) に入る順位の幅の和。
  static long long seg(const varcst::FlatFenwick &f, const std::vector<int> &d,
                       int base, int sz, int lo, int hi) {
    const int *b = d.data() + base + 1;
    const int i1 = (int)(std::lower_bound(b, b + sz, lo) - b);
    const int i2 = (int)(std::lower_bound(b, b + sz, hi) - b);
    if (i1 >= i2) return 0;
    return f.prefix(base, i2) - f.prefix(base, i1);
  }

  void run() {
    ans.clear();
    ans.reserve(queries.size());
    for (const auto &e : queries) {
      if (e[0] == 0) {
        add_value((int)e[1], e[2]);
      } else {
        ans.push_back(range_sum((int)e[1], (int)e[2], (int)e[3]));
      }
    }
  }

  const std::vector<long long> &answers() const { return ans; }
};
