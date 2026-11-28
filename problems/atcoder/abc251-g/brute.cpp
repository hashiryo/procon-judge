#include <iostream>
#include <vector>
using namespace std;
// 小さい M,Q 用の愚直解。各クエリ x について、各平行移動 t_i で x - t_i が P の内側かを
// __int128 の外積で見る。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N;
 cin >> N;
 vector<long long> px(N), py(N);
 for(int i= 0; i < N; ++i) cin >> px[i] >> py[i];
 int M;
 cin >> M;
 vector<long long> ux(M), uy(M);
 for(int i= 0; i < M; ++i) cin >> ux[i] >> uy[i];
 int Q;
 cin >> Q;
 while(Q--) {
  long long a, b;
  cin >> a >> b;
  bool all= true;
  for(int i= 0; i < M && all; ++i) {
   long long x= a - ux[i], y= b - uy[i];
   for(int e= 0; e < N; ++e) {
    long long dx= px[(e + 1) % N] - px[e], dy= py[(e + 1) % N] - py[e];
    __int128 c= (__int128)dx * (y - py[e]) - (__int128)dy * (x - px[e]);
    if(c < 0) {
     all= false;
     break;
    }
   }
  }
  cout << (all ? "Yes" : "No") << '\n';
 }
 return 0;
}
