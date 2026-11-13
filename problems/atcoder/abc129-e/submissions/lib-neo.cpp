// lib.cpp と同じ数え上げを NeoLibrary の DFA (neo/counting/DFA.hpp) で行う。a + b = a xor b が L 以下であることは、2 進の
// leq_digits を、記号 (a の桁, b の桁) を a xor b の桁と読む map_symbols で写して表す。
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include "neo/counting/DFA.hpp"
#include "neo/algebra/ZMod.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<int(1e9 + 7)>;
 string L;
 cin >> L;
 int n= L.length();
 vector<int> d(n);
 for(int i= 0; i < n; ++i) d[i]= L[i] - '0';
 using symbol_t= array<bool, 2>;
 vector<symbol_t> alp= {{0, 0}, {0, 1}, {1, 0}};
 auto dfa= leq_digits(d, 2).map_symbols(alp, [](symbol_t c) { return c[0] ^ c[1]; });
 cout << dfa.count<Mint>(n) << '\n';
 return 0;
}
