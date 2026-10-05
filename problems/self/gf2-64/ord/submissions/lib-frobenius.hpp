// frobenius.hpp と同じ手順を、Library の GF2p64 の中身 (名前空間の LinMap の F2、F4、F7、F8、F16、F32 と mul、sq) で書いた版。
// Library に ord() を入れるときの形。Library の名前空間には F9 と F10 が無いので、F9 = sq∘F8、F10 = sq∘sq∘F8 とする。
// 表は名前空間の LinMap だけで、log の表 (IL16 など) は作らない。Library の内部 (gf2p64_internal) の名前を使うので、
// Library の内部が変わると組めなくなることがある。
#include "mylib/algebra/GF2p64.hpp"
#include <vector>
using namespace std;
using u64= unsigned long long;
namespace gf2_64_ord_lib_frob {
using namespace gf2p64_internal;
inline u64 ord(u64 a) {
 const u64 n= mul(a, F32(a)), fn= F16(n), m= mul(n, fn);
 const u64 f8= F8(m), t= mul(m, f8), f4= F4(t), u= mul(t, f4), f2= F2(u), v= mul(u, f2);
 const u64 c= mul(mul(a, F7(a)), sq(F8(a)));
 const u64 x3= mul(a, sq(a)), x15= mul(x3, F2(x3)), x51= mul(x3, F4(x3));
 const u64 d= mul(mul(sq(F16(x51)), sq(sq(F8(x15)))), mul(F7(x3), a));
 return u64(v != 1 ? 3 : 1) * (f2 != u ? 5 : 1) * (f4 != t ? 17 : 1) * (f8 != m ? 257 : 1) * (fn != n ? 65537 : 1) * (F32(d) != d ? 641 : 1) * (F32(c) != c ? 6700417 : 1);
}
}  // namespace gf2_64_ord_lib_frob
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2_64_ord_lib_frob::ord(as[i]);
 return ans;
}
