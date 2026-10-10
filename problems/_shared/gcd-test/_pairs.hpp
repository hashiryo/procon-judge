#pragma once
// gcd-test の族のハーネスが、ケースの 1 行 (N amode bmode gmode seed) から組 (a_i, b_i) を作る。
// 作るのは計測区間の外で、乱数は splitmix64 (LCG の下位ビットは周期が短いので使わない)。
//
// 値の幅 W は 64 か 32 で、32 bit の問題は W = 32 で作る。
// mode の書き方:
//   uB (B = 1..W) は [0, 2^B) の一様。
//   l は桁数 L を 1..W から一様に選び、[2^{L-1}, 2^L) の一様 (大きさの違う組が混ざる)。
//   e は角の値の表 EDGE のうち 2^W 未満のものから一様。
//   p は素数の表 PRIMES のうち 2^W 未満のものから一様。
//   r は [0, b) の一様で、a にだけ使える (b を先に作る)。b = 0 なら 0。
// gmode が 1 でなければ、gmode で作った g (0 なら 1) を a と b の両方に掛ける。
// 掛けると 2^W 以上になる組は掛けずに残す。
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace gcd_test {
using u64= unsigned long long;
struct SplitMix64 {
 u64 s;
 u64 next() {
  u64 z= (s+= 0x9e3779b97f4a7c15ull);
  z= (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
  z= (z ^ (z >> 27)) * 0x94d049bb133111ebull;
  return z ^ (z >> 31);
 }
};
inline constexpr u64 EDGE[]= {
 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 17, 31, 32, 33, 63, 64, 65, 255, 256, 65535, 65536, 65537,
 (1ull << 31) - 1, 1ull << 31, (1ull << 32) - 1, 1ull << 32, (1ull << 32) + 1,
 (1ull << 61) - 1, 1ull << 62, (1ull << 62) + 1, 3ull << 62, (1ull << 63) - 1, 1ull << 63, (1ull << 63) + 1,
 ~0ull, ~0ull - 1, ~0ull - 2,
 18446744073709551557ull,  // 2^64 未満で最大の素数
 18446744069414584321ull,  // 2^64 - 2^32 + 1 (素数)
 998244353, 1000000007,
 7540113804746346429ull, 12200160415121876738ull,  // F_92, F_93 (Euclid の互除法が最も長くなる組)
 0x5555555555555555ull, 0xaaaaaaaaaaaaaaaaull,
 12157665459056928801ull,  // 3^40
 14975624970497949696ull,  // 3^20 2^32
 614889782588491410ull,    // 47 以下の素数の積
};
inline constexpr u64 PRIMES[]= {
 3, 5, 7, 65537, 998244353, 1000000007, 1000000009, 2147483647, 4294967291ull,
 2305843009213693951ull, 4611686018427387847ull, 9223372036854775783ull, 18446744073709551557ull, 18446744069414584321ull,
};
struct Mode {
 char kind;  // 'u', 'l', 'e', 'p', 'r', '1'
 int bits;
};
inline Mode parse_mode(const std::string& s, int W) {
 if(s == "l" || s == "e" || s == "p" || s == "r" || s == "1") return {s[0], 0};
 if(s.size() >= 2 && s[0] == 'u') {
  int b= std::atoi(s.c_str() + 1);
  if(1 <= b && b <= W) return {'u', b};
 }
 std::fprintf(stderr, "unknown mode: %s\n", s.c_str());
 std::exit(1);
}
struct Gen {
 int W;
 u64 lim;  // 2^W - 1
 std::vector<u64> edge, primes;
 explicit Gen(int W_): W(W_), lim(W_ == 64 ? ~0ull : (1ull << W_) - 1) {
  for(u64 x: EDGE)
   if(x <= lim) edge.push_back(x);
  for(u64 x: PRIMES)
   if(x <= lim) primes.push_back(x);
 }
 u64 draw(Mode m, SplitMix64& rng, u64 b) const {
  switch(m.kind) {
  case 'u': return rng.next() >> (64 - m.bits);
  case 'l': {
   int L= int(rng.next() % W) + 1;
   return (rng.next() >> (64 - L)) | (1ull << (L - 1));
  }
  case 'e': return edge[rng.next() % edge.size()];
  case 'p': return primes[rng.next() % primes.size()];
  case 'r': return b ? rng.next() % b : 0;
  default: return 1;
  }
 }
};
// b_positive なら b = 0 の組を b = 1 にする (inv_gcd は b >= 1 を前提にする)。
inline void make_pairs(u64 n, const std::string& am, const std::string& bm, const std::string& gm, u64 seed, std::vector<u64>& as, std::vector<u64>& bs, bool b_positive, int W= 64) {
 const Mode ma= parse_mode(am, W), mb= parse_mode(bm, W), mg= parse_mode(gm, W);
 if(mb.kind == 'r') {
  std::fprintf(stderr, "mode r is only for a\n");
  std::exit(1);
 }
 const Gen gen(W);
 SplitMix64 rng{seed};
 as.resize(n), bs.resize(n);
 for(u64 i= 0; i < n; ++i) {
  u64 b= gen.draw(mb, rng, 0), a= gen.draw(ma, rng, b);
  if(mg.kind != '1') {
   u64 g= gen.draw(mg, rng, 0), ga, gb;
   if(g == 0) g= 1;
   if(!__builtin_mul_overflow(a, g, &ga) && !__builtin_mul_overflow(b, g, &gb) && ga <= gen.lim && gb <= gen.lim) a= ga, b= gb;
  }
  if(b_positive && b == 0) b= 1;
  as[i]= a, bs[i]= b;
 }
}
}  // namespace gcd_test
