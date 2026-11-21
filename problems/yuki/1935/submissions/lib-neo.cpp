// lib.cpp を NeoLibrary の FunctionalGraph で書いたもの。今の Library の Period は状態 (4 つの水の量) をライブラリの中で番号付け
// したが、NeoLibrary は int の関数だけを受けるので、始めの状態から f をたどって訪れた状態にハッシュ表で番号を振り、to を作って
// 渡す (同じ状態に戻ったところで止める)。
#include <array>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "neo/base/ds/tree/FunctionalGraph.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Dat= array<int, 4>;
 Dat V;
 long long N;
 cin >> V[0] >> V[1] >> V[2] >> V[3] >> N;
 auto f= [&](int& a, int& b, int v) {
  int x= min(a, v - b);
  a-= x, b+= x;
 };
 auto g= [&](Dat a) {
  f(a[0], a[1], V[1]);
  f(a[1], a[2], V[2]);
  f(a[2], a[3], V[3]);
  f(a[3], a[0], V[0]);
  return a;
 };
 auto key= [](const Dat& a) { return uint64_t(a[0]) << 48 | uint64_t(a[1]) << 32 | uint64_t(a[2]) << 16 | uint64_t(a[3]); };
 Dat init= {V[0], 0, 0, 0};
 vector<Dat> st{init};
 vector<int> to;
 unordered_map<uint64_t, int> id{{key(init), 0}};
 for(int i= 0;; ++i) {
  const Dat y= g(st[i]);
  auto [it, fresh]= id.emplace(key(y), int(st.size()));
  to.push_back(it->second);
  if(!fresh) break;
  st.push_back(y);
 }
 auto [q, r]= div(N, 4LL);
 auto v= st[FunctionalGraph(to).jump(0, q)];
 for(int i= 0; i < r; ++i) f(v[i], v[i + 1], V[i + 1]);
 cout << v[0] << ' ' << v[1] << ' ' << v[2] << ' ' << v[3] << '\n';
 return 0;
}
