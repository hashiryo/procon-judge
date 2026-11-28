#include <iostream>
#include <vector>
using namespace std;
// 小さい N 用の愚直解。外積を __int128 でそのまま計算する。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N;
 cin >> N;
 vector<long long> x(N), y(N);
 for(int i= 0; i < N; ++i) cin >> x[i] >> y[i];
 bool ok= false;
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < i; ++j)
   for(int k= 0; k < j; ++k) {
    __int128 c= (__int128)(x[j] - x[i]) * (y[k] - y[i]) - (__int128)(y[j] - y[i]) * (x[k] - x[i]);
    if(c == 0) ok= true;
   }
 cout << (ok ? "Yes" : "No") << '\n';
 return 0;
}
