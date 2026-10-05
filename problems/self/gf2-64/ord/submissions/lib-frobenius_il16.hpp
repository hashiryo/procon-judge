// lib-frobenius.hpp の 3、5、17、257 の判定を、ノルムを部分体へ順に落とす形 (掛け算 3 回と Frobenius 3 回) から、Library の log の
// 表 IL16 を 1 回引く形に替えた版。m = n^(1+2^16) (GF(2^16) へのノルム) の log16 (IL16 の下位 16 bit) を p が割り切れば、p 成分は 1。
// 65537、641、6700417 は lib-frobenius と同じ。IL16 (256 KB) はコンパイル時に作る。Library の内部 (gf2p64_internal) の名前を使うので、
// Library の内部が変わると組めなくなることがある。
#include "mylib/algebra/GF2p64.hpp"
#include <vector>
using namespace std;
using u64= unsigned long long;
namespace gf2_64_ord_lib_frob_il16 {
using namespace gf2p64_internal;
inline u64 ord(u64 a) {
 const u64 n= mul(a, F32(a)), fn= F16(n), m= mul(n, fn);
 const u32 la= Log<0>::IL16.t[u16(m)] & 65535;
 const u64 c= mul(mul(a, F7(a)), sq(F8(a)));
 const u64 x3= mul(a, sq(a)), x15= mul(x3, F2(x3)), x51= mul(x3, F4(x3));
 const u64 d= mul(mul(sq(F16(x51)), sq(sq(F8(x15)))), mul(F7(x3), a));
 return u64(la % 3 ? 3 : 1) * (la % 5 ? 5 : 1) * (la % 17 ? 17 : 1) * (la % 257 ? 257 : 1) * (fn != n ? 65537 : 1) * (F32(d) != d ? 641 : 1) * (F32(c) != c ? 6700417 : 1);
}
}  // namespace gf2_64_ord_lib_frob_il16
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2_64_ord_lib_frob_il16::ord(as[i]);
 return ans;
}
