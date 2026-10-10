#pragma once
// _ecm.hpp の 2 本同時の ECM (ecm_dual) に、掛け算を減らす手を 2 つ足したもの。2 本同時にすると x64 は掛け算器の
// throughput で決まる側に移った (1 本ずつより 14 % 速く、掛け算 1 回 1 サイクルの見積もりと合う) ので、数を減らすと効くはず。
//
// NORM: stage 1 を、B1 以下の素数冪の積全体 (約 220 bit) の 1 本の ladder にし、始点を z = 1 に正規化する。
//   差の点の Z が 1 なので xADD の掛け算が 6 回から 5 回に減り、1 段 (2 本分) は 22 回から 20 回になる。
//   2 本の曲線の (A + 2)/4 と始点の x の逆元は、Montgomery の技で 1 回の inv_gcd にまとめる。
// S2: stage 2 の baby step を 6 刻みの 2 本の鎖 ([b] = [b - 6] + [6]) で作り (点の足し算 51 回 → 36 回)、
//   mD ± b のどちらも (B1, B2] の素数でない組は飛ばす (D = 210、m <= 16 で 384 組のうち 66 組)。
#include "_ecm.hpp"

namespace ecm_fact {
// B1 以下の素数冪の積 (2 と 3 は E2、E3 だけ余分) を多倍長で持つ。w は下の語から。
template <int B1, int E2, int E3> struct BigK {
 std::array<u64, 8> w{};
 int bits= 0;
 constexpr BigK() {
  w[0]= 1;
  for(int p= 2; p <= B1; ++p) {
   bool pr= true;
   for(int d= 2; d * d <= p; ++d)
    if(p % d == 0) pr= false;
   if(!pr) continue;
   int e= (p == 2 ? E2 : p == 3 ? E3 : 0);
   for(long long q= p; q <= B1; q*= p) ++e;
   for(int i= 0; i < e; ++i) {
    u64 carry= 0;
    for(auto& x: w) {
     const u128 t= (u128)x * p + carry;
     x= u64(t), carry= u64(t >> 64);
    }
   }
  }
  for(int i= 7; i >= 0; --i)
   if(w[i]) {
    bits= 64 * i + 64 - __builtin_clzll(w[i]);
    break;
   }
 }
};
// P + Q。差 P - Q が (xd : 1)。
inline Pt xadd1(const M60& m, Pt P, Pt Q, u64 xd) {
 u64 u= m.mul(P.X - P.Z + m.n2, Q.X + Q.Z), v= m.mul(P.X + P.Z, Q.X - Q.Z + m.n2);
 u64 w= u + v, y= u - v + m.n2;
 return {m.mul(w, w), m.mul(xd, m.mul(y, y))};
}
// 始点 (xa : 1)、(xb : 1) の 2 本に [K]P を同時に掛ける。
template <int B1, int E2, int E3> inline void ladder2n(const M60& m, u64 xa, u64 xb, u64 one, u64 a24a, u64 a24b, Pt& outa, Pt& outb) {
 static constexpr BigK<B1, E2, E3> K{};
 const Pt Pa{xa, one}, Pb{xb, one};
 Pt R0a= Pa, R1a= xdbl(m, Pa, a24a), R0b= Pb, R1b= xdbl(m, Pb, a24b);
 u64 sw= 0;
 for(int i= K.bits - 2; i >= 0; --i) {
  const u64 bit= (K.w[i >> 6] >> (i & 63)) & 1, mk= 0 - (bit ^ sw);
  sw= bit;
  const u64 dax= (R0a.X ^ R1a.X) & mk, daz= (R0a.Z ^ R1a.Z) & mk, dbx= (R0b.X ^ R1b.X) & mk, dbz= (R0b.Z ^ R1b.Z) & mk;
  const Pt Aa{R0a.X ^ dax, R0a.Z ^ daz}, Ba{R1a.X ^ dax, R1a.Z ^ daz}, Ab{R0b.X ^ dbx, R0b.Z ^ dbz}, Bb{R1b.X ^ dbx, R1b.Z ^ dbz};
  R1a= xadd1(m, Aa, Ba, xa), R0a= xdbl(m, Aa, a24a);
  R1b= xadd1(m, Ab, Bb, xb), R0b= xdbl(m, Ab, a24b);
 }
 outa= sw ? R1a : R0a, outb= sw ? R1b : R0b;
}
// m ごとに、mD ± b のどちらかが (B1, MH D + 103] の素数になる BABY の添字の集合。
template <int B1, int MH> struct S2Mask {
 std::array<u32, MH + 1> mask{};
 constexpr S2Mask() {
  auto isp= [](int x) {
   if(x < 2) return false;
   for(int d= 2; d * d <= x; ++d)
    if(x % d == 0) return false;
   return true;
  };
  for(int k= 1; k <= MH; ++k)
   for(int j= 0; j < 24; ++j) {
    const int lo= k * D - BABY[j], hi= k * D + BABY[j];
    if((lo > B1 && isp(lo)) || (hi > B1 && isp(hi))) mask[k]|= 1u << j;
   }
 }
};
template <int B1, int MH> inline u64 stage2opt(const M60& m, Pt Q, u64 a24) {
 static constexpr S2Mask<B1, MH> MK{};
 // odd[b >> 1] = [b]Q。b ≡ 1 (mod 6) と b ≡ 5 (mod 6) の 2 本の鎖を [6] 刻みで伸ばす。差は鎖の 2 つ前 (最初は [5] と [1])。
 std::array<Pt, 53> odd{};
 const Pt Q2= xdbl(m, Q, a24), Q3= xadd(m, Q2, Q, Q), Q5= xadd(m, Q3, Q2, Q), Q6= xdbl(m, Q3, a24);
 odd[0]= Q, odd[2]= Q5;
 Pt pa= Q5, ca= Q, pb= Q, cb= Q5;
 for(int b= 7; b <= 103; b+= 6) {
  const Pt na= xadd(m, ca, Q6, pa);
  odd[b >> 1]= na, pa= ca, ca= na;
  if(b + 4 <= 101) {
   const Pt nb= xadd(m, cb, Q6, pb);
   odd[(b + 4) >> 1]= nb, pb= cb, cb= nb;
  }
 }
 std::array<u64, 24> bx, bz, bxz;
 for(int j= 0; j < 24; ++j) {
  const Pt B= odd[BABY[j] >> 1];
  bx[j]= B.X, bz[j]= B.Z, bxz[j]= m.mul(B.X, B.Z);
 }
 const Pt G1= xdbl(m, xadd(m, odd[51], Q2, odd[50]), a24);  // [210] = 2 ([103] + [2])、差 [101]
 Pt Gp= G1, Gc= xdbl(m, G1, a24);
 u64 acc[4]= {1, 1, 1, 1};
 unsigned c= 0;
 auto pairs= [&](Pt G, u32 mk) {
  const u64 gxz= m.mul(G.X, G.Z);
  for(; mk; mk&= mk - 1, ++c) {
   const int j= __builtin_ctz(mk);
   const u64 t= m.mul(G.X - bx[j] + m.n2, G.Z + bz[j]) + bxz[j] - gxz + m.n2;
   acc[c & 3]= m.mul(acc[c & 3], t);
  }
 };
 pairs(Gp, MK.mask[1]);
 pairs(Gc, MK.mask[2]);
 for(int k= 3; k <= MH; ++k) {
  const Pt Gn= xadd(m, Gc, G1, Gp);
  Gp= Gc, Gc= Gn;
  pairs(Gc, MK.mask[k]);
 }
 return m.mul(m.mul(acc[0], acc[1]), m.mul(acc[2], acc[3]));
}
template <int B1, int MH, bool S2> inline u64 stage2x(const M60& m, Pt Q, u64 a24) {
 if constexpr(S2) return stage2opt<B1, MH>(m, Q, a24);
 else return stage2<MH>(m, Q, a24);
}
// σ の曲線の値を作る (逆元はまだ取らない)。
struct CurveRaw {
 u64 U3, V3, num, den;
};
inline CurveRaw curve_raw(const M60& m, u64 sigma, u64 c16) {
 const u64 U= m.norm(m.to(sigma * sigma - 5)), V= m.norm(m.to(4 * sigma));
 const u64 U3= m.mul(m.mul(U, U), U), V3= m.mul(m.mul(V, V), V), vu= V - U + m.n, w= 3 * U + V;
 return {U3, V3, m.mul(m.mul(m.mul(vu, vu), vu), w), m.mul(m.mul(U3, V), c16)};
}
template <int B1, int E2, int E3, int MH, bool NORM, bool S2> inline u64 ecm_dual2(u64 n) {
 static constexpr Stage1<B1, E2, E3> S1{};
 const M60 m(n);
 const u64 c16= m.to(16), one= m.to(1);
 auto check= [&](u64 za, u64 zb) -> u64 {
  const u64 g= gcd(m.norm(m.mul(za, zb)), n);
  if(g != n) return g == 1 ? 0 : g;
  for(u64 z: {za, zb})
   if(const u64 h= gcd(m.norm(z), n); h != 1 && h != n) return h;
  return n;
 };
 for(u64 sigma= 6;; sigma+= 2) {
  Pt Pa, Pb;
  u64 a24a, a24b;
  if constexpr(NORM) {
   const CurveRaw ca= curve_raw(m, sigma, c16), cb= curve_raw(m, sigma + 1, c16);
   // 1 / (den V3) を 2 本分まとめて 1 回で求め、(A + 2)/4 = num / den と始点の x = U3 / V3 を作る。
   const u64 ta= m.mul(ca.den, ca.V3), tb= m.mul(cb.den, cb.V3);
   const auto [g, inv]= inv_gcd(m.from(m.mul(ta, tb)), n);
   if(g != 1) {
    if(g != n) return g;
    continue;
   }
   const u64 it= m.to(inv), ia= m.mul(it, tb), ib= m.mul(it, ta);
   a24a= m.mul(ca.num, m.mul(ia, ca.V3)), a24b= m.mul(cb.num, m.mul(ib, cb.V3));
   const u64 xa= m.mul(ca.U3, m.mul(ia, ca.den)), xb= m.mul(cb.U3, m.mul(ib, cb.den));
   ladder2n<B1, E2, E3>(m, xa, xb, one, a24a, a24b, Pa, Pb);
  } else {
   u64 f;
   const int sa= curve(m, n, sigma, c16, Pa, a24a, f);
   if(sa == 1) return f;
   const int sb= curve(m, n, sigma + 1, c16, Pb, a24b, f);
   if(sb == 1) return f;
   if(sa || sb) continue;
   for(int i= 0; i < S1.len; ++i) ladder2(m, Pa, Pb, S1.c[i], a24a, a24b);
  }
  if(const u64 g= check(Pa.Z, Pb.Z); g) {
   if(g != n) return g;
   continue;
  }
  const u64 g2= check(stage2x<B1, MH, S2>(m, Pa, a24a), stage2x<B1, MH, S2>(m, Pb, a24b));
  if(g2 && g2 != n) return g2;
 }
}
inline u64 isqrt(u64 n) {
 u64 r= std::sqrt((double)n);
 if(r > 0xffffffffull) r= 0xffffffffull;
 while(r * r > n) --r;
 while(r < 0xffffffffull && (r + 1) * (r + 1) <= n) ++r;
 return r;
}
template <int T, int B1, int E2, int E3, int MH, bool NORM, bool S2> inline void rec2(u64 n, std::vector<u64>& out) {
 if(n == 1) return;
 if(is_prime(n)) return out.push_back(n);
 if(const u64 r= isqrt(n); r * r == n) return rec2<T, B1, E2, E3, MH, NORM, S2>(r, out), rec2<T, B1, E2, E3, MH, NORM, S2>(r, out);
 u64 d= 0;
 if(n >= (1ull << 60)) d= rho_slow(n);
 else if(n >= (1ull << T)) d= ecm_dual2<B1, E2, E3, MH, NORM, S2>(n);
 else d= rho(n);
 rec2<T, B1, E2, E3, MH, NORM, S2>(d, out), rec2<T, B1, E2, E3, MH, NORM, S2>(n / d, out);
}
template <bool NORM, bool S2, int B1= 150, int MH= 16, int T= 48, int E2= 3, int E3= 2> inline std::vector<u64> factorize2(u64 n) {
 std::vector<u64> out;
 if(n <= 1) return out;
 const int z= __builtin_ctzll(n);
 out.insert(out.end(), z, 2);
 n>>= z;
 for(const auto& [p, inv, lim]: SMALL) {
  if(p * p > n) break;
  while(n * inv <= lim) out.push_back(p), n*= inv;
 }
 if(n > 1) rec2<T, B1, E2, E3, MH, NORM, S2>(n, out);
 std::sort(out.begin(), out.end());
 return out;
}
}  // namespace ecm_fact
