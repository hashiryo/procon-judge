#include <iostream>
#include <set>
#include <vector>
using namespace std;
// 小さい N 用の愚直解。各直線を、その上にある点の添字の昇順の列で表して重複を除く。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N, k;
 cin >> N >> k;
 vector<long long> x(N), y(N);
 for(int i= 0; i < N; ++i) cin >> x[i] >> y[i];
 if(k == 1) {
  cout << "Infinity" << '\n';
  return 0;
 }
 set<vector<int>> lines;
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < i; ++j) {
   vector<int> on;
   for(int l= 0; l < N; ++l) {
    __int128 c= (__int128)(x[j] - x[i]) * (y[l] - y[i]) - (__int128)(y[j] - y[i]) * (x[l] - x[i]);
    if(c == 0) on.push_back(l);
   }
   if((int)on.size() >= k) lines.insert(on);
  }
 cout << (long long)lines.size() << '\n';
 return 0;
}
