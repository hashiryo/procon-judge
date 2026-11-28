#pragma once
#include <algorithm>
#include <set>
#include <utility>

// 列を「p 昇順か降順に並んだ塊」に分けて持つ。塊の中は p を座標圧縮したキーの
// 動的セグメント木で、各節にキー昇順と降順の affine の合成を持つ。塊の列は位置の
// セグメント木に塊の左端だけ値を載せて合成する。
//
// タイプ 2/3 は両端で塊を割ってから、間の塊をキーのセグメント木の併合で 1 つにし、
// 向きの印を付ける。タイプ 1 は両端で割って、間の合成を位置のセグメント木から読む。
// タイプ 0 は 1 要素の塊に置き換える。
//
// 塊の木の根はいつもキーの全体 [0, K) を張るようにする。割るときは右側の根を
// 新しい節にして同じ範囲に置く。こうしておくと、併合は両方の根が同じ範囲から
// 始まるので、子をたどるだけでよい。
//
// 併合は重なった節を落としながら片方へ寄せる。節が増えるのはタイプ 0 の置き換えと
// 塊を割るときだけで、どちらもキーの深さぶんしか作らない。落とす節の総数は作った
// 節の総数以下なので、併合の総費用は (N + Q) log K で抑えられる。
struct Solver {
  static constexpr u32 MOD = 998244353;

  struct Node {
    int l, r;    // 子 (0 は無し)
    int sz;      // 部分木の要素数
    u32 aa, ab;  // キー昇順に並べたときの合成
    u32 da, db;  // キー降順に並べたときの合成
  };

  int n, k;  // k は座標圧縮したキーの数
  vector<Node> pool;
  vector<int> root;  // 塊の左端にだけ使う
  vector<char> rev;  // 塊の並びがキー降順なら 1
  vector<int> qkey;  // タイプ 0 のクエリの新しい p のキー
  vector<u32> sa, sb;  // 位置のセグメント木 (塊の左端に塊の合成)
  int segn;
  set<int> starts;  // 塊の左端
  const vector<array<i64, 5>> &qs;
  vector<i64> ans;

  Solver(int n_, const vector<array<i64, 3>> &init,
         const vector<array<i64, 5>> &queries)
      : n(n_), qs(queries) {
    // 出てくる p を全部集めて座標圧縮する。クエリも先に読めているので、あとから
    // 木を伸ばさずに済む。
    vector<i64> keys;
    keys.reserve(n + qs.size());
    for (auto &e : init) keys.push_back(e[0]);
    for (auto &e : qs)
      if (e[0] == 0) keys.push_back(e[2]);
    sort(keys.begin(), keys.end());
    keys.erase(unique(keys.begin(), keys.end()), keys.end());
    k = (int)keys.size();

    int sets = 0;
    qkey.assign(qs.size(), 0);
    for (size_t j = 0; j < qs.size(); ++j) {
      if (qs[j][0] == 0) {
        qkey[j] = int(lower_bound(keys.begin(), keys.end(), qs[j][2]) - keys.begin());
        ++sets;
      }
    }

    // 節が増えるのは、最初の要素 (N 個)、タイプ 0 の置き換え、塊を割るときの右側。
    // どれもキーの深さぶん。割るのは 1 クエリで 2 回まで。
    pool.reserve((size_t)(n + sets) * 20 + qs.size() * 40 + 16);
    pool.push_back(Node{0, 0, 0, 1, 0, 1, 0});  // 0 番は番兵
    root.assign(n, 0);
    rev.assign(n, 0);
    for (int i = 0; i < n; ++i) {
      int key = int(lower_bound(keys.begin(), keys.end(), init[i][0]) - keys.begin());
      root[i] = ins(0, k, key, (u32)init[i][1], (u32)init[i][2]);
    }

    segn = 1;
    while (segn < n) segn <<= 1;
    sa.assign(2 * segn, 1);
    sb.assign(2 * segn, 0);
    for (int i = 0; i < n; ++i) {
      starts.insert(i);
      set_block(i);
    }
    ans.reserve(qs.size());
  }

  void run() {
    for (size_t j = 0; j < qs.size(); ++j) {
      const array<i64, 5> &e = qs[j];
      int t = (int)e[0];
      if (t == 0) {
        // 1 要素の塊 [i, i+1) に置き換える。古い節は捨てる。
        int i = (int)e[1];
        split_at(i);
        split_at(i + 1);
        rev[i] = 0;
        root[i] = ins(0, k, qkey[j], (u32)e[3], (u32)e[4]);
        set_block(i);
      } else if (t == 1) {
        int l = (int)e[1], r = (int)e[2];
        split_at(l);
        split_at(r);
        auto [a, b] = range_prod(l, r);
        ans.push_back(((u64)a * (u64)e[3] + b) % MOD);
      } else {
        int l = (int)e[1], r = (int)e[2];
        split_at(l);
        split_at(r);
        // l より右で r より左の塊を、l の塊へキーで併合していく。
        auto it = starts.upper_bound(l);
        while (it != starts.end() && *it < r) {
          int i = *it;
          it = starts.erase(it);
          root[l] = merge_trees(root[l], root[i]);
          set_point(i, 1, 0);
        }
        rev[l] = (t == 3);
        set_block(l);
      }
    }
  }

  const vector<i64> &answer() const { return ans; }

 private:
  int alloc() {
    pool.push_back(Node{0, 0, 0, 1, 0, 1, 0});
    return (int)pool.size() - 1;
  }

  // 空の木に (key, a, b) を 1 つ入れた木を作る。
  int ins(int lo, int hi, int key, u32 a, u32 b) {
    int idx = alloc();
    if (hi - lo == 1) {
      pool[idx].sz = 1;
      pool[idx].aa = a;
      pool[idx].ab = b;
      pool[idx].da = a;
      pool[idx].db = b;
      return idx;
    }
    int mid = (lo + hi) >> 1;
    if (key < mid) {
      int c = ins(lo, mid, key, a, b);
      pool[idx].l = c;
    } else {
      int c = ins(mid, hi, key, a, b);
      pool[idx].r = c;
    }
    pull(idx);
    return idx;
  }

  void pull(int i) {
    Node &x = pool[i];
    int l = x.l, r = x.r;
    if (!l && !r) return;  // 葉 (キー 1 個の範囲) は値をそのまま持つ
    x.sz = (l ? pool[l].sz : 0) + (r ? pool[r].sz : 0);
    if (l && r) {
      // 昇順は左のあとに右、降順は右のあとに左。合成は後から当てる側を先に書く。
      x.aa = (u64)pool[r].aa * pool[l].aa % MOD;
      x.ab = ((u64)pool[r].aa * pool[l].ab + pool[r].ab) % MOD;
      x.da = (u64)pool[l].da * pool[r].da % MOD;
      x.db = ((u64)pool[l].da * pool[r].db + pool[l].db) % MOD;
    } else {
      const Node &c = pool[l ? l : r];
      x.aa = c.aa;
      x.ab = c.ab;
      x.da = c.da;
      x.db = c.db;
    }
  }

  // キー昇順で先頭 cnt 個と残りに割る。左は元の節をそのまま使い、右は同じ範囲の
  // 新しい節にする。どちらの根もキーの全体 [0, K) を張ったままになる。
  pair<int, int> split_tree(int n, int cnt) {
    if (cnt <= 0) return {0, n};
    if (cnt >= pool[n].sz) return {n, 0};
    int l = pool[n].l;
    int szl = l ? pool[l].sz : 0;
    int b = alloc();
    if (cnt <= szl) {
      auto [nl, nr] = split_tree(l, cnt);
      pool[b].l = nr;
      pool[b].r = pool[n].r;
      pool[n].l = nl;
      pool[n].r = 0;
    } else {
      auto [nl, nr] = split_tree(pool[n].r, cnt - szl);
      pool[n].r = nl;
      pool[b].l = 0;
      pool[b].r = nr;
    }
    pull(n);
    pull(b);
    return {n, b};
  }

  // キーが重ならない 2 つの木を併合する。重なった節は b の側を捨てる。
  int merge_trees(int a, int b) {
    if (!a) return b;
    if (!b) return a;
    if (!pool[a].l && !pool[a].r) return a;  // 同じキーは現れない
    int x = merge_trees(pool[a].l, pool[b].l);
    int y = merge_trees(pool[a].r, pool[b].r);
    pool[a].l = x;
    pool[a].r = y;
    pull(a);
    return a;
  }

  void set_point(int i, u32 a, u32 b) {
    int p = i + segn;
    sa[p] = a;
    sb[p] = b;
    for (p >>= 1; p; p >>= 1) {
      int l = p << 1, r = l | 1;
      sa[p] = (u64)sa[r] * sa[l] % MOD;
      sb[p] = ((u64)sa[r] * sb[l] + sb[r]) % MOD;
    }
  }

  pair<u32, u32> range_prod(int l, int r) {
    u32 la = 1, lb = 0, ra = 1, rb = 0;
    for (l += segn, r += segn; l < r; l >>= 1, r >>= 1) {
      if (l & 1) {
        u32 na = (u64)sa[l] * la % MOD;
        u32 nb = ((u64)sa[l] * lb + sb[l]) % MOD;
        la = na;
        lb = nb;
        ++l;
      }
      if (r & 1) {
        --r;
        u32 na = (u64)ra * sa[r] % MOD;
        u32 nb = ((u64)ra * sb[r] + rb) % MOD;
        ra = na;
        rb = nb;
      }
    }
    return {(u32)((u64)ra * la % MOD), (u32)(((u64)ra * lb + rb) % MOD)};
  }

  void set_block(int i) {
    const Node &t = pool[root[i]];
    if (rev[i]) set_point(i, t.da, t.db);
    else set_point(i, t.aa, t.ab);
  }

  // x が塊の左端になるように、x を含む塊を割る。x == n は何もしない。
  void split_at(int x) {
    if (x >= n) return;
    auto it = starts.lower_bound(x);
    if (it != starts.end() && *it == x) return;
    int b = (it == starts.end()) ? n : *it;
    int a = *prev(it);
    starts.insert(x);
    if (!rev[a]) {
      auto [t1, t2] = split_tree(root[a], x - a);
      root[a] = t1;
      root[x] = t2;
      rev[a] = 0;
      rev[x] = 0;
    } else {
      // 降順の塊は、先頭 x - a 個がキーの大きい側になる。
      auto [t1, t2] = split_tree(root[a], b - x);
      root[a] = t2;
      root[x] = t1;
      rev[a] = 1;
      rev[x] = 1;
    }
    set_block(a);
    set_block(x);
  }
};
