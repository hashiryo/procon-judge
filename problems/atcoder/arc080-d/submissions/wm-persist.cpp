// https://atcoder.jp/contests/arc080/tasks/arc080_d
// 重み最小の完全マッチングを、重みを C - w (C = 頂点数 + 2) にした重み最大マッチング (procon-judge の
// _shared/flow/wm_persist_avx2.hpp) で解く。C を頂点数 + 1 より大きくすると、完全マッチングのほうがどの完全でない
// マッチングより重くなる。
#include <iostream>
#include <vector>
#include "_shared/flow/wm_persist_avx2.hpp"
#include "mylib/number_theory/is_prime.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<int> x(N);
 for(int i= 0; i < N; ++i) cin >> x[i];
 vector<int> vs= {x[0]};
 for(int i= 1; i < N; ++i)
  if(x[i] - x[i - 1] > 1) vs.push_back(x[i - 1] + 1), vs.push_back(x[i]);
 vs.push_back(x[N - 1] + 1);
 const int m= vs.size();
 const long long C= m + 2;
 wm_persist_avx2::Solver s(m);
 vector<vector<int>> w(m, vector<int>(m));
 for(int i= 0; i < m; ++i)
  for(int j= i + 1; j < m; ++j) {
   const int k= vs[j] - vs[i];
   w[i][j]= w[j][i]= k & 1 ? is_prime(k) ? 1 : 3 : 2;
   s.add_edge(i, j, C - w[i][j]);
  }
 const auto mt= s.solve();
 long long ans= 0;
 for(int i= 0; i < m; ++i)
  if(mt[i] > i) ans+= w[i][mt[i]];
 cout << ans << '\n';
}
