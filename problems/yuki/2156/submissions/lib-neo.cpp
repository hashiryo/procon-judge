// ぞい文字列の DFA を NeoLibrary の DFA (neo/algo/count/automata/DFA.hpp) で組み、長さ N (≤ 10^18) の数を count_large で求める。
#include <iostream>
#include "neo/algo/count/automata/DFA.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
using namespace std;
signed main() {
 unsigned long long N;
 cin >> N;
 // 記号 0 が「ぞ」、1 が「い」。状態 0: 空、1: 末尾がぞで、いがまだ無い、2: 末尾がぞで、いがある、3: 末尾がい
 const int tr[4][2]= {{1, -1}, {1, 3}, {2, 3}, {2, -1}};
 DFA<int> d({0, 1}, 0, [&](int s, int c) { return s < 0 ? -1 : tr[s][c]; }, [](int s) { return s == 2 || s == 3; }, -1);
 cout << d.count_large<ZMod<998244353>>(N) << '\n';
 return 0;
}
