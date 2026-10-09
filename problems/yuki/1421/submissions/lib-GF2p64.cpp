// https://yukicoder.me/problems/no/1421
// あえて GF2p64 の体の上で掃き出す。係数は 0 か 1 なので、Y をそのまま GF2p64 の元とみてよく、解も 2^30 未満に収まる
#include <iostream>
#include <vector>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, M;
 cin >> N >> M;
 vector a(M, vector<GF2p64>(N + 1));  // 拡大係数行列
 for(auto& row: a) {
  int A;
  cin >> A;
  for(int j= 0; j < A; ++j) {
   int B;
   cin >> B;
   row[B - 1]= GF2p64(1);
  }
  u64 y;
  cin >> y;
  row[N]= GF2p64(y);
 }
 vector<int> piv;  // i 行目の軸の列
 for(int c= 0; c < N && (int)piv.size() < M; ++c) {
  const int r= piv.size();
  int p= r;
  while(p < M && !a[p][c]) ++p;
  if(p == M) continue;
  swap(a[p], a[r]);
  const GF2p64 iv= a[r][c].inv();
  for(int j= c; j <= N; ++j) a[r][j]*= iv;
  for(int i= 0; i < M; ++i)
   if(i != r && a[i][c]) {
    const GF2p64 m= a[i][c];
    for(int j= c; j <= N; ++j) a[i][j]-= m * a[r][j];
   }
  piv.push_back(c);
 }
 for(int i= piv.size(); i < M; ++i)
  if(a[i][N]) return cout << -1 << '\n', 0;
 vector<GF2p64> x(N);
 for(int i= 0; i < (int)piv.size(); ++i) x[piv[i]]= a[i][N];
 for(auto v: x) cout << u64(v) << '\n';
 return 0;
}
