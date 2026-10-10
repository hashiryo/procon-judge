#pragma once
#include "_shared/modulo-test/_common.hpp"
// Stein の方法の拡張版。Kaliski の almost inverse (1995) と同じく、値を 2^c で割る代わりにもう一方の係数を 2^c 倍し、
// 最後に 2^{-k} を Montgomery の還元で掛けて戻す。値は Library の binary_gcd と同じく
// (大きいほう, 小さいほう) → (差 >> c, 小さいほう) の形で持ち、鎖は引き算、tzcnt、シフトの 3 段のまま。
//
// B を奇数として、2 つの枠 (値 a, 係数 ca) と (値 b, 係数 cb) は、符号 σ_a = -σ_b と k について
//   A ca ≡ σ_a a 2^k,  A cb ≡ σ_b b 2^k  (mod B),  B = a cb + b ca
// を保つ。最後の式の項がすべて非負なので、係数はずっと B 以下に収まり、u64 全体の入力を 128 bit なしで扱える。
// 終わりは a = b = g で、B / g = ca + cb、(A / g) cb ≡ σ_b 2^k (mod B / g) となる。
//
// T は割り算を混ぜる桁数の差。A が B より T bit を超えて長ければ A %= B を、B が A より長ければ互除法の 1 段
// (B = q A + r、ca = q) を先に済ませる。どちらも上の式を保つ。T >= 64 なら割り算を使わない。
namespace kaliski {
constexpr u64 inv64(u64 n) {
 u64 x= (3 * n) ^ 2, y= 1 - n * x;
 x*= 1 + y, y*= y;
 x*= 1 + y, y*= y;
 x*= 1 + y, y*= y;
 return x * (1 + y);
}
// v 2^{-c} mod M を [0, M] で返す。0 <= c <= 64、v <= M、M は奇数、Minv = M^{-1} mod 2^64。
inline u64 redc_c(u64 v, int c, u64 M, u64 Minv) {
 u128 t= (u128)v << (64 - c);
 u64 hi= u64(t >> 64), mh= u64((u128)(u64(t) * Minv) * M >> 64);
 return hi >= mh ? hi - mh : hi - mh + M;
}
struct Res {
 u64 g, x, M;  // A x ≡ g (mod B)、0 <= x < M = B / g
};
// B は奇数、Binv = B^{-1} mod 2^64。gcc は大きさを見て展開しないことがあるので、展開を強いる。
template <int T> [[gnu::always_inline]] inline Res odd(u64 A, u64 B, u64 Binv) {
 if(T < 64 && A > B && __builtin_clzll(B) - __builtin_clzll(A) > T) A%= B;
 if(A == 0) return {B, 0, 1};
 int k= __builtin_ctzll(A);
 u64 a= B, b= A >> k, ca= 0, cb= 1, neg= 0;
 if(T < 64 && __builtin_clzll(b) - __builtin_clzll(a) > T) {
  u64 q= a / b, r= a % b;
  if(r == 0) a= b, ca= q - 1;
  else {
   int c= __builtin_ctzll(r);
   a= r >> c, ca= q, cb<<= c, k+= c;
  }
 }
 while(a != b) {
  u64 d= a - b, e= b - a;
  int c= __builtin_ctzll(d);
  u64 mn= std::min(a, b), t= a > b ? d : e, cm= a > b ? cb : ca;
  neg+= a < b;  // 入れ替えの回数。足し算で書くと adc 1 つになる (xor だと setb、movzbl、xor の 3 つ)
  ca+= cb, cb= cm << c, k+= c;
  a= t >> c, b= mn;
 }
 const u64 M= ca + cb, Minv= Binv * b;
 const int c1= k >> 1;
 u64 x= redc_c(redc_c(cb, c1, M, Minv), k - c1, M, Minv);
 x= x >= M ? x - M : x;
 return {b, (neg & 1) && x ? M - x : x, M};
}
// odd を呼ぶところを 1 か所にする。3 か所に分けていた最初の版 (procon-judge 7d333e11) は、
// T = 8 と 16 のとき gcc 15 が odd を展開せず、結果をメモリ経由で返して 1 回 3 ns ほど遅かった。
template <int T> inline std::pair<u64, u64> inv_gcd(u64 a, u64 b) {
 // 共通の 2 の冪を除く (b が奇数なら z = 0)。a = 0 なら a1 = 0 で、odd が (b1, 0) を返す。
 const int z= __builtin_ctzll(a | b);
 const u64 a1= a >> z, b1= b >> z;
 // b1 が偶数なら a1 は奇数なので、役を入れ替えて b1 y ≡ g1 (mod a1) を解き、a1 x + b1 y = g1 から x を出す。
 const bool sw= !(b1 & 1);
 const u64 B= sw ? a1 : b1, Binv= inv64(B);
 const Res r= odd<T>(sw ? b1 : a1, B, Binv);
 if(!sw) return {r.g << z, r.x};
 // t = (b1 y - g1) / a1 は割り切れて -1 <= t < b1 / g1 なので、mod 2^64 で a1^{-1} を掛けて求まる。
 const u64 t= (b1 * r.x - r.g) * Binv, m= b1 * (r.M * Binv);  // m = b1 / g1 (g1^{-1} = (a1 / g1) a1^{-1})
 const u64 x= m - t;
 return {r.g << z, x >= m ? x - m : x};
}
}  // namespace kaliski
