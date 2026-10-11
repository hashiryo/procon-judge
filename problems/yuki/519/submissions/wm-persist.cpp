// 一般グラフの重み最大マッチング (procon-judge の _shared/flow/wm_persist_avx2.hpp。増加路を流しても交互森を作り直さない
// O(n^3) の花の方法) で解く。
#include <iostream>
#include "_shared/flow/wm_persist_avx2.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n;
 cin >> n;
 wm_persist_avx2::Solver s(n);
 vector<vector<long long>> f(n, vector<long long>(n));
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) cin >> f[i][j];
 for(int i= 0; i < n; ++i)
  for(int j= i + 1; j < n; ++j) s.add_edge(i, j, max(f[i][j], f[j][i]));
 const auto mt= s.solve();
 long long ans= 0;
 for(int i= 0; i < n; ++i)
  if(mt[i] > i) ans+= max(f[i][mt[i]], f[mt[i]][i]);
 cout << ans << '\n';
}
