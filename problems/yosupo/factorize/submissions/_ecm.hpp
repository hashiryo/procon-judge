#pragma once
// 64 bit の素因数分解の核。小さい素数で試し割りしてから、2^T 以上の合成数は ECM で、それより小さければ
// Pollard rho (Brent、2 本の列) で因数を 1 つずつ見つける。素数判定、gcd、逆元は NeoLibrary のものを使う。
//
// ECM は Montgomery 曲線 B y^2 = x^3 + A x^2 + x の x 座標だけで計算する (X:Z の射影座標)。曲線は Suyama の形で、
// 群の位数が 12 で割り切れる。σ = 6, 7, 8, ... に u = σ^2 - 5、v = 4σ、点 (u^3 : v^3)、
// (A + 2) / 4 = (v - u)^3 (3u + v) / (16 u^3 v) で、逆元は曲線ごとに 1 回 inv_gcd で求める。
// stage 1 は B1 以下の素数冪の積 (2 と 3 は E2、E3 だけ余分に掛ける) を 64 bit ずつに束ねて Montgomery ladder で掛け、
// stage 2 は D = 210 の baby-step giant-step で、m = 1..MH と D と互いに素な b < D/2 のすべての組の
// X_m Z_b - X_b Z_m を 4 本の積に掛けていく (B2 ≈ MH D)。
//
// 剰余の掛け算は R = 2^64 の Montgomery で、n < 2^60 なら値を 2n 未満のまま持ち、掛け算に渡すだけの和と差は
// 4n 未満のまま補正しない (4n 未満どうしの積の還元は 2n 未満に収まる)。n >= 2^60 は 128 bit の剰余で回す遅い rho に回す
// (Library Checker の入力は 10^18 以下なので使わない)。
#include "../common.hpp"
#include "neo/number_theory/gcd.hpp"
#include "neo/number_theory/inv_gcd.hpp"
#include "neo/number_theory/is_prime.hpp"

namespace ecm_fact {
using u64 = unsigned long long;
using u128 = unsigned __int128;

constexpr u64 inv64(u64 n) {
 u64 x= (3 * n) ^ 2, y= 1 - n * x;
 x*= 1 + y, y*= y;
 x*= 1 + y, y*= y;
 x*= 1 + y, y*= y;
 return x * (1 + y);
}

struct M60 {
 u64 n, ninv, n2, r2;
 explicit M60(u64 n_): n(n_), ninv(inv64(n_)), n2(2 * n_) {
  u64 r= (0 - n) % n;
  r2= u64((u128)r * r % n);
 }
 // a, b < 4n -> (0, 2n)
 u64 mul(u64 a, u64 b) const {
  u128 t= (u128)a * b;
  return u64(t >> 64) + n - u64((u128)(u64(t) * ninv) * n >> 64);
 }
 u64 to(u64 x) const { return mul(x, r2); }  // x < 4n
 u64 norm(u64 x) const {  // [0, 4n) -> [0, n)
  x= x >= n2 ? x - n2 : x;
  return x >= n ? x - n : x;
 }
 u64 from(u64 x) const { return norm(mul(x, 1)); }
};

struct Pt {
 u64 X, Z;
};
inline Pt xdbl(const M60& m, Pt P, u64 a24) {
 u64 s= P.X + P.Z, d= P.X - P.Z + m.n2;
 u64 ss= m.mul(s, s), dd= m.mul(d, d);
 u64 t= ss - dd + m.n2;
 return {m.mul(ss, dd), m.mul(t, dd + m.mul(a24, t))};
}
// P + Q。D = P - Q。
inline Pt xadd(const M60& m, Pt P, Pt Q, Pt D) {
 u64 u= m.mul(P.X - P.Z + m.n2, Q.X + Q.Z), v= m.mul(P.X + P.Z, Q.X - Q.Z + m.n2);
 u64 w= u + v, y= u - v + m.n2;
 return {m.mul(D.Z, m.mul(w, w)), m.mul(D.X, m.mul(y, y))};
}
// [k]P (k >= 2)。ビットで 2 点を入れ替えるのは cmov で書き、分岐にしない。
inline Pt ladder(const M60& m, Pt P, u64 k, u64 a24) {
 int L= 63 - __builtin_clzll(k);
 Pt R0= P, R1= xdbl(m, P, a24);
 u64 sw= 0;
 for(int i= L - 1; i >= 0; --i) {
  u64 bit= (k >> i) & 1, c= bit ^ sw;
  sw= bit;
  Pt A{c ? R1.X : R0.X, c ? R1.Z : R0.Z}, B{c ? R0.X : R1.X, c ? R0.Z : R1.Z};
  R1= xadd(m, A, B, P);
  R0= xdbl(m, A, a24);
 }
 return sw ? R1 : R0;
}

// 2 本の曲線の [k]P を同じ手順で同時に求める。ビットが同じなので入れ替えも共通で、xor とマスクで書く
// (gcc 15 は三項演算子で書くと入れ替えを分岐にした)。1 本だと鎖の長さで決まり、掛け算器が空くのを埋める。
inline void ladder2(const M60& m, Pt& Pa, Pt& Pb, u64 k, u64 a24a, u64 a24b) {
 Pt R0a= Pa, R1a= xdbl(m, Pa, a24a), R0b= Pb, R1b= xdbl(m, Pb, a24b);
 u64 sw= 0;
 for(int i= 62 - __builtin_clzll(k); i >= 0; --i) {
  const u64 bit= (k >> i) & 1, mk= 0 - (bit ^ sw);
  sw= bit;
  const u64 dax= (R0a.X ^ R1a.X) & mk, daz= (R0a.Z ^ R1a.Z) & mk, dbx= (R0b.X ^ R1b.X) & mk, dbz= (R0b.Z ^ R1b.Z) & mk;
  const Pt Aa{R0a.X ^ dax, R0a.Z ^ daz}, Ba{R1a.X ^ dax, R1a.Z ^ daz}, Ab{R0b.X ^ dbx, R0b.Z ^ dbz}, Bb{R1b.X ^ dbx, R1b.Z ^ dbz};
  R1a= xadd(m, Aa, Ba, Pa), R0a= xdbl(m, Aa, a24a);
  R1b= xadd(m, Ab, Bb, Pb), R0b= xdbl(m, Ab, a24b);
 }
 Pa= sw ? R1a : R0a, Pb= sw ? R1b : R0b;
}

template <int B1, int E2, int E3> struct Stage1 {
 std::array<u64, 16> c{};
 int len= 0;
 constexpr Stage1() {
  u64 cur= 1;
  for(int p= 2; p <= B1; ++p) {
   bool pr= true;
   for(int d= 2; d * d <= p; ++d)
    if(p % d == 0) pr= false;
   if(!pr) continue;
   int e= (p == 2 ? E2 : p == 3 ? E3 : 0);
   for(long long q= p; q <= B1; q*= p) ++e;
   for(int i= 0; i < e; ++i) {
    if(cur > ~0ull / p) c[len++]= cur, cur= 1;
    cur*= p;
   }
  }
  if(cur > 1) c[len++]= cur;
 }
};

constexpr int D= 210;
constexpr std::array<int, 24> BABY= {1, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103};

template <int MH> inline u64 stage2(const M60& m, Pt Q, u64 a24) {
 // odd[i] = [2i + 1]Q (i < 53)。[b] = [b - 4] + [4] (差 [b - 8]) の 2 本の鎖で作る。
 std::array<Pt, 53> odd;
 odd[0]= Q;
 const Pt Q2= xdbl(m, Q, a24), Q4= xdbl(m, Q2, a24);
 odd[1]= xadd(m, Q2, Q, Q);
 odd[2]= xadd(m, Q4, Q, odd[1]);
 odd[3]= xadd(m, Q4, odd[1], Q);
 for(int i= 4; i < 53; ++i) odd[i]= xadd(m, odd[i - 2], Q4, odd[i - 4]);
 std::array<u64, 24> bx, bz, bxz;
 for(int j= 0; j < 24; ++j) {
  const Pt B= odd[BABY[j] >> 1];
  bx[j]= B.X, bz[j]= B.Z, bxz[j]= m.mul(B.X, B.Z);
 }
 const Pt G1= xdbl(m, odd[52], a24);  // [210]Q
 Pt Gp= G1, Gc= xdbl(m, G1, a24);
 u64 acc[4]= {1, 1, 1, 1};
 auto pairs= [&](Pt G) {
  const u64 gxz= m.mul(G.X, G.Z);
  for(int j= 0; j < 24; ++j) {
   // (X_m - X_b)(Z_m + Z_b) - X_m Z_m + X_b Z_b = X_m Z_b - X_b Z_m。値は (0, 6n) で、2n 未満の acc との積は還元できる。
   const u64 t= m.mul(G.X - bx[j] + m.n2, G.Z + bz[j]) + bxz[j] - gxz + m.n2;
   acc[j & 3]= m.mul(acc[j & 3], t);
  }
 };
 pairs(Gp);
 pairs(Gc);
 for(int k= 3; k <= MH; ++k) {
  const Pt Gn= xadd(m, Gc, G1, Gp);
  Gp= Gc, Gc= Gn;
  pairs(Gc);
 }
 return m.mul(m.mul(acc[0], acc[1]), m.mul(acc[2], acc[3]));
}

// 奇数の合成数 n < 2^60 の自明でない因数を ECM で探す。見つからなければ 0。
template <int B1, int E2, int E3, int MH> inline u64 ecm(u64 n, int max_curves) {
 static constexpr Stage1<B1, E2, E3> S1{};
 const M60 m(n);
 const u64 c16= m.to(16);
 for(u64 sigma= 6; sigma < 6 + (u64)max_curves; ++sigma) {
  const u64 U= m.norm(m.to(sigma * sigma - 5)), V= m.norm(m.to(4 * sigma));
  const u64 U3= m.mul(m.mul(U, U), U), V3= m.mul(m.mul(V, V), V);
  const u64 vu= V - U + m.n, w= 3 * U + V;  // どちらも 4n 未満
  const u64 num= m.mul(m.mul(m.mul(vu, vu), vu), w), den= m.mul(m.mul(U3, V), c16);
  const auto [g, inv]= inv_gcd(m.from(den), n);
  if(g != 1) {
   if(g != n) return g;
   continue;
  }
  const u64 a24= m.mul(num, m.to(inv));
  Pt P{U3, V3};
  for(int i= 0; i < S1.len; ++i) P= ladder(m, P, S1.c[i], a24);
  const u64 g1= gcd(m.norm(P.Z), n);
  if(g1 != 1) {
   if(g1 != n) return g1;
   continue;
  }
  const u64 g2= gcd(m.norm(stage2<MH>(m, P, a24)), n);
  if(g2 != 1 && g2 != n) return g2;
 }
 return 0;
}

// σ の曲線を作る。0 なら (P, a24) が使え、1 なら f に因数、-1 ならこの σ は使えない。
inline int curve(const M60& m, u64 n, u64 sigma, u64 c16, Pt& P, u64& a24, u64& f) {
 const u64 U= m.norm(m.to(sigma * sigma - 5)), V= m.norm(m.to(4 * sigma));
 const u64 U3= m.mul(m.mul(U, U), U), V3= m.mul(m.mul(V, V), V), vu= V - U + m.n, w= 3 * U + V;
 const u64 num= m.mul(m.mul(m.mul(vu, vu), vu), w), den= m.mul(m.mul(U3, V), c16);
 const auto [g, inv]= inv_gcd(m.from(den), n);
 if(g != 1) return f= g, g != n ? 1 : -1;
 a24= m.mul(num, m.to(inv)), P= {U3, V3};
 return 0;
}
// 2 本の曲線を同時に回す ECM。gcd は 2 本の積でまとめて取り、n ごと割れたときだけ 1 本ずつ見る。
template <int B1, int E2, int E3, int MH> inline u64 ecm_dual(u64 n) {
 static constexpr Stage1<B1, E2, E3> S1{};
 const M60 m(n);
 const u64 c16= m.to(16);
 for(u64 sigma= 6;; sigma+= 2) {
  Pt Pa, Pb;
  u64 a24a, a24b, f;
  const int sa= curve(m, n, sigma, c16, Pa, a24a, f);
  if(sa == 1) return f;
  const int sb= curve(m, n, sigma + 1, c16, Pb, a24b, f);
  if(sb == 1) return f;
  if(sa || sb) continue;
  for(int i= 0; i < S1.len; ++i) ladder2(m, Pa, Pb, S1.c[i], a24a, a24b);
  auto check= [&](u64 za, u64 zb) -> u64 {  // za zb の gcd。n ごとなら 1 本ずつ
   const u64 g= gcd(m.norm(m.mul(za, zb)), n);
   if(g == 1 || g != n) return g == 1 ? 0 : g;
   for(u64 z: {za, zb})
    if(const u64 h= gcd(m.norm(z), n); h != 1 && h != n) return h;
   return n;
  };
  if(const u64 g= check(Pa.Z, Pb.Z); g) {
   if(g != n) return g;
   continue;
  }
  const u64 g2= check(stage2<MH>(m, Pa, a24a), stage2<MH>(m, Pb, a24b));
  if(g2 && g2 != n) return g2;
 }
}

// Pollard rho (Brent)。c の違う 2 本の列を同時に進め、M 段ごとに差の積の gcd を取る。奇数の合成数 n < 2^60。
inline u64 rho(u64 n) {
 const u64 ninv= inv64(n);
 auto mul= [&](u64 a, u64 b) {
  u128 t= (u128)a * b;
  return u64(t >> 64) + n - u64((u128)(u64(t) * ninv) * n >> 64);
 };
 constexpr u64 M= 128;
 for(u64 c1= 1, c2= 2;; c1+= 2, c2+= 2) {
  u64 z1= c1, z2= c2, ys1= 0, ys2= 0, x1= 0, x2= 0, g= 1;
  for(u64 k= M;; k<<= 1) {
   x1= z1 + 2 * n, x2= z2 + 2 * n;  // z は (c, 2n + c)、保存した z は c より大きいので x - z は (0, 4n)
   bool found= false;
   for(u64 j= 0; j < k && !found; j+= M) {
    ys1= z1, ys2= z2;
    u64 q1= 1, q2= 1;
    for(u64 i= 0; i < M; ++i) {
     z1= mul(z1, z1) + c1, z2= mul(z2, z2) + c2;
     q1= mul(q1, x1 - z1), q2= mul(q2, x2 - z2);
    }
    g= gcd(mul(q1, q2) % n, n);
    found= g != 1;
   }
   if(!found) continue;
   if(g != n) return g;
   for(int s= 0; s < 2; ++s) {  // M 段を 1 段ずつやり直す
    u64 y= s ? ys2 : ys1, x= s ? x2 : x1, c= s ? c2 : c1;
    for(u64 i= 0; i < M; ++i) {
     y= mul(y, y) + c;
     const u64 gg= gcd((x - y) % n, n);
     if(gg != 1) {
      if(gg != n) return gg;
      break;
     }
    }
   }
   break;  // どちらの列も n ごと割れたので c を変える
  }
 }
}

// n >= 2^60 用の遅い rho (Floyd、128 bit の剰余)。
inline u64 rho_slow(u64 n) {
 for(u64 c= 1;; ++c) {
  auto f= [&](u64 x) { return u64(((u128)x * x + c) % n); };
  u64 x= 2, y= 2, g= 1;
  while(g == 1) x= f(x), y= f(f(y)), g= gcd(x > y ? x - y : y - x, n);
  if(g != n) return g;
 }
}

template <int T, int B1, int E2, int E3, int MH, bool DUAL> inline void rec(u64 n, std::vector<u64>& out) {
 if(n == 1) return;
 if(is_prime(n)) return out.push_back(n);
 u64 d= 0;
 if(n >= (1ull << 60)) d= rho_slow(n);
 else if(n >= (1ull << T)) d= DUAL ? ecm_dual<B1, E2, E3, MH>(n) : ecm<B1, E2, E3, MH>(n, 1 << 20);
 if(!d) d= rho(n);
 rec<T, B1, E2, E3, MH, DUAL>(d, out), rec<T, B1, E2, E3, MH, DUAL>(n / d, out);
}

// 97 以下の奇素数 p について、p で割り切れるかを n p^{-1} mod 2^64 <= (2^64 - 1) / p で見る。
struct SmallPrime {
 u64 p, inv, lim;
};
inline constexpr auto SMALL= [] {
 std::array<SmallPrime, 24> t{};
 int k= 0;
 for(u64 p= 3; p < 100; p+= 2) {
  bool pr= true;
  for(u64 d= 3; d * d <= p; d+= 2)
   if(p % d == 0) pr= false;
  if(pr) t[k++]= {p, inv64(p), ~0ull / p};
 }
 return t;
}();

template <int T= 40, bool DUAL= false, int B1= 150, int E2= 3, int E3= 2, int MH= 16> inline std::vector<u64> factorize(u64 n) {
 std::vector<u64> out;
 if(n <= 1) return out;
 const int z= __builtin_ctzll(n);
 out.insert(out.end(), z, 2);
 n>>= z;
 for(const auto& [p, inv, lim]: SMALL) {
  if(p * p > n) break;
  while(n * inv <= lim) out.push_back(p), n*= inv;
 }
 if(n > 1) rec<T, B1, E2, E3, MH, DUAL>(n, out);
 std::sort(out.begin(), out.end());
 return out;
}
}  // namespace ecm_fact
