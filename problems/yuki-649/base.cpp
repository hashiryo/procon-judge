// https://yukicoder.me/problems/no/649
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(i64 k);  // K は 1-indexed。構築。計測区間の外。
//     void insert(i64 x);      // 多重集合に x を 1 つ足す
//     i64 pop_kth();           // K 番目に小さい値を 1 つ取り出して返す。
//                              // 要素が K 個に満たなければ -1 を返す。
//   };
//
// 追加する値は構築のときに渡さない。座標圧縮のような前処理をさせないためで、
// 前処理の計算量がクエリ列と同じ桁になるので、計測区間の外に出すと比較が
// 歪む。この問題のインターフェースはオンラインの実装だけを相手にする。
//
// 計測区間にはクエリの処理だけを残す。入力の解析、Solver の構築、答えの整形は
// すべて外に出してある。
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/two_heaps.hpp"
#endif
#include SUBMISSION_HPP

static void must_scan(int got, int want) {
  if (got != want) {
    fprintf(stderr, "input format error\n");
    exit(1);
  }
}

signed main() {
  int q;
  i64 k;
  must_scan(scanf("%d %lld", &q, &k), 2);

  // タイプ 2 は -1 で表す。値は 0 以上なので混ざらない。
  vector<i64> qs(q);
  for (auto &v : qs) {
    int type;
    must_scan(scanf("%d", &type), 1);
    if (type == 1) must_scan(scanf("%lld", &v), 1);
    else v = -1;
  }

  Solver s(k);
  vector<i64> ans;
  ans.reserve(q);

  auto t0 = chrono::steady_clock::now();
  for (i64 v : qs) {
    if (v < 0) ans.push_back(s.pop_kth());
    else s.insert(v);
  }
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(ans.size() * 20);
  for (i64 v : ans) {
    out += to_string(v);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
