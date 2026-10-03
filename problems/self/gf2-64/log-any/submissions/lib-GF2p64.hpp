#include "mylib/algebra/GF2p64.hpp"
#include <numeric>
#include <vector>
using namespace std;
using u64= unsigned long long;
// A = log a、B = log b として A·k ≡ B (mod 2^64-1) を解く。d = gcd(A, 2^64-1) が B を割り切らなければ
// 解なし。割り切れば k = (B/d)·(A/d)^-1 mod ((2^64-1)/d) で、これが最小の解 ((2^64-1)/d = ord(a))。
inline u64 solve_linear(u64 A, u64 B) {
 constexpr u64 M= ~0ull;
 const u64 d= gcd(A, M);  // A = 0 (a = 1) なら d = M
 if(B % d) return M;
 const u64 m= M / d;
 if(m == 1) return 0;
 // 拡張ユークリッドで (A/d)^-1 mod m。余りは u64 に収まるので、128 bit で持つのは係数だけにする
 // (128 bit の割り算は遅い)。
 u64 r0= m, r1= A / d % m;
 __int128 s0= 0, s1= 1;
 while(r1) {
  const u64 q= r0 / r1;
  tie(r0, r1)= make_pair(r1, r0 - q * r1);
  tie(s0, s1)= make_pair(s1, s0 - __int128(q) * s1);
 }
 const u64 inv= u64((s0 % __int128(m) + m) % m);
 return u64(__uint128_t(B / d % m) * inv % m);
}
inline vector<u64> run(const vector<u64>& as, const vector<u64>& bs) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= solve_linear(GF2p64(as[i]).log(), GF2p64(bs[i]).log());
 return ans;
}
