// cf-343-e の提出で共有する、入力の読み込みと、Gomory-Hu 木から答えを組む部分。
//
// 隣り合う駅の組の最大流の和の最大は、Gomory-Hu 木の辺の重みの和に等しい。順列は、木の中で重みが最小の辺を切り、切った
// 片側を並べてからもう片側を並べる、を再帰で繰り返して作る。継ぎ目の組 (片側の最後ともう片側の最初) の最大流は、木の
// 道がちょうど切った辺を通り、その成分の中ではその辺が最小なので、切った辺の重みに等しい。そのため和は木の辺の重みの和に
// なる。
#include <array>
#include <cstdio>
#include <string>
#include <vector>
struct CF343E {
 int n= 0, m= 0;
 std::vector<std::array<int, 3>> e;  // {a, b, c}。駅は 0 から n - 1
 bool read() {
  if(scanf("%d %d", &n, &m) != 2) return false;
  e.resize(m);
  for(auto& [a, b, c]: e) {
   if(scanf("%d %d %d", &a, &b, &c) != 3) return false;
   --a, --b;
  }
  return true;
 }
 // 根付き木 (parent[root] < 0) と、辺 (v, parent[v]) の重み w[v] から、給料の最大と順列を出す。
 void print(const std::vector<int>& parent, const std::vector<long long>& w) const {
  std::vector<std::vector<std::pair<int, long long>>> adj(n);
  long long sum= 0;
  for(int v= 0; v < n; ++v)
   if(parent[v] >= 0) adj[v].push_back({parent[v], w[v]}), adj[parent[v]].push_back({v, w[v]}), sum+= w[v];
  std::vector<char> cut(n, 0);  // cut[v]: 辺 (v, parent[v]) を切った
  std::vector<int> order, comp, st;
  std::vector<char> seen(n, 0);
  // v を含む成分 (切っていない辺でつながる頂点) を comp に集める。
  auto collect= [&](int v) {
   comp.clear(), st.assign(1, v), seen[v]= 1;
   while(!st.empty()) {
    const int x= st.back();
    st.pop_back(), comp.push_back(x);
    for(auto [y, ww]: adj[x])
     if(!seen[y] && !cut[parent[x] == y ? x : y]) seen[y]= 1, st.push_back(y);
   }
   for(int x: comp) seen[x]= 0;
  };
  // 頂点 v を含む成分を並べる。
  auto rec= [&](auto&& self, int v) -> void {
   collect(v);
   int arg= -1;  // 成分の中で重みが最小の辺 (子の頂点で表す)
   for(int x: comp)
    if(parent[x] >= 0 && !cut[x]) {
     bool inside= false;
     for(int y: comp) inside|= y == parent[x];
     if(inside && (arg < 0 || w[x] < w[arg])) arg= x;
    }
   if(arg < 0) return order.push_back(v);
   cut[arg]= 1;
   const int a= arg, b= parent[arg];
   self(self, a), self(self, b);
  };
  rec(rec, 0);
  std::string out= std::to_string(sum) + "\n";
  for(int i= 0; i < n; ++i) out+= std::to_string(order[i] + 1), out+= i + 1 < n ? ' ' : '\n';
  fputs(out.c_str(), stdout);
 }
};
