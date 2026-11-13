// lib.cpp と同じ DP を NeoLibrary の DFA (neo/counting/DFA.hpp) で回す。N 以下の DFA は、下の桁から読む leq_digits_lsb にする。
#include <iostream>
#include <vector>
#include <algorithm>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 long long N;
 cin >> N;
 auto dfa= leq_digits_lsb(N, 10);
 auto op= [](long long l, long long r) { return max(l, r); };
 auto f= [](long long x, int c, int) { return x + c; };
 cout << dfa.dp(60, op, 0ll, f, 0ll) << '\n';
 return 0;
}
