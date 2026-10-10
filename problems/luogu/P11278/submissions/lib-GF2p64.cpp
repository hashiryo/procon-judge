// https://www.luogu.com.cn/problem/P11278
// 2^32 未満の nimber は 2^32 元の体で、nim 平方はフロベニウス写像なので、32 回くり返すと元に戻る。各要素を 32 回平方した値を
// GF2p64 の square で前もって作り (from_nimber で移して平方し、to_nimber で戻す)、セグメント木の節ごとに「あと s 回平方したとき」
// の xor と和を s = 0, ..., 31 について持つ。区間の平方は、その区間の節の s を 1 つずらすだけになる
#include <cstdio>
#include <vector>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u32= unsigned;
using u64= unsigned long long;
static char ibuf[1 << 25];
size_t ipos, ilen;
u64 read_u64() {
 auto get= [&]() { return ipos < ilen || (ipos= 0, (ilen= fread(ibuf, 1, sizeof(ibuf), stdin)) != 0) ? ibuf[ipos++] : '\0'; };
 char c= get();
 while(c < '0' || c > '9') c= get();
 u64 x= 0;
 for(; c >= '0' && c <= '9'; c= get()) x= x * 10 + (c - '0');
 return x;
}
int n, sz;
vector<u64> S;          // S[k * 32 + s]: 節 k の要素を (rot[k] + s) 回平方したものの和 (s は 32 で割った余り)
vector<u32> X;          // X[k * 32 + s]: 同じく xor
vector<unsigned char> rot, tag;  // rot[k] は節 k の今の状態の位置、tag[k] は子にまだ配っていない平方の回数
void shift(int k, int t) { rot[k]= (rot[k] + t) & 31, tag[k]= (tag[k] + t) & 31; }
void push(int k) {
 if(tag[k]) shift(2 * k, tag[k]), shift(2 * k + 1, tag[k]), tag[k]= 0;
}
void pull(int k) {
 const int l= 2 * k, r= 2 * k + 1, rl= rot[l], rr= rot[r];
 for(int s= 0; s < 32; ++s) {
  S[k * 32 + s]= S[l * 32 + ((rl + s) & 31)] + S[r * 32 + ((rr + s) & 31)];
  X[k * 32 + s]= X[l * 32 + ((rl + s) & 31)] ^ X[r * 32 + ((rr + s) & 31)];
 }
 rot[k]= 0;
}
void square(int k, int l, int r, int a, int b) {
 if(b <= l || r <= a) return;
 if(a <= l && r <= b) return shift(k, 1);
 push(k);
 const int m= (l + r) / 2;
 square(2 * k, l, m, a, b), square(2 * k + 1, m, r, a, b), pull(k);
}
pair<u64, u32> query(int k, int l, int r, int a, int b) {
 if(b <= l || r <= a) return {0, 0};
 if(a <= l && r <= b) return {S[k * 32 + rot[k]], X[k * 32 + rot[k]]};
 push(k);
 const int m= (l + r) / 2;
 const auto [s1, x1]= query(2 * k, l, m, a, b);
 const auto [s2, x2]= query(2 * k + 1, m, r, a, b);
 return {s1 + s2, x1 ^ x2};
}
signed main() {
 n= read_u64();
 const int q= read_u64();
 for(sz= 1; sz < n;) sz*= 2;
 S.assign(size_t(2) * sz * 32, 0), X.assign(size_t(2) * sz * 32, 0), rot.assign(2 * sz, 0), tag.assign(2 * sz, 0);
 for(int i= 0; i < n; ++i) {
  GF2p64 g= GF2p64::from_nimber(read_u64());
  for(int s= 0; s < 32; ++s, g= g.square()) {
   const u64 v= g.to_nimber();
   S[size_t(sz + i) * 32 + s]= v, X[size_t(sz + i) * 32 + s]= u32(v);
  }
 }
 for(int k= sz; --k;) pull(k);
 vector<char> out;
 char buf[24];
 for(int i= 0; i < q; ++i) {
  const int t= read_u64(), l= read_u64() - 1, r= read_u64();
  if(t == 1) {
   square(1, 0, sz, l, r);
   continue;
  }
  const auto [s, x]= query(1, 0, sz, l, r);
  const int len= snprintf(buf, sizeof(buf), "%llu\n", t == 2 ? u64(x) : s);
  out.insert(out.end(), buf, buf + len);
 }
 fwrite(out.data(), 1, out.size(), stdout);
 return 0;
}
