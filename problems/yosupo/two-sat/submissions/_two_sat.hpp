#pragma once
// 2-SAT を、含意グラフの強連結成分分解で解く (yosupo-scc の核を写した _scc_rec.hpp を使う)。変数 i (1 から n) の肯定を
// 頂点 2 (i - 1)、否定を 2 (i - 1) + 1 とし、節 a ∨ b を辺 ¬a → b と ¬b → a にする。辺の列は作らず、節の列をそのまま辺の列に
// 見せて核に渡す。
// 成分の番号はトポロジカル順なので、x と ¬x が同じ成分なら充足不能で、そうでなければ x が真になるのは comp(x) > comp(¬x)
// のとき (x から ¬x へ道があれば x は偽でなければならず、そのとき comp(x) <= comp(¬x))。
#include "_scc_rec.hpp"
namespace two_sat {
// 節の列を、含意グラフの辺の列に見せる。辺 2 j と 2 j + 1 は、節 j = (a, b) の ¬a → b と ¬b → a。
struct ImplicationEdges {
 const vector<array<int, 2>>& cs;
 // リテラル l を頂点にする。負なら否定で、下位 1 bit が立つ。
 static int vertex(int l) { return 2 * (l < 0 ? -l : l) - 2 + (l < 0); }
 size_t size() const { return 2 * cs.size(); }
 array<int, 2> operator[](size_t i) const {
  const auto& c= cs[i >> 1];
  const int t= int(i & 1);
  return {vertex(c[t]) ^ 1, vertex(c[t ^ 1])};
 }
 struct It {
  const ImplicationEdges* p;
  size_t i;
  array<int, 2> operator*() const { return (*p)[i]; }
  It& operator++() { return ++i, *this; }
  bool operator!=(const It& o) const { return i != o.i; }
 };
 It begin() const { return {this, 0}; }
 It end() const { return {this, size()}; }
};
template <class A> struct Solver {
 int n;
 ImplicationEdges es;
 A a;
 bool sat= true;
 Solver(int n, const vector<array<int, 2>>& cs): n(n), es{cs} {}
 void run() {
  a.run(2 * n, es);
  for(int i= 0; i < n; ++i)
   if(a.comp(2 * i) == a.comp(2 * i + 1)) {
    sat= false;
    break;
   }
 }
 bool satisfiable() const { return sat; }
 bool value(int i) const { return a.comp(2 * i - 2) > a.comp(2 * i - 1); }
};
}
