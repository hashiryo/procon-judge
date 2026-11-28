#pragma once
// 反転の遅延を持つ implicit treap。位置で分割し、区間 [l, r) を切り出して反転・和を取る。
// 優先度は splitmix64 で決めるので、同じ入力ならいつも同じ形の木になる。
// 和は素の 64 bit (最悪 N * A_MAX = 2 * 10^14)。
#include <cstdint>
#include <utility>
#include <vector>

struct Solver {
  // 0 番は番兵。lc[0] = rc[0] = sz[0] = 0、sm[0] = 0 なので、子の有無を見ずに足せる。
  std::vector<int> lc, rc, sz;
  std::vector<std::uint32_t> pri;
  std::vector<long long> val, sm;
  std::vector<char> rev;
  int root = 0;

  explicit Solver(const std::vector<long long> &a) {
    const int n = (int)a.size();
    lc.assign(n + 1, 0);
    rc.assign(n + 1, 0);
    sz.assign(n + 1, 0);
    pri.assign(n + 1, 0);
    val.assign(n + 1, 0);
    sm.assign(n + 1, 0);
    rev.assign(n + 1, 0);

    // 右の背骨を積んだまま、優先度の大きい方を上にして Cartesian 木を組む。
    std::uint64_t s = 88172645463325252ull;
    std::vector<int> spine;
    spine.reserve(n);
    for (int i = 1; i <= n; ++i) {
      s += 0x9e3779b97f4a7c15ull;
      std::uint64_t z = s;
      z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
      z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
      z ^= z >> 31;
      pri[i] = (std::uint32_t)(z >> 32);
      val[i] = a[i - 1];
      sz[i] = 1;
      sm[i] = a[i - 1];
      int last = 0;
      while (!spine.empty() && pri[spine.back()] < pri[i]) {
        last = spine.back();
        spine.pop_back();
      }
      lc[i] = last;
      if (!spine.empty()) rc[spine.back()] = i;
      spine.push_back(i);
    }
    root = spine.empty() ? 0 : spine.front();
    build_meta(root);
  }

  void build_meta(int x) {
    if (!x) return;
    build_meta(lc[x]);
    build_meta(rc[x]);
    pull(x);
  }

  void pull(int x) {
    sz[x] = 1 + sz[lc[x]] + sz[rc[x]];
    sm[x] = val[x] + sm[lc[x]] + sm[rc[x]];
  }

  // 部分木ごと反転する。左右を入れ替え、遅延の印を付ける。
  void apply_rev(int x) {
    if (!x) return;
    std::swap(lc[x], rc[x]);
    rev[x] ^= 1;
  }

  void push(int x) {
    if (rev[x]) {
      apply_rev(lc[x]);
      apply_rev(rc[x]);
      rev[x] = 0;
    }
  }

  // 先頭 k 個と残りに分ける。
  std::pair<int, int> split(int x, int k) {
    if (!x) return {0, 0};
    push(x);
    if (sz[lc[x]] >= k) {
      auto [a, b] = split(lc[x], k);
      lc[x] = b;
      pull(x);
      return {a, x};
    }
    auto [a, b] = split(rc[x], k - sz[lc[x]] - 1);
    rc[x] = a;
    pull(x);
    return {x, b};
  }

  int merge(int a, int b) {
    if (!a) return b;
    if (!b) return a;
    if (pri[a] > pri[b]) {
      push(a);
      rc[a] = merge(rc[a], b);
      pull(a);
      return a;
    }
    push(b);
    lc[b] = merge(a, lc[b]);
    pull(b);
    return b;
  }

  void reverse(int l, int r) {
    if (l >= r) return;
    auto [a, bc] = split(root, l);
    auto [b, c] = split(bc, r - l);
    apply_rev(b);
    root = merge(a, merge(b, c));
  }

  long long sum(int l, int r) {
    if (l >= r) return 0;
    auto [a, bc] = split(root, l);
    auto [b, c] = split(bc, r - l);
    const long long res = sm[b];
    root = merge(a, merge(b, c));
    return res;
  }
};
