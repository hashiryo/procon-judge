#pragma once
// ライブラリを使う提出が共有するもの。中点をまたぐ区間を分割統治で見ていく
// 骨組みはどの実装でも同じなので、ここに置く。比べたいのは、中点をまたぐ区間の
// 最小を「行ごとの最小」として取るところを何でやるか。
#include <algorithm>
#include "pj.hpp"

constexpr i64 INF = 1e18;

// 木 i .. j-1 を燃やしたときの悲しさ (j - i)^2 + A_i + ... + A_{j-1}。
struct Cost {
  vector<i64> sum;

  explicit Cost(const vector<i64> &a) : sum(a.size() + 1, 0) {
    for (size_t i = 0; i < a.size(); ++i) sum[i + 1] = sum[i] + a[i];
  }

  i64 operator()(int i, int j) const {
    return (i64)(j - i) * (j - i) + sum[j] - sum[i];
  }
};

// Engine は次を実装する。
//   Engine(const Cost &w, int n);
//   // 各 i in [L, M) について、r を [M, R) で動かした w(i, r + 1) の最小。
//   vector<i64> min_right(int L, int M, int R);
//   // 各 i in [M, R) について、l を [L, M] で動かした w(l, i + 1) の最小。
//   vector<i64> min_left(int L, int M, int R);
// どちらも i の順に並べて返す。
template <class Engine> struct BurnSolver {
  vector<i64> a, ans;

  explicit BurnSolver(const vector<i64> &a) : a(a) {}

  void run() {
    const int n = (int)a.size();
    Cost w(a);
    Engine eng(w, n);
    ans.assign(n, INF);
    // [L, R) の中点 M をまたぐ区間をここで見て、M を含まない区間は左右に任せる。
    auto rec = [&](auto &rec, int L, int R) -> void {
      if (L == R) return;
      const int M = (L + R) / 2;
      {
        // 左端 i in [L, M)、右端 r in [M, R)。左端を i より左へ動かしても i を
        // 含むので、i の昇順に最小を引き継ぐ。
        auto v = eng.min_right(L, M, R);
        i64 mn = INF;
        for (int i = L; i < M; ++i) {
          mn = std::min(mn, v[i - L]);
          ans[i] = std::min(ans[i], mn);
        }
      }
      {
        // 左端 l in [L, M]、右端 i in [M, R)。右端を i より右へ動かしても i を
        // 含むので、i の降順に最小を引き継ぐ。
        auto v = eng.min_left(L, M, R);
        i64 mn = INF;
        for (int i = R; i-- > M;) {
          mn = std::min(mn, v[i - M]);
          ans[i] = std::min(ans[i], mn);
        }
      }
      rec(rec, L, M), rec(rec, M + 1, R);
    };
    rec(rec, 0, n);
  }

  const vector<i64> &answer() const { return ans; }
};
