#include <iomanip>
#include <iostream>
#include <string>
#include "neo/base/core/geo/Convex.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using R= long double;
 int H, W;
 cin >> H >> W;
 R mA[2], mB[2], mX;
 cin >> mA[0] >> mA[1] >> mB[0] >> mB[1] >> mX;
 string M[H];
 for(int i= 0; i < H; ++i) cin >> M[i];
 int cA= 0, cB= 0, cX= 0;
 R Ax= 0, Bx= 0, Xx= 0;
 R Ay= 0, By= 0, Xy= 0;
 for(int i= 0; i < H; ++i)
  for(int j= 0; j < W; ++j) {
   if(M[i][j] == 'A') ++cA, Ax+= j + 0.5, Ay+= i + 0.5;
   if(M[i][j] == 'B') ++cB, Bx+= j + 0.5, By+= i + 0.5;
   if(M[i][j] == 'X') ++cX, Xx+= j + 0.5, Xy+= i + 0.5;
  }
 Ax/= cA, Bx/= cB, Xx/= cX, Ay/= cA, By/= cB, Xy/= cX;
 mA[0]*= cA, mA[1]*= cA, mB[0]*= cB, mB[1]*= cB, mX*= cX;
 R ans= 0;
 Convex<R> ch_all({{mA[0], mB[0]}, {mA[1], mB[0]}, {mA[1], mB[1]}, {mA[0], mB[1]}});
 for(int i= 0; i < H; ++i)
  for(int j= 0; j < W; ++j)
   if(M[i][j] != '.') {
    auto ch= ch_all;
    ch= ch.cut(Ax - j, Bx - j, (Xx - j) * mX, 1);
    ch= ch.cut(Ax - j - 1, Bx - j - 1, (Xx - j - 1) * mX, -1);
    ch= ch.cut(Ay - i, By - i, (Xy - i) * mX, 1);
    ch= ch.cut(Ay - i - 1, By - i - 1, (Xy - i - 1) * mX, -1);
    ans+= ch.area();
   }
 ans/= ch_all.area();
 cout << fixed << setprecision(12) << ans << '\n';
 return 0;
}
