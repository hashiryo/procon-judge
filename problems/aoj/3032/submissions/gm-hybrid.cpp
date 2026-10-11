// 一般グラフの最大マッチング (procon-judge の _shared/flow/gm_dcpr.hpp の hybrid。Karp-Sipser と交互森を 2 回まで回し、
// 終わらなければ二部の二重被覆の push-relabel から仕上げる) で解く。
#include <cstdlib>
#include <iostream>
#include "_shared/flow/gm_dcpr.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, A, B;
 cin >> N >> A >> B;
 vector<int> a(N), b(N);
 for(int i= 0; i < N; i++) cin >> a[i] >> b[i];
 vector<array<int, 2>> e;
 auto ok= [&](int d) { return d <= A || (B <= d && d <= 2 * A); };
 for(int i= 0; i < N; i++) {
  if(ok(abs(a[i] - b[i]))) e.push_back({i, i + N});
  for(int j= i + 1; j < N; j++)
   if(ok(abs(a[i] + a[j] - b[i] - b[j]))) e.push_back({i, j});
 }
 const auto mt= gm_dcpr::hybrid(2 * N, e, 2);
 int cnt= 0;
 for(int v= 0; v < 2 * N; ++v) cnt+= mt[v] > v;
 cout << cnt << '\n';
}
