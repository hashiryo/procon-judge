// Library の GF2p64 の log (Log<0>::ln) の前半だけを使う版。log は a を素数ごとの部分群へ写してから、それぞれで離散対数を
// 求めるが、位数には「写した先が 1 か」だけが要るので、BSGS、表引き (LN641、CLS65537)、CRT を省く。
//   65537: n = a^(1+2^32) が GF(2^16) に入るか (F16(n) == n)
//   3、5、17、257: GF(2^16) へのノルムの log16 (IL16 を引いた下位 16 bit) を割り切るか
//   641、6700417: ln と同じ短い手順で作る z、y が 1 か
// Library の内部 (gf2p64_internal) の名前を使うので、Library の内部が変わると組めなくなることがある。
#include "mylib/algebra/GF2p64.hpp"
#include <vector>
using namespace std;
using u64= unsigned long long;
namespace gf2_64_ord_proj {
using namespace gf2p64_internal;
template <bool V> inline u64 ord(u64 x) {
 using L= Log<0>;
 const u64 x32= F32(x), n= mul(x, x32), fn= F16(n);
 auto [x_f16, w]= unpack(mul2<V>(_mm256_set_epi64x(0, sq(x32), 0, n), _mm256_set1_epi64x(fn)));
 const u32 il= L::IL16.t[u16(x_f16)], la= il & 65535;
 const u64 s= mul(EMB(u16(il >> 16)), w), w1= mul(s, L::F9(s)), e57= L::F57(w1);
 auto [w2, y]= unpack(mul2<V>(_mm256_set_epi64x(0, e57, 0, sq(w1)), _mm256_set_epi64x(0, s, 0, w1)));
 const u64 w3= mul(s, sq(w2)), w4= mul(w3, F4(w3)), z= mul(e57, w4);
 return u64(la % 3 ? 3 : 1) * (la % 5 ? 5 : 1) * (la % 17 ? 17 : 1) * (la % 257 ? 257 : 1) * (fn != n ? 65537 : 1) * (z != 1 ? 641 : 1) * (y != 1 ? 6700417 : 1);
}
template <bool V> inline void solve_all(const vector<u64>& as, vector<u64>& ans) {
 for(size_t i= 0; i < as.size(); ++i) ans[i]= ord<V>(as[i]);
}
}  // namespace gf2_64_ord_proj
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
#ifdef __x86_64__
 if(__builtin_cpu_supports("vpclmulqdq")) return gf2_64_ord_proj::solve_all<1>(as, ans), ans;
#endif
 return gf2_64_ord_proj::solve_all<0>(as, ans), ans;
}
