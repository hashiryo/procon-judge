// https://yukicoder.me/problems/no/259
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);                         // 釣り堀の長さ
//     void add(i64 t, int y, i64 z, bool to_right);   // 時刻 t に位置 y へ z 匹 (to_right なら右向き)
//     i64 count(i64 t, int y, int z);                 // 時刻 t の [y, z) にいる魚の数
//   };
//
// 左向きの魚と右向きの魚を別々の列に持つと、時間が dt 進むのは列を dt ずらして
// はみ出たぶんを反転して反対の列へ移す操作になる。その手順は common.hpp に置き、
// 反転と分割と結合ができる平衡二分木の実装 (乱択 / Splay / 重み平衡) を比べる。
//
// 構築を計測区間に入れてある。長さ N の列を 2 本作るところから実装で違う。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-splay.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  struct Query {
    char kind;
    i64 t, y, z;
  };
  vector<Query> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    e.kind = read_token()[0];
    must_scan(scanf("%lld %lld %lld", &e.t, &e.y, &e.z), 3);
    if (e.kind == 'C') ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n);
  for (auto &e : qs) {
    if (e.kind == 'C') ans.push_back(s.count(e.t, (int)e.y, (int)e.z));
    else s.add(e.t, (int)e.y, e.z, e.kind == 'R');
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
