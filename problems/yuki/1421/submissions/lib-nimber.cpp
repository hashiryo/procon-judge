// https://yukicoder.me/problems/no/1421
// あえて Nimber の体の上で LU 分解する。係数は 0 か 1 なので、解も 2^30 未満に収まる
#include <iostream>
#include "mylib/algebra/Nimber.hpp"
#include "mylib/algebra/LU_Decomposition.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 Nimber::init();
 int N, M;
 cin >> N >> M;
 Matrix<Nimber> m(M, N);
 Vector<Nimber> Y(M);
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
 auto sol= LU_Decomposition(m).linear_equations(Y);
 if(sol) {
  for(int i= 0; i < N; ++i) cout << sol[i] << '\n';
 } else cout << -1 << '\n';
 return 0;
}
