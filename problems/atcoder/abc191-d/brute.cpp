#include <cmath>
#include <iostream>
#include <string>
using namespace std;
// 小さい半径用の愚直解。格子点を 1 つずつ不等式で見る。
static long long parse(const string& s) {
 bool neg= !s.empty() && s[0] == '-';
 int st= neg ? 1 : 0;
 auto dot= s.find('.', st);
 string ip= dot == string::npos ? s.substr(st) : s.substr(st, dot - st);
 string fp= dot == string::npos ? "" : s.substr(dot + 1);
 while((int)fp.size() < 4) fp+= '0';
 fp= fp.substr(0, 4);
 long long v= stoll(ip.empty() ? "0" : ip) * 10000 + stoll(fp);
 return neg ? -v : v;
}
static long long floordiv(long long a, long long b) {
 long long q= a / b;
 if((a % b) && ((a < 0) != (b < 0))) --q;
 return q;
}
static long long ceildiv(long long a, long long b) { return -floordiv(-a, b); }
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 string sx, sy, sr;
 cin >> sx >> sy >> sr;
 long long X= parse(sx), Y= parse(sy), R= parse(sr);
 const long long S= 10000;
 long long ans= 0;
 for(long long x= ceildiv(X - R, S), xe= floordiv(X + R, S); x <= xe; ++x)
  for(long long y= ceildiv(Y - R, S), ye= floordiv(Y + R, S); y <= ye; ++y) {
   __int128 dx= (__int128)x * S - X, dy= (__int128)y * S - Y;
   if(dx * dx + dy * dy <= (__int128)R * R) ++ans;
  }
 cout << ans << '\n';
 return 0;
}
