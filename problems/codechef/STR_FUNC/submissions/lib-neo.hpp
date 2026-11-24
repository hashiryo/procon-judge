#pragma once
// NeoLibrary の sps::online_subset_zeta (neo/base/core/set_fps/relaxed.hpp) で解く。値は (G, F^2) の組で、phi が受け取る和は
// (真の部分集合の G の和、真の部分集合の F^2 の和)。N が 2 冪でないときは、先は 0 の組を入れる。
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/set_fps/relaxed.hpp"
struct Solver {
 using Z= ZMod<1000000007>;
 struct Pair {
  Z g, f2;
  Pair& operator+=(const Pair& o) {
   g+= o.g, f2+= o.f2;
   return *this;
  }
 };
 vector<int> a;
 u32 ans= 0;
 explicit Solver(const vector<int>& a_): a(a_) {}
 void run() {
  const int n= int(a.size());
  int m= 0;
  while((1 << m) < n) ++m;
  Z total= Z();
  sps::online_subset_zeta<Pair>(m, [&](int i, Pair& x) {
   if(i >= n) {
    x= Pair{Z(), Z()};
    return;
   }
   const Z F= Z(a[i]) * Z(a[i]) + x.g * x.g, F2= F * F, G= x.f2 + F2;
   total+= Z(i) * F * G;
   x= Pair{G, F2};
  });
  ans= total.val();
 }
 u32 answer() const { return ans; }
};
