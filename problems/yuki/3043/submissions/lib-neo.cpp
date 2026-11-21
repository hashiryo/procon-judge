// 括弧列の深さを状態にした NeoLibrary の DFA (neo/algo/count/automata/DFA.hpp) で、値を (個数, f の和) の組にし、) を読んだら移った先の
// 深さを足す。足す量が状態で決まるので、dp の f に遷移の前後の状態 (Step) を受けさせる。
#include <iostream>
#include <vector>
#include <utility>
#include "neo/algo/count/automata/DFA.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
using namespace std;
signed main() {
 using Z= ZMod<998244353>;
 int N;
 cin >> N;
 DFA<char> d({'(', ')'}, 0, [&](int s, char c) { return c == '(' ? (s + 1 > N / 2 ? -1 : s + 1) : s - 1; }, [](int s) { return s == 0; }, -1);
 using V= pair<Z, Z>;
 auto op= [](const V& a, const V& b) { return V{a.first + b.first, a.second + b.second}; };
 auto f= [](const V& v, char c, int, const DFA<char>::Step& t) { return c == ')' ? V{v.first, v.second + v.first * Z(t.to.get<int>())} : v; };
 cout << d.dp(N, op, V{0, 0}, f, V{1, 0}).second << '\n';
 return 0;
}
