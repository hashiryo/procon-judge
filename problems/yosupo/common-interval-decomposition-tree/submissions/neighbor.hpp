#pragma once
#include "pj.hpp"

// 順列の common interval decomposition tree (Uno–Yagiura / Xuan–Habib–Paul の
// スタック法)。右端 r を伸ばし、左端の候補を積む。候補 (l, [vl, vr)) について
//   - 値域 [vl, vr) が l より前の位置を含むなら、左に閉じていないので 1 つ前へ吸収する
//   - vr - vl == r - l なら [l, r) は connected interval として確定する
//   - どちらでもなければ保留する
// 確定した connected interval を左端の successor リストで組み立て、2 タイルなら
// linear (同じ向きなら伸ばす)、3 タイル以上なら prime にする。
// 「値域が l より前を含むか」は値域の双方向リストで O(1) で見る。i = n-1..0 で P[i] を
// リストから抜くと、抜く直前の両隣が位置 i より前での値の前後になる (soryuusi0219 の 418976)。
struct Solver {
  struct Node {
    int parent;
    int l, r;
    bool linear;
  };

  int n;
  vector<int> p;
  vector<Node> nodes;

  Solver(int n_, const vector<int> &p_) : n(n_), p(p_) {}

  void run() {
    // 値域の双方向リスト。後ろから抜くと、各値について位置 i より前での前後の値が残る。
    vector<int> lower(n), upper(n);
    for (int i = 0; i < n; ++i) lower[i] = i - 1, upper[i] = i + 1;
    for (int i = n - 1; i >= 0; --i) {
      int v = p[i], lo = lower[v], hi = upper[v];
      if (lo >= 0) upper[lo] = hi;
      if (hi < n) lower[hi] = lo;
    }

    struct LeftBase {
      int l, vl, vr;
    };
    struct Common {
      int l, r;
    };
    vector<LeftBase> st;
    vector<Common> coms;
    st.reserve(n);
    coms.reserve(2 * n);

    for (int r = 1; r <= n; ++r) {
      int a = p[r - 1];
      LeftBase bs{r - 1, a, a + 1};
      while (!st.empty()) {
        if (bs.vl < st.back().vl) st.back().vl = bs.vl;
        if (bs.vr > st.back().vr) st.back().vr = bs.vr;
        LeftBase nx = st.back();
        if (!(lower[p[nx.l]] < nx.vl && nx.vr <= upper[p[nx.l]])) {
          // 左に閉じていない。1 つ前の候補へ吸収する (一番下は l = 0 なので来ない)。
          st.pop_back();
          LeftBase &nx2 = st.back();
          if (nx.vl < nx2.vl) nx2.vl = nx.vl;
          if (nx.vr > nx2.vr) nx2.vr = nx.vr;
        } else if (nx.vr - nx.vl == r - nx.l) {
          bs = nx;
          st.pop_back();
          coms.push_back({nx.l, r});
        } else {
          break;
        }
      }
      st.push_back(bs);
    }
    while (st.size() >= 2) {
      LeftBase nx = st.back();
      st.pop_back();
      LeftBase &nx2 = st.back();
      if (nx.vl < nx2.vl) nx2.vl = nx.vl;
      if (nx.vr > nx2.vr) nx2.vr = nx.vr;
      if (nx2.vr - nx2.vl == n - nx2.l) coms.push_back({nx2.l, n});
    }
    if (st.size() != 1) coms.push_back({0, n});

    // 組み立て。葉を 0..n-1 に置き、確定区間を左から順にまとめる。
    nodes.clear();
    nodes.reserve(2 * n);
    vector<signed char> type;  // 0: prime、1: dec、2: inc、3: 葉
    type.reserve(2 * n);
    for (int i = 0; i < n; ++i) {
      nodes.push_back({-1, i, i + 1, true});
      type.push_back(3);
    }
    vector<int> nodeid(n), sll(n);
    for (int i = 0; i < n; ++i) nodeid[i] = i, sll[i] = i + 1;
    for (const Common &com : coms) {
      int m = sll[com.l];
      if (sll[m] == com.r) {
        // 2 タイル。左と同じ向きの linear なら右端を伸ばす。
        int a = nodeid[com.l], b = nodeid[m];
        sll[com.l] = com.r;
        signed char t = p[com.l] < p[com.r - 1] ? 2 : 1;
        if (type[a] == t) {
          nodes[b].parent = a;
          nodes[a].r = com.r;
        } else {
          int c = (int)nodes.size();
          nodes.push_back({-1, com.l, com.r, true});
          type.push_back(t);
          nodes[a].parent = c;
          nodes[b].parent = c;
          nodeid[com.l] = c;
        }
      } else {
        // 3 タイル以上。全部を子にする prime。
        int c = (int)nodes.size();
        nodes.push_back({-1, com.l, com.r, false});
        type.push_back(0);
        for (int x = com.l; x < com.r; x = sll[x]) nodes[nodeid[x]].parent = c;
        nodeid[com.l] = c;
        sll[com.l] = com.r;
      }
    }
  }

  const vector<Node> &answer() const { return nodes; }
};
