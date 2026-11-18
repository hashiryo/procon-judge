// 2×2 行列の列への挿入、削除、反転と区間の積 (自作)。入力は gen.py の説明のとおり。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<Mat> &a);  // 構築。計測区間の外。
//     void insert(int i, const Mat &m);       // 位置 i に m を挿む
//     void erase(int i);                      // 位置 i を取り除く
//     void reverse(int l, int r);             // [l, r) を反転
//     Mat prod(int l, int r);                 // a[l] a[l+1] ... a[r-1] (l < r)
//   };
//
// 列の木の書き比べで、op が重く可換でない場合を見るための問題 (algo-notes の notes/sequence-tree.md)。1 回の積は掛け算 8 回と
// 剰余 4 回で、dynamic-sequence-range-affine-range-sum の和より重い。構築は配列から組むだけなので計測区間の外に置く。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<Mat> a(n);
  for (auto &m : a) must_scan(scanf("%u %u %u %u", &m.a, &m.b, &m.c, &m.d), 4);

  // 種別ごとに項目数が違うので、種別を読んでから残りを読む。
  vector<array<u32, 6>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%u", &e[0]), 1);
    if (e[0] == 0) must_scan(scanf("%u %u %u %u %u", &e[1], &e[2], &e[3], &e[4], &e[5]), 5);
    else if (e[0] == 1) must_scan(scanf("%u", &e[1]), 1);
    else must_scan(scanf("%u %u", &e[1], &e[2]), 2);
    gets += e[0] == 3;
  }

  Solver s(a);
  vector<Mat> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.insert((int)e[1], Mat{e[2], e[3], e[4], e[5]});
    else if (e[0] == 1) s.erase((int)e[1]);
    else if (e[0] == 2) s.reverse((int)e[1], (int)e[2]);
    else ans.push_back(s.prod((int)e[1], (int)e[2]));
  }
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(ans.size() * 44);
  char buf[64];
  for (auto &m : ans) {
    const int k = snprintf(buf, sizeof(buf), "%u %u %u %u\n", m.a, m.b, m.c, m.d);
    out.append(buf, k);
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
