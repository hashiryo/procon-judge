// LOJ 2062 「HAOI2016」地图
// 仙人掌を圆方树 (block-cut tree) にする。クエリ x では、1 から x への単純パスが通る
// 街路がすべて塞がれる。仙人掌では x の親ブロックの辺がすべて塞がり、x の子ブロックの
// 辺はどれも 1 から x への単純パスに現れないので、x から行ける頂点はブロック木の x の
// 部分木の頂点ちょうどになる (平行辺の 2 角形も 2 頂点のブロックとして同じ扱い)。
// 木の頂点に a_v を持たせ、区間 [dfn[x], dfn[x]+sz[x]) を Mo's algorithm で動かし、
// 値の sqrt 分解 (奇数個の値と正の偶数個の値の数) で y 以下の答えを出す。
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
using namespace std;

namespace fastio {
constexpr int BUFSIZE = 1 << 22;
char buf[BUFSIZE];
int len = 0, pos = 0;
inline int getChar() {
 if(pos == len) {
  len = (int)fread(buf, 1, BUFSIZE, stdin), pos = 0;
  if(len <= 0) return -1;
 }
 return buf[pos++];
}
inline int readInt() {
 int c= getChar();
 while(c >= 0 && c <= 32) c= getChar();
 int x= 0;
 while(c > 32) x= x * 10 + (c - '0'), c= getChar();
 return x;
}
}  // namespace fastio

namespace fastout {
char buf[1 << 22];
int len= 0;
inline void flush() { fwrite(buf, 1, len, stdout), len= 0; }
inline void putChar(char c) {
 if(len == (int)sizeof(buf)) flush();
 buf[len++]= c;
}
inline void writeInt(int x) {
 char tmp[12];
 int t= 0;
 if(x == 0) tmp[t++]= '0';
 else
  while(x) tmp[t++]= (char)('0' + x % 10), x/= 10;
 while(t) putChar(tmp[--t]);
 putChar('\n');
}
}  // namespace fastout

signed main() {
 using fastio::readInt;
 int n= readInt(), m= readInt();
 vector<int> a(n);
 for(int i= 0; i < n; ++i) a[i]= readInt();

 // グラフを CSR で持つ。辺の id は親辺の除外 (平行辺を 2 角形として拾う) に使う。
 vector<int> deg(n, 0), eu(m), ev(m);
 for(int i= 0; i < m; ++i) {
  int u= readInt() - 1, v= readInt() - 1;
  eu[i]= u, ev[i]= v, ++deg[u], ++deg[v];
 }
 vector<int> start(n + 1, 0), adjTo(2 * m), adjEid(2 * m), fillPos(n);
 for(int i= 0; i < n; ++i) start[i + 1]= start[i] + deg[i];
 for(int i= 0; i < n; ++i) fillPos[i]= start[i];
 for(int i= 0; i < m; ++i) {
  const int u= eu[i], v= ev[i];
  adjTo[fillPos[u]]= v, adjEid[fillPos[u]++]= i;
  adjTo[fillPos[v]]= u, adjEid[fillPos[v]++]= i;
 }

 // 反復 DFS で親と深さを取り、祖先へ戻る辺 (後退辺) を集める。
 vector<int> par(n, -1), dep(n, 0), parentEdge(n, -1), adjIt(start.begin(), start.begin() + n), dfsStack;
 vector<char> visited(n, 0);
 vector<int> backU, backV;
 dfsStack.reserve(n), dfsStack.push_back(0), visited[0]= 1;
 while(!dfsStack.empty()) {
  const int v= dfsStack.back();
  if(adjIt[v] < start[v + 1]) {
   const int idx= adjIt[v]++, w= adjTo[idx], e= adjEid[idx];
   if(e == parentEdge[v]) continue;
   if(!visited[w]) {
    visited[w]= 1, par[w]= v, dep[w]= dep[v] + 1, parentEdge[w]= e;
    dfsStack.push_back(w);
   } else if(dep[w] < dep[v]) {
    backU.push_back(v), backV.push_back(w);
   }
  } else {
   dfsStack.pop_back();
  }
 }

 // 各後退辺が作る基本閉路を 1 つのブロック (角ノード) とする。仙人掌なので木の辺は
 // 高々 1 つの閉路にしか入らず、この走査の合計は O(n + m)。
 const int cycleCount= (int)backU.size();
 vector<int> upCycle(n, -1), cycleTop(cycleCount), cycleBegin(cycleCount + 1), cycleVerts;
 cycleVerts.reserve(n + cycleCount);
 for(int c= 0; c < cycleCount; ++c) {
  const int bottom= backU[c], top= backV[c];
  cycleBegin[c]= (int)cycleVerts.size();
  int x= bottom;
  while(x != top) upCycle[x]= c, cycleVerts.push_back(x), x= par[x];
  cycleVerts.push_back(top);
  reverse(cycleVerts.begin() + cycleBegin[c], cycleVerts.end());  // 上から下へ
  cycleTop[c]= top;
 }
 cycleBegin[cycleCount]= (int)cycleVerts.size();

 // ブロック木の子を、橋 (頂点が直接の子) と閉路 (角ノードをはさんだ子) に分けて持つ。
 vector<int> firstCycle(n, -1), nextCycle(cycleCount, -1);
 for(int c= 0; c < cycleCount; ++c) {
  const int t= cycleTop[c];
  nextCycle[c]= firstCycle[t], firstCycle[t]= c;
 }
 vector<int> firstBridge(n, -1), nextBridge(n, -1);
 for(int v= 1; v < n; ++v)
  if(upCycle[v] == -1) {
   const int p= par[v];
   nextBridge[v]= firstBridge[p], firstBridge[p]= v;
  }

 // ブロック木の DFS。角ノードは頂点を数えないので、上から見て角ノードの子の頂点を
 // そのまま頂点の子として展開する。部分木の頂点は dfn の連続区間になる。
 vector<int> dfn(n), sz(n), byDfn(n);
 int vertexCount= 0;
 vector<int> stack;
 stack.reserve(4 * n + 4 * cycleCount), stack.push_back(0);
 while(!stack.empty()) {
  int code= stack.back();
  stack.pop_back();
  const int v= code >> 1;
  if((code & 1) == 0) {
   dfn[v]= vertexCount, byDfn[vertexCount]= v, ++vertexCount;
   stack.push_back(code | 1);
   for(int c= firstCycle[v]; c != -1; c= nextCycle[c])
    for(int j= cycleBegin[c + 1] - 1; j > cycleBegin[c]; --j) stack.push_back(cycleVerts[j] << 1);
   for(int w= firstBridge[v]; w != -1; w= nextBridge[w]) stack.push_back(w << 1);
  } else {
   sz[v]= vertexCount - dfn[v];
  }
 }

 const int Q= readInt();
 vector<int> queryType(Q), queryX(Q), queryK(Q), ans(Q);
 {
  vector<int> queryY(Q);
  for(int i= 0; i < Q; ++i) queryType[i]= readInt(), queryX[i]= readInt() - 1, queryY[i]= readInt();
  // 値の座標圧縮。y 以下を、rank の接頭辞の長さ k に直す。
  vector<int> values(a);
  sort(values.begin(), values.end());
  values.erase(unique(values.begin(), values.end()), values.end());
  const int valueCount= (int)values.size();
  vector<int> rankOf(n);
  for(int v= 0; v < n; ++v) rankOf[v]= (int)(lower_bound(values.begin(), values.end(), a[v]) - values.begin());
  for(int i= 0; i < Q; ++i) queryK[i]= (int)(upper_bound(values.begin(), values.end(), queryY[i]) - values.begin());

  Mo mo;
  for(int i= 0; i < Q; ++i) mo.query(dfn[queryX[i]], dfn[queryX[i]] + sz[queryX[i]]);

  // 値の sqrt 分解。各ブロックに、奇数個の値と正の偶数個の値の数を保つ。
  int blockSize= (int)std::sqrt((double)valueCount);
  if(blockSize < 1) blockSize= 1;
  const int blockCount= (valueCount + blockSize - 1) / blockSize;
  vector<int> count(valueCount, 0), blockOf(valueCount), odd(blockCount, 0), even(blockCount, 0);
  for(int c= 0; c < valueCount; ++c) blockOf[c]= c / blockSize;

  auto add= [&](int position) {
   const int c= rankOf[byDfn[position]], b= blockOf[c], old= count[c];
   ++count[c];
   if(old & 1) --odd[b], ++even[b];
   else {
    ++odd[b];
    if(old) --even[b];
   }
  };
  auto erase= [&](int position) {
   const int c= rankOf[byDfn[position]], b= blockOf[c], old= count[c];
   --count[c];
   if(old & 1) {
    --odd[b];
    if(old > 1) ++even[b];
   } else {
    ++odd[b], --even[b];
   }
  };
  auto out= [&](int id) {
   const int k= queryK[id], b= k / blockSize;
   int oddSum= 0, evenSum= 0;
   for(int j= 0; j < b; ++j) oddSum+= odd[j], evenSum+= even[j];
   for(int c= b * blockSize; c < k; ++c) {
    if(count[c] & 1) ++oddSum;
    else if(count[c]) ++evenSum;
   }
   ans[id]= queryType[id] ? oddSum : evenSum;
  };
  mo.run(add, erase, out);
 }
 for(int i= 0; i < Q; ++i) fastout::writeInt(ans[i]);
 fastout::flush();
}
