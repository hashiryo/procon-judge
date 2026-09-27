// 期待出力を作る参照実装。submissions/lib-enum-quo.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// O(√N)
#include <iostream>
#include <vector>
#include <algorithm>
#include <tuple>
#include <cmath>
#include <cstdint>
// (q,l,r) : i in (l,r], ⌊N/i⌋ = q
std::vector<std::tuple<uint64_t, uint64_t, uint64_t>> enumerate_quotients(uint64_t N) {
 uint64_t sq= std::sqrt(N), prev= N, x;
 std::vector<std::tuple<uint64_t, uint64_t, uint64_t>> ret;
 for (int q= 1, n= (sq * sq + sq <= N ? sq : sq - 1); q <= n; ++q) ret.emplace_back(q, x= double(N) / (q + 1), prev), prev= x;
 for (int l= sq; l >= 1; --l) ret.emplace_back(double(N) / l, l - 1, l);
 return ret;
}
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 long long ans= 0;
 for(auto [q, l, r]: enumerate_quotients(N - 1)) ans+= (r - l) * q;
 cout << ans << '\n';
 return 0;
}
