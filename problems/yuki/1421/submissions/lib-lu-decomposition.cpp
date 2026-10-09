// https://yukicoder.me/problems/no/1421
// bit ごとに F_2 の連立方程式を LU 分解で解く
#include <iostream>
#include <vector>
#include "mylib/algebra/LU_Decomposition.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, M;
 cin >> N >> M;
 Matrix<bool> m(M, N);
 vector<int> Y(M);
 for(int i= 0; i < M; ++i) {
  int A;
  cin >> A;
  for(int j= 0; j < A; ++j) {
   int B;
   cin >> B, --B;
   m[i][B]= 1;
  }
  cin >> Y[i];
 }
 LU_Decomposition lud(m);
 vector<int> ans(N);
 for(int k= 0; k < 30; ++k) {
  Vector<bool> b(M);
  for(int i= 0; i < M; ++i) b[i]= (Y[i] >> k) & 1;
  auto sol= lud.linear_equations(b);
  if(!sol) return cout << -1 << '\n', 0;
  for(int i= 0; i < N; ++i) ans[i]|= sol[i] << k;
 }
 for(int i= 0; i < N; ++i) cout << ans[i] << '\n';
 return 0;
}
