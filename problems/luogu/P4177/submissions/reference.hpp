#pragma once
// 素朴な Dinic で最大重み閉包の最小カットを求める (期待出力を作る参照実装)。s → 仕事に収入、仕事 → 機械に借りる費用、
// 機械 → t に買う費用の容量を張り、収入の和から最大流を引く。弧は CSR に並べ、段の BFS と、今の弧を進める DFS を繰り返す。
// ライブラリも _shared も include しない (期待出力のキャッシュの鍵はこのファイルの中身だけで決まるため)。
#include <algorithm>
#include <array>
#include <vector>
struct Solver {
 int n, m;
 const vector<int>&x, &off;
 const vector<array<int, 2>>& proc;
 const vector<int>& y;
 long long ans= 0;
 Solver(int n, int m, const vector<int>& x, const vector<int>& off, const vector<array<int, 2>>& proc, const vector<int>& y): n(n), m(m), x(x), off(off), proc(proc), y(y) {}
 void run() {
  const int V= n + m + 2, s= n + m, t= s + 1;
  vector<array<int, 3>> es;  // {from, to, cap}
  es.reserve(n + proc.size() + m);
  long long sum= 0;
  for(int i= 0; i < n; ++i) {
   es.push_back({s, i, x[i]}), sum+= x[i];
   for(int k= off[i]; k < off[i + 1]; ++k) es.push_back({i, n + proc[k][0], proc[k][1]});
  }
  for(int j= 0; j < m; ++j) es.push_back({n + j, t, y[j]});
  vector<int> st(V + 1);
  for(auto& [u, v, c]: es) ++st[u + 1], ++st[v + 1];
  for(int i= 0; i < V; ++i) st[i + 1]+= st[i];
  const int E= 2 * es.size();
  vector<int> to(E), rev(E);
  vector<long long> cap(E);
  {
   vector<int> p(st.begin(), st.end() - 1);
   for(auto& [u, v, c]: es) {
    const int a= p[u]++, b= p[v]++;
    to[a]= v, rev[a]= b, cap[a]= c;
    to[b]= u, rev[b]= a, cap[b]= 0;
   }
  }
  vector<int> lv(V), it(V), q(V);
  auto bfs= [&]() {
   fill(lv.begin(), lv.end(), -1);
   int qh= 0, qt= 0;
   lv[s]= 0, q[qt++]= s;
   while(qh < qt) {
    const int u= q[qh++];
    for(int a= st[u]; a < st[u + 1]; ++a)
     if(cap[a] > 0 && lv[to[a]] < 0) lv[to[a]]= lv[u] + 1, q[qt++]= to[a];
   }
   return lv[t] >= 0;
  };
  auto dfs= [&](auto&& self, int u, long long f) -> long long {
   if(u == t) return f;
   for(int& a= it[u]; a < st[u + 1]; ++a) {
    const int v= to[a];
    if(cap[a] > 0 && lv[v] == lv[u] + 1) {
     const long long g= self(self, v, min(f, cap[a]));
     if(g > 0) {
      cap[a]-= g, cap[rev[a]]+= g;
      return g;
     }
    }
   }
   return 0;
  };
  long long flow= 0;
  while(bfs()) {
   copy(st.begin(), st.end() - 1, it.begin());
   for(long long f; (f= dfs(dfs, s, 1LL << 60)) > 0;) flow+= f;
  }
  ans= sum - flow;
 }
 long long answer() const { return ans; }
};
