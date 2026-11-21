// lib.cpp の Graph と Rerooting を NeoLibrary の Rerooting (木は辺の列で渡す) に替えたもの。約数ごとの値の列は今の Library の
// ArrayOnDivisors と ModInt のまま (Rerooting に大きい型を渡す例になる)。
#include <array>
#include <iostream>
#include <numeric>
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/number_theory/ArrayOnDivisors.hpp"
#include "neo/base/engine/tree/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ModInt<998244353>;
 int n;
 cin >> n;
 vector<int> A(n);
 Mint sum= 0;
 for(int& a: A) cin >> a, sum+= a;
 vector<array<int, 2>> es(n - 1);
 for(auto& [a, b]: es) cin >> a >> b, --a, --b;
 using T= ArrayOnDivisors<int, pair<int, Mint>>;
 auto put_edge= [&](int v, int, const T& dat) {
  T ret(A[v]);
  for(auto&& [d, a]: dat) {
   auto& [l0, l1]= ret[gcd(d, A[v])];
   auto [r0, r1]= a;
   l1+= r1;
   l0+= r0;
  }
  return ret;
 };
 auto op= [&](const T& l, const T& r) {
  if(l.size() == 0) return r;
  if(r.size() == 0) return l;
  auto ret= l;
  for(auto& [d, a]: ret) {
   auto& [l0, l1]= a;
   auto [r0, r1]= r[d];
   l1+= r1;
   l0+= r0;
  }
  return ret;
 };
 auto put_vertex= [&](int v, T dat) {
  if(dat.size() == 0) dat= T(A[v]);
  dat[A[v]].first+= 1;
  for(auto& [d, x]: dat) x.second+= x.first;
  return dat;
 };
 Mint ans= 0;
 for(const auto& dat: Rerooting<T>(n, es, put_edge, op, T(), put_vertex))
  for(auto&& [d, x]: dat) ans+= x.second * d;
 ans-= sum, ans/= 2;
 cout << ans << '\n';
 return 0;
}
