#pragma once
// intrinsics を使わない素朴な Bostan–Mori (bmo の積をいつもスカラーにしたもの)。Q = E(x^2) + x O(x^2)、P = Pe(x^2) + x Po(x^2)
// と分けて、半分の大きさの積 4 回で 1 段進める。積は短い側の 1 項ずつを行にして u64 にため、15 行ごとに剰余を取る
// (コンパイラがベクトル化するかは任せる)。作業の配列は呼び出しをまたいで使い回す。
#include <algorithm>
#include <vector>
namespace bm_naive {
constexpr u32 M= 998244353;
inline u32 mulm(u32 a, u32 b) { return u32(u64(a) * b % M); }
inline u32 pw(u32 a, u64 e) {
 u32 r= 1;
 for(; e; e>>= 1, a= mulm(a, a))
  if(e & 1) r= mulm(r, a);
 return r;
}
struct Work {
 std::vector<u64> acc;
};
// out[0 .. n+m-1) = a b (M 未満)。短い側の 1 項ずつを行にして u64 にため、15 行ごとに剰余を取る。
inline void mul(const u32* a, int n, const u32* b, int m, u32* out, Work& w) {
 if(!n || !m) return;
 if(n > m) std::swap(a, b), std::swap(n, m);
 const int len= n + m - 1;
 w.acc.assign(len, 0);
 u64* acc= w.acc.data();
 for(int i= 0; i < n; ++i) {
  const u64 x= a[i];
  for(int j= 0; j < m; ++j) acc[i + j]+= x * b[j];
  if(i % 15 == 14)
   for(int k= 0; k < len; ++k) acc[k]%= M;
 }
 for(int k= 0; k < len; ++k) out[k]= u32(acc[k] % M);
}
inline u32 sub(u32 x, u32 y) { return x >= y ? x - y : x + M - y; }
}  // namespace bm_naive
inline u32 run(const vector<u32>& a, const vector<u32>& c, u64 k) {
 using namespace bm_naive;
 const int d= int(a.size());
 if(k < u64(d)) return a[k];
 if(d == 1) return mulm(a[0], pw(c[0], k));
 static thread_local std::vector<u32> Q, P, E, O, Pe, Po, T1, T2;
 static thread_local Work w;
 Q.assign(d + 1, 0), Q[0]= 1;
 for(int j= 1; j <= d; ++j) Q[j]= c[j - 1] ? M - c[j - 1] : 0;
 T1.resize(2 * d + 1);
 mul(a.data(), d, Q.data(), d + 1, T1.data(), w);
 P.assign(T1.begin(), T1.begin() + d);
 const int ne= d / 2 + 1, no= (d + 1) / 2, npe= (d + 1) / 2, npo= d / 2;
 E.resize(ne), O.resize(no), Pe.resize(npe), Po.resize(npo), T2.resize(2 * d + 1);
 for(; k; k>>= 1) {
  for(int i= 0; i < ne; ++i) E[i]= Q[2 * i];
  for(int i= 0; i < no; ++i) O[i]= Q[2 * i + 1];
  for(int i= 0; i < npe; ++i) Pe[i]= P[2 * i];
  for(int i= 0; i < npo; ++i) Po[i]= P[2 * i + 1];
  mul(E.data(), ne, E.data(), ne, T1.data(), w);
  mul(O.data(), no, O.data(), no, T2.data(), w);
  Q[0]= T1[0];
  for(int i= 1; i <= d; ++i) Q[i]= sub(i < 2 * ne - 1 ? T1[i] : 0, i - 1 < 2 * no - 1 ? T2[i - 1] : 0);
  if(k & 1) {
   mul(Po.data(), npo, E.data(), ne, T1.data(), w);
   mul(Pe.data(), npe, O.data(), no, T2.data(), w);
   for(int i= 0; i < d; ++i) P[i]= sub(i < npo + ne - 1 ? T1[i] : 0, i < npe + no - 1 ? T2[i] : 0);
  } else {
   mul(Pe.data(), npe, E.data(), ne, T1.data(), w);
   mul(Po.data(), npo, O.data(), no, T2.data(), w);
   P[0]= T1[0];
   for(int i= 1; i < d; ++i) P[i]= sub(i < npe + ne - 1 ? T1[i] : 0, i - 1 < npo + no - 1 ? T2[i - 1] : 0);
  }
 }
 return P[0];
}
