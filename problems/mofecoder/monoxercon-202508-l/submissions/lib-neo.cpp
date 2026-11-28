// Monoxer L FF?DLP。(A + Bi)^k = C + Di (mod p) の最小の k >= 0 を、モノイド作用の一般版で解く。
//
// 複素数 x + yi を F_p[i] の元として掛け算で作用させる。p = 1 (mod 4) なら F_p x F_p、p = 3 (mod 4) なら F_{p^2}。
// yuki 950 (2x2 行列の累乗) と同じく、ノルム (行列式) x^2 + y^2 の離散対数で k の剰余を決め、そのあと
// ノルム 1 の元の累乗を BSGS で解く。
//   ノルム 0 の元 (零因子) は、1, z, z^2, ... の周期が p - 1 以下なので、そのまま N = 2p + 1 で解く。
//   ノルムが可逆なら、r = det(z)^k = det(w) の最小の k (r)、m0 = det(z) の位数を出し、X = z^m0, S = z^r として
//   X^q S = w の最小の q を解き、k = m0 q + r とする。X のノルムは 1 なので位数は p + 1 以下 (p = 1 mod 4 なら p - 1 以下)。
#include "pj.hpp"
#include "neo/algo/compute/DiscreteLogAction.hpp"
using namespace std;
using Cx = array<long long, 2>;

signed main() {
  long long A, B, C, D, p;
  must_scan(scanf("%lld %lld %lld %lld %lld", &A, &B, &C, &D, &p), 5);
  auto mul = [p](Cx x, Cx y) {
    long long re = (x[0] * y[0] - x[1] * y[1]) % p;
    if (re < 0) re += p;
    long long im = (x[0] * y[1] + x[1] * y[0]) % p;
    return Cx{re, im};
  };
  auto cpow = [&](Cx x, long long e) {
    Cx r{1 % p, 0};
    while (e) {
      if (e & 1) r = mul(r, x);
      x = mul(x, x);
      e >>= 1;
    }
    return r;
  };
  auto norm = [p](Cx x) { return (x[0] * x[0] + x[1] * x[1]) % p; };
  auto hash = [](Cx x) { return (unsigned)x[0]; };
  const Cx z{A % p, B % p}, w{C % p, D % p};
  const long long det_z = norm(z), det_w = norm(w);
  long long ans = -1;
  if (det_z == 0) {
    DiscreteLogAction dlog(mul, mul, hash, 2 * p + 1);
    ans = dlog(z, Cx{1 % p, 0}, w, 2 * p + 1);
  } else {
    auto sact = [p](long long x, long long y) { return x * y % p; };
    auto shash = [](long long x) { return (unsigned)x; };
    DiscreteLogAction dlog_s(sact, sact, shash, p);
    const long long r = dlog_s(det_z, 1 % p, det_w, p);
    // 位数は正の最小の m。dlog は 0 を返しうるので、det_z から 1 へ戻る最小の歩数 + 1 にする。
    const long long m0 = dlog_s(det_z, det_z, 1 % p, p) + 1;
    if (r >= 0) {
      const Cx X = cpow(z, m0), S = cpow(z, r);
      DiscreteLogAction dlog_c(mul, mul, hash, 2 * p + 1);
      const long long q = dlog_c(X, S, w, 2 * p + 1);
      if (q >= 0) ans = m0 * q + r;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
