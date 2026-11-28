#include <cstdio>
#include <numeric>
#include <string>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
using namespace std;
namespace io {
constexpr int BUF= 1 << 16;
char buf[BUF];
int len= 0, pos= 0;
inline int get_char() {
 if(pos == len) len= (int)fread(buf, 1, BUF, stdin), pos= 0;
 return pos < len ? buf[pos++] : -1;
}
inline int read_int() {
 int c= get_char();
 while(c < '0' || c > '9') c= get_char();
 int x= 0;
 while('0' <= c && c <= '9') x= x * 10 + (c - '0'), c= get_char();
 return x;
}
}  // namespace io
// 区間 [l, r] の頻度表を Mo で動かす。cnt[x] は値 x の出現回数、F[d] は「ちょうど d 回出る値」の個数。
// 長さ L の区間で F[d] > 0 になる d は 1 + 2 + ... + k <= L より k = O(sqrt(L)) 個しかないので、
// 各クエリは F[d] > 0 の d を並べた列を走査して gcd(d, k) = 1 の F[d] を足せばよい。
// 列は追加と削除が O(1) の順不同の配列 (pos[d] が列の中の位置) で持つ。
signed main() {
 int n= io::read_int(), m= io::read_int();
 vector<int> a(n);
 for(auto& x: a) x= io::read_int();
 Mo mo;
 vector<int> ks(m);
 for(int i= 0; i < m; ++i) {
  int l= io::read_int(), r= io::read_int(), k= io::read_int();
  mo.query(l - 1, r);
  ks[i]= k;
 }
 vector<int> cnt(n + 1, 0), F(n + 2, 0), pos(n + 2, -1), act;
 act.reserve(1024);
 auto ins= [&](int d) {
  pos[d]= (int)act.size();
  act.push_back(d);
 };
 auto rem= [&](int d) {
  int i= pos[d], last= act.back();
  act[i]= last, pos[last]= i;
  act.pop_back(), pos[d]= -1;
 };
 auto add= [&](int i) {
  int x= a[i], c= cnt[x];
  if(c > 0 && --F[c] == 0) rem(c);
  if(F[c + 1]++ == 0) ins(c + 1);
  ++cnt[x];
 };
 auto erase= [&](int i) {
  int x= a[i], c= cnt[x];
  if(--F[c] == 0) rem(c);
  --cnt[x];
  if(c > 1 && F[c - 1]++ == 0) ins(c - 1);
 };
 vector<int> ans(m);
 auto out= [&](int id) {
  int k= ks[id], s= 0;
  for(int d: act)
   if(gcd(d, k) == 1) s+= F[d];
  ans[id]= s;
 };
 mo.run(add, erase, out);
 string res;
 res.reserve((size_t)m * 7);
 for(int i= 0; i < m; ++i) {
  int x= ans[i];
  char tmp[12];
  int k= 0;
  do tmp[k++]= char('0' + x % 10), x/= 10;
  while(x);
  while(k) res.push_back(tmp[--k]);
  res.push_back('\n');
 }
 fwrite(res.data(), 1, res.size(), stdout);
}
