#include <cmath>
#include <iostream>
#include <string>
using namespace std;
// 小数第 4 位までを 10^4 倍の整数に直し、列ごとに整数平方根で y の範囲を数える。
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
static long long isqrt128(__int128 n) {
 long long r= (long long)sqrtl((long double)n);
 while((__int128)(r + 1) * (r + 1) <= n) ++r;
 while((__int128)r * r > n) --r;
 return r;
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
 for(long long x= ceildiv(X - R, S), xe= floordiv(X + R, S); x <= xe; ++x) {
  __int128 dx= (__int128)x * S - X;
  __int128 rem= (__int128)R * R - dx * dx;
  if(rem < 0) continue;
  long long h= isqrt128(rem);
  long long ylo= ceildiv(Y - h, S), yhi= floordiv(Y + h, S);
  if(yhi >= ylo) ans+= yhi - ylo + 1;
 }
 cout << ans << '\n';
 return 0;
}
