#pragma once
// F_p 上の多項式の因数分解 (次数 ≤ 100、FFT なし)。係数は NeoLibrary の ZMod<0> (法は実行時)。
// 1. 平方因子なしの分解: f' と gcd を取る Yun の方法。f' = 0 の部分は f = h(x^p) なので h に直して重複度を p 倍する。
// 2. 次数ごとの分解: 法 s での Frobenius h -> h^p を、行 x^{ip} mod s を並べた行列で O(n^2) で当て、gcd(s, x^{p^d} - x)。
// 3. 同じ次数 d の積の分解: 乱択の a について、p が奇数なら gcd(g, a^{(p^d - 1)/2} - 1)、p = 2 なら
//    gcd(g, a + a^2 + ... + a^{2^{d-1}})。
#include <random>
#include <utility>
#include <vector>
#include "neo/algebra/ZMod.hpp"
namespace fz1 {
using Z= ZMod<0>;
using poly= std::vector<Z>;
inline void trim(poly& a) {
 while(!a.empty() && a.back() == Z()) a.pop_back();
}
inline int deg(const poly& a) { return int(a.size()) - 1; }
inline void monic(poly& a) {
 if(a.empty()) return;
 const Z il= a.back().inv();
 for(auto& x: a) x*= il;
}
inline poly mul(const poly& a, const poly& b) {
 if(a.empty() || b.empty()) return {};
 poly c(a.size() + b.size() - 1);
 for(size_t i= 0; i < a.size(); ++i) {
  if(a[i] == Z()) continue;
  const auto ai= a[i].fixed();
  for(size_t j= 0; j < b.size(); ++j) c[i + j]+= b[j] * ai;
 }
 return c;
}
// a を b で割った商と余り
inline std::pair<poly, poly> divmod(poly a, const poly& b) {
 trim(a);
 const int db= deg(b);
 if(deg(a) < db) return {poly{}, a};
 const Z il= b.back().inv();
 poly q(deg(a) - db + 1);
 for(int i= deg(a); i >= db; --i) {
  const Z c= a[i] * il;
  q[i - db]= c;
  if(c == Z()) continue;
  const auto cf= c.fixed();
  for(int j= 0; j < db; ++j) a[i - db + j]-= b[j] * cf;
  a[i]= Z();
 }
 a.resize(db), trim(a);
 return {q, a};
}
inline poly rem(const poly& a, const poly& b) { return divmod(a, b).second; }
inline poly quo(const poly& a, const poly& b) { return divmod(a, b).first; }
inline poly gcd(poly a, poly b) {
 trim(a), trim(b);
 while(!b.empty()) a= rem(a, b), std::swap(a, b);
 monic(a);
 return a;
}
inline poly deriv(const poly& a) {
 poly r(a.size() > 1 ? a.size() - 1 : 0);
 for(size_t i= 1; i < a.size(); ++i) r[i - 1]= a[i] * Z(i);
 trim(r);
 return r;
}
inline poly powmod(poly b, u64 e, const poly& m) {
 poly r{Z(1)};
 b= rem(b, m);
 for(; e; e>>= 1, b= rem(mul(b, b), m))
  if(e & 1) r= rem(mul(r, b), m);
 return r;
}
// 法 m (n 次) での Frobenius の行列。Q[i] = x^{ip} mod m (i < n)。
inline std::vector<poly> frob_matrix(const poly& m) {
 const int n= deg(m);
 std::vector<poly> Q(n);
 Q[0]= poly{Z(1)};
 if(n == 1) return Q;
 const poly xp= powmod(poly{Z(), Z(1)}, Z::mod(), m);
 for(int i= 1; i < n; ++i) Q[i]= rem(mul(Q[i - 1], xp), m);
 return Q;
}
// h^p mod m (h は m より次数が低い)
inline poly frob(const poly& h, const std::vector<poly>& Q) {
 poly r(Q.size());
 for(size_t i= 0; i < h.size(); ++i) {
  if(h[i] == Z()) continue;
  const auto hi= h[i].fixed();
  for(size_t j= 0; j < Q[i].size(); ++j) r[j]+= Q[i][j] * hi;
 }
 trim(r);
 return r;
}
using Out= std::vector<std::pair<int, poly>>;
inline void edf(const poly& g, int d, int mult, Out& out, std::mt19937& rng) {
 const int n= deg(g);
 if(n == d) return out.push_back({mult, g}), void();
 const auto Q= frob_matrix(g);
 const u32 p= Z::mod();
 while(true) {
  poly a(n);
  for(auto& x: a) x= Z(rng() % p);
  trim(a);
  if(deg(a) <= 0) continue;
  poly b;
  if(p == 2) {
   poly t= a;
   b= a;
   for(int j= 1; j < d; ++j) {
    t= frob(t, Q);
    if(b.size() < t.size()) b.resize(t.size());
    for(size_t k= 0; k < t.size(); ++k) b[k]+= t[k];
   }
   trim(b);
  } else {
   poly t= a, pr= a;
   for(int j= 1; j < d; ++j) t= frob(t, Q), pr= rem(mul(pr, t), g);
   b= powmod(pr, (p - 1) / 2, g);
   if(b.empty()) b.push_back(Z());
   b[0]-= Z(1), trim(b);
  }
  const poly h= gcd(g, b);
  if(0 < deg(h) && deg(h) < n) {
   edf(h, d, mult, out, rng);
   edf(quo(g, h), d, mult, out, rng);
   return;
  }
 }
}
// 平方因子のない monic な s を分解する。
inline void factor_sqfree(const poly& s, int mult, Out& out, std::mt19937& rng) {
 if(deg(s) == 1) return out.push_back({mult, s}), void();
 const auto Q= frob_matrix(s);
 poly cur= s, h{Z(), Z(1)};
 for(int d= 1; 2 * d <= deg(cur); ++d) {
  h= rem(frob(h, Q), cur);
  poly t= h;
  if(t.size() < 2) t.resize(2);
  t[1]-= Z(1), trim(t);
  const poly g= gcd(cur, t);
  if(deg(g) > 0) {
   edf(g, d, mult, out, rng);
   cur= quo(cur, g);
   h= rem(h, cur);
  }
 }
 if(deg(cur) > 0) out.push_back({mult, cur});
}
// monic な f の平方因子なしの分解 (重複度 mult 倍) を、そのまま factor_sqfree に回す。
inline void sff(poly f, int mult, Out& out, std::mt19937& rng) {
 if(deg(f) <= 0) return;
 const u32 p= Z::mod();
 const poly g= deriv(f);
 if(g.empty()) {  // f = h(x^p)
  poly h(deg(f) / p + 1);
  for(size_t i= 0; i < h.size(); ++i) h[i]= f[i * p];
  return sff(h, mult * int(p), out, rng);
 }
 poly c= gcd(f, g), w= quo(f, c);
 for(int i= 1; deg(w) > 0; ++i) {
  const poly y= gcd(w, c);
  poly fac= quo(w, y);
  if(deg(fac) > 0) monic(fac), factor_sqfree(fac, i * mult, out, rng);
  w= y, c= quo(c, y);
 }
 if(deg(c) > 0) {  // 残りは重複度が p の倍数の因子で、c = h(x^p)
  poly h(deg(c) / p + 1);
  for(size_t i= 0; i < h.size(); ++i) h[i]= c[i * p];
  monic(h);
  sff(h, mult * int(p), out, rng);
 }
}
}  // namespace fz1
inline vector<pair<int, vector<u32>>> run(u32 p, const vector<u32>& a) {
 using namespace fz1;
 Z::set_mod(p);
 poly f(a.size());
 for(size_t i= 0; i < a.size(); ++i) f[i]= Z(a[i]);
 trim(f);
 Out out;
 std::mt19937 rng(20261118);
 sff(f, 1, out, rng);
 vector<pair<int, vector<u32>>> res;
 for(auto& [e, g]: out) {
  vector<u32> b(g.size());
  for(size_t i= 0; i < g.size(); ++i) b[i]= g[i].val();
  res.push_back({e, b});
 }
 return res;
}
