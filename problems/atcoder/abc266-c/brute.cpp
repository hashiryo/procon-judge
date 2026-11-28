#include <iostream>
using namespace std;
// 小さい入力用の愚直解。各頂点の折れの向きを __int128 の外積で見る。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 long long x[4], y[4];
 for(int i= 0; i < 4; ++i) cin >> x[i] >> y[i];
 bool ok= true;
 for(int i= 0; i < 4; ++i) {
  int j= (i + 1) % 4, k= (i + 2) % 4;
  __int128 c= (__int128)(x[j] - x[i]) * (y[k] - y[i]) - (__int128)(y[j] - y[i]) * (x[k] - x[i]);
  if(c <= 0) ok= false;
 }
 cout << (ok ? "Yes" : "No") << '\n';
 return 0;
}
