#pragma once
#include "pj.hpp"

// 重心分解。クエリは「p からの距離が [l, r) の頂点に x を足す」と「a_p を出す」。
//
// 重心 c ごとに、c の成分の頂点を c からの距離で並べた列を持つ。列に要るのは
// 区間加算と 1 点取得だけなので、差分の Fenwick 木で持つ。c の成分を、c を
// 取り除いてできる子成分ごとにも同じ列を持つ。
//
// 加算 (p, l, r, x) は p の重心祖先 c を回り、d = dist(p, c) として
//   全体の列に [l - d, r - d) を +x、p を含む子成分の列に同じ区間を -x
// と足す。取得 (p) は p の重心祖先 c を回り、全体の列の index dist(p, c) と
// 子成分の列の同じ index を足す (子成分の列には -x が入っているので、符号は
// そのままでよい)。
//
// 足した x が残るのは、重心木で p と v の LCA が c のところだけで、そこでは
// 実際のパスが c を通るので dist(p, v) = dist(p, c) + dist(c, v) になる。c より
// 上では v が p と同じ子成分に入るので、+x と -x が同じ距離で打ち消し合う。
// 距離の上限は N なので、和は 64 bit。
struct Solver {
  struct Rec {
    int cid;    // 重心の番号。全体の列の番号でもある。
    int dist;   // その重心からの距離。
    int child;  // p を含む子成分の列の番号。p 自身が重心なら -1。
  };

  int n, stride;
  vector<i64> a;             // 初期値。答えは a[p] に足された分を足したもの。
  vector<int> adj_off, adj;  // 隣接の CSR
  vector<Rec> recs;          // 頂点ごとに stride 個の枠
  vector<int> rec_cnt;       // 頂点ごとの重心祖先の数
  vector<int> seq_len;       // 列の長さ。番号 0..n-1 が重心、n.. が子成分。
  vector<int> seq_off;       // Fenwick 木の先頭。列 i は fw[seq_off[i] + 1..+len]。
  vector<i64> fw;            // 全列を 1 本に繋いだ差分の Fenwick 木
  const vector<array<i64, 5>> &queries;
  vector<i64> ans;

  Solver(int n_, const vector<i64> &a_, const vector<pair<int, int>> &edges,
         const vector<array<i64, 5>> &qs)
      : n(n_), a(a_), queries(qs) {
    // 重心分解の深さは log2(n) + 1 以下なので、その分の枠を頂点ごとに取る。
    stride = 1;
    while ((1 << stride) < n) ++stride;
    ++stride;

    adj_off.assign(n + 1, 0);
    for (const auto &e : edges) ++adj_off[e.first + 1], ++adj_off[e.second + 1];
    for (int i = 0; i < n; ++i) adj_off[i + 1] += adj_off[i];
    adj.resize(size_t(2) * edges.size());
    {
      vector<int> pos(adj_off.begin(), adj_off.end() - 1);
      for (const auto &e : edges) {
        adj[pos[e.first]++] = e.second;
        adj[pos[e.second]++] = e.first;
      }
    }

    recs.assign(size_t(n) * stride, Rec{0, 0, -1});
    rec_cnt.assign(n, 0);
    seq_len.assign(size_t(2) * n, 0);
    build_centroid();

    seq_off.assign(size_t(2) * n + 1, 0);
    for (size_t i = 0; i < size_t(2) * n; ++i) seq_off[i + 1] = seq_off[i] + seq_len[i] + 1;
    fw.assign(seq_off[size_t(2) * n], 0);

    int gets = 0;
    for (const auto &e : queries)
      if (e[0] != 0) ++gets;
    ans.reserve(gets);
  }

  // 重心分解して、頂点ごとに (重心, 距離, 子成分) を積む。ついでに列の長さを出す。
  void build_centroid() {
    vector<char> removed(n, 0);
    vector<int> par(n, -1), size(n), dist(n), child(n), child_seq(n, -1);
    vector<int> order, bfs, stack;
    order.reserve(n), bfs.reserve(n);
    stack.push_back(0);
    int nseq = n;  // 子成分の列の番号は n から
    while (!stack.empty()) {
      const int entry = stack.back();
      stack.pop_back();
      // 成分の頂点を集め、(entry を根にした) 親を付ける。
      order.clear();
      order.push_back(entry);
      par[entry] = -1;
      for (size_t i = 0; i < order.size(); ++i) {
        const int u = order[i];
        for (int j = adj_off[u]; j < adj_off[u + 1]; ++j) {
          const int v = adj[j];
          if (removed[v] || v == par[u]) continue;
          par[v] = u;
          order.push_back(v);
        }
      }
      const int total = (int)order.size();
      for (int i = total - 1; i >= 0; --i) {
        const int u = order[i];
        size[u] = 1;
        for (int j = adj_off[u]; j < adj_off[u + 1]; ++j) {
          const int v = adj[j];
          if (removed[v] || par[v] != u) continue;
          size[u] += size[v];
        }
      }
      // どの子部分木も total/2 以下になる頂点が重心。
      int c = -1;
      for (const int u : order) {
        int mx = total - size[u];
        for (int j = adj_off[u]; j < adj_off[u + 1]; ++j) {
          const int v = adj[j];
          if (removed[v] || par[v] != u) continue;
          if (size[v] > mx) mx = size[v];
        }
        if (mx * 2 <= total) {
          c = u;
          break;
        }
      }
      // 重心から BFS して距離と、どの子成分に入るかを振る。
      bfs.clear();
      bfs.push_back(c);
      par[c] = -1;
      dist[c] = 0;
      child[c] = -1;
      for (int j = adj_off[c]; j < adj_off[c + 1]; ++j) {
        const int v = adj[j];
        if (!removed[v]) child_seq[v] = nseq++;
      }
      for (size_t i = 0; i < bfs.size(); ++i) {
        const int u = bfs[i];
        for (int j = adj_off[u]; j < adj_off[u + 1]; ++j) {
          const int v = adj[j];
          if (removed[v] || v == par[u]) continue;
          par[v] = u;
          dist[v] = dist[u] + 1;
          child[v] = (u == c) ? v : child[u];
          bfs.push_back(v);
        }
      }
      for (const int u : bfs) {
        const int ch = (u == c) ? -1 : child_seq[child[u]];
        Rec &r = recs[size_t(u) * stride + rec_cnt[u]++];
        r.cid = c;
        r.dist = dist[u];
        r.child = ch;
        const int d1 = dist[u] + 1;
        if (d1 > seq_len[c]) seq_len[c] = d1;
        if (ch >= 0 && d1 > seq_len[ch]) seq_len[ch] = d1;
      }
      removed[c] = 1;
      for (int j = adj_off[c]; j < adj_off[c + 1]; ++j) {
        const int v = adj[j];
        if (!removed[v]) stack.push_back(v);
      }
    }
  }

  // 列 seq の位置 pos (0 始まり) に x を足す。pos が列の長さ以上なら何もしない。
  void bit_add(int seq, int pos, i64 x) {
    const int len = seq_len[seq];
    i64 *b = fw.data() + seq_off[seq];
    for (int i = pos + 1; i <= len; i += i & -i) b[i] += x;
  }
  // 列 seq の [0, pos] の和。
  i64 bit_sum(int seq, int pos) const {
    const i64 *b = fw.data() + seq_off[seq];
    i64 s = 0;
    for (int i = pos + 1; i > 0; i -= i & -i) s += b[i];
    return s;
  }
  // 列 seq の [l, r) に x を足す。範囲外の番地は上の 2 つが黙って捨てる。
  void range_add(int seq, int l, int r, i64 x) {
    bit_add(seq, l, x);
    bit_add(seq, r, -x);
  }

  void run() {
    for (const auto &e : queries) {
      const int p = (int)e[1];
      const Rec *rp = recs.data() + size_t(p) * stride;
      const int cnt = rec_cnt[p];
      if (e[0] == 0) {
        const i64 l = e[2], r = e[3], x = e[4];
        for (int k = 0; k < cnt; ++k) {
          const int cid = rp[k].cid, d = rp[k].dist, ch = rp[k].child;
          i64 lo = l - d, hi = r - d;
          if (lo < 0) lo = 0;
          if (hi > seq_len[cid]) hi = seq_len[cid];
          if (lo >= hi) continue;
          const int a0 = (int)lo, a1 = (int)hi;
          range_add(cid, a0, a1, x);
          if (ch >= 0) range_add(ch, a0, a1, -x);
        }
      } else {
        i64 v = a[p];
        for (int k = 0; k < cnt; ++k) {
          // 子成分の列には -x を入れてあるので、こちらは足すだけでよい。
          v += bit_sum(rp[k].cid, rp[k].dist);
          if (rp[k].child >= 0) v += bit_sum(rp[k].child, rp[k].dist);
        }
        ans.push_back(v);
      }
    }
  }

  const vector<i64> &answers() const { return ans; }
};
