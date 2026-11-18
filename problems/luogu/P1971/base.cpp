// 洛谷 P1971 [NOI2011] 兔兔与蛋蛋游戏。n 行 m 列の盤 (n, m ≤ 40) に、黒 (X) と白 (O) の駒と、空きマス (.) が 1 つある。
// 兔兔は空きマスの隣の白い駒を、蛋蛋は隣の黒い駒を空きマスへ動かし、兔兔から交互に指す。動かせない人が負け。蛋蛋が勝った
// 対局の記録 (各 k 手、k ≤ 1000) が与えられるので、兔兔の手のうち、指す前は兔兔に必勝法があり、指したあとは蛋蛋に必勝法が
// ある手 (犯错误) の番号を答える。
//
// 提出は次を実装する。
//   struct Solver {
//     // board は n 行の文字列、moves は 2k 手の (行, 列) で 0 始まり (偶数番目が兔兔、奇数番目が蛋蛋)
//     Solver(int n, int m, const vector<string> &board, const vector<array<int, 2>> &moves);
//     vector<int> run();  // 犯错误の手の番号 (1 始まり、昇順)
//   };
// 入力を読んだあと、構築と run を測る。出力は個数と番号。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<string> board(n);
  for (auto &s : board) s = read_token();
  int k;
  must_scan(scanf("%d", &k), 1);
  vector<array<int, 2>> moves(2 * k);
  for (auto &[x, y] : moves) must_scan(scanf("%d %d", &x, &y), 2), --x, --y;
  auto t0 = chrono::steady_clock::now();
  Solver sol(n, m, board, moves);
  const vector<int> ans = sol.run();
  auto t1 = chrono::steady_clock::now();
  printf("%d\n", (int)ans.size());
  print_all(ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
