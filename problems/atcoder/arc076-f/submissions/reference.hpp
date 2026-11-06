#pragma once
// 想定解の貪欲 (期待出力を作る参照実装)。人を L の小さい順に見て、左の椅子 1..L に座る人の R を最小ヒープに持ち、人数が
// L を超えたら R の最も小さい人を右へ回す。右へ回した人は R の大きい順に、空いた椅子のうち最も右のものへ座らせる。
// ライブラリも _shared も include しない (期待出力のキャッシュの鍵はこのファイルの中身だけで決まるため)。
#include <algorithm>
#include <functional>
#include <queue>
#include <vector>
struct Solver {
 int n, m;
 const vector<array<int, 2>>& lr;
 int ans= 0;
 Solver(int n, int m, const vector<array<int, 2>>& lr): n(n), m(m), lr(lr) {}
 void run() {
  vector<array<int, 2>> a(lr.begin(), lr.end());
  sort(a.begin(), a.end());
  priority_queue<int, vector<int>, greater<int>> left;
  vector<int> rest;
  for(auto [l, r]: a) {
   left.push(r);
   if((int)left.size() > l) rest.push_back(left.top()), left.pop();
  }
  const int used= left.size();
  sort(rest.rbegin(), rest.rend());
  int chair= m, sat= 0;
  for(int r: rest)
   if(chair > used && chair >= r) --chair, ++sat;
  ans= (int)rest.size() - sat;
 }
 int answer() const { return ans; }
};
