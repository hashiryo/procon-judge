// 3 と 5 だけからなり 15 の倍数の数を、NeoLibrary の DFA (neo/counting/DFA.hpp) の ranker で長さごとに数え、kth で作る。
#include <iostream>
#include <vector>
#include <utility>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 unsigned long long N;
 cin >> N;
 using state_t= pair<int, bool>;  // (桁和 mod 3, 末尾が 5)
 DFA<int> d({3, 5}, state_t{0, false}, [](state_t s, int c) { return state_t{(s.first + c) % 3, c == 5}; }, [](state_t s) { return s.first == 0 && s.second; });
 auto R= d.ranker<unsigned long long>(60);
 for(int len= 1;; ++len) {
  const unsigned long long c= R.count(len);
  if(N <= c) {
   for(int x: R.kth(N - 1, len)) cout << x;
   cout << '\n';
   return 0;
  }
  N-= c;
 }
}
