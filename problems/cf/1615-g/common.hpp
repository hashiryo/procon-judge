#pragma once
// cf-1615-g の提出で共通の帰着と出力。マッチングを求めるところだけを提出ごとに替える。
//
// 0 の区間 [l, r] (長さ L、左の端の値 x、右の端の値 y。配列の端なら 0) の使い方は、次の 3 通りで足りる。
// - 0 を 2 つずつ、配列に出てこない新しい値の組 (f f) で埋める。floor(L / 2) 個の値が数えられる。新しい値の数は足りる
//   (配列に出てこない値の数は 0 の数以上ある)。
// - L が奇数なら、端の値の 1 つ (x か y) を区間の端の 0 に写して、その値も数える。残りは新しい値の組で埋める。
// - L が偶数なら、両端の値を写して x と y を数え、間の L - 2 個を新しい値の組で埋める (L / 2 - 1 + 2)。片方だけ写すのは、
//   新しい値の組を 1 つ減らして値を 1 つ増やすだけなので、得にならない。
// どの値も 1 回しか数えないので、すでに 0 でない隣り合う組のある値を除いた値を頂点にする。奇数の区間は 1 頂点 (x と y への辺)、
// 偶数の区間は 2 頂点 s1, s2 (s1 - s2、s1 - x、s2 - y の辺) にした一般グラフの最大マッチングの大きさを M とすると、
// 答えは (すでに組のある値の数) + Σ floor(L / 2) + M - (偶数の区間の数) になる。偶数の区間は、s1 と s2 が両方とも値と組んだ
// ときだけ 1 増える。
#include <array>
#include <cstdio>
#include <vector>
namespace cf1615g {
struct Seg {
 int l, r, x, y, s;  // 区間 [l, r]、両端の値 (無ければ 0)、頂点の番号 (偶数の区間は s と s + 1)
};
struct Reduction {
 int n= 0, nv= 0;
 std::vector<int> a;
 std::vector<Seg> segs;
 std::vector<std::array<int, 2>> edges;  // 頂点の数は nv。値 v (1 以上 min(n, 600) 以下) は頂点 v - 1
};
inline Reduction read_and_build() {
 Reduction R;
 int n= 0;
 if(scanf("%d", &n) != 1) return R;
 R.n= n, R.a.resize(n);
 for(int& x: R.a)
  if(scanf("%d", &x) != 1) x= 0;
 const int V= n < 600 ? n : 600;
 std::vector<char> paired(V + 1, 0);
 for(int i= 0; i + 1 < n; ++i)
  if(R.a[i] && R.a[i] == R.a[i + 1]) paired[R.a[i]]= 1;
 auto usable= [&](int v) { return v > 0 && !paired[v]; };
 int nv= V;
 for(int i= 0; i < n;) {
  if(R.a[i]) {
   ++i;
   continue;
  }
  int j= i;
  while(j < n && R.a[j] == 0) ++j;
  const int x= i > 0 ? R.a[i - 1] : 0, y= j < n ? R.a[j] : 0;
  R.segs.push_back({i, j - 1, x, y, nv});
  if((j - i) & 1) {
   if(usable(x)) R.edges.push_back({nv, x - 1});
   if(usable(y) && y != x) R.edges.push_back({nv, y - 1});
   nv+= 1;
  } else {
   R.edges.push_back({nv, nv + 1});
   if(usable(x)) R.edges.push_back({nv, x - 1});
   if(usable(y)) R.edges.push_back({nv + 1, y - 1});
   nv+= 2;
  }
  i= j;
 }
 R.nv= nv;
 return R;
}
// マッチングの相手の配列 (頂点ごと、いなければ -1) から答えの配列を作って出す。
inline void restore_and_print(const Reduction& R, const std::vector<int>& mate) {
 const int n= R.n;
 std::vector<int> b= R.a;
 // 配列に出てこない値を、新しい値として小さい順に使う。
 std::vector<char> used(n + 1, 0);
 for(int x: R.a) used[x]= 1;
 int f= 1;
 auto fill_pairs= [&](int l, int r) {  // [l, r] (長さは偶数) を新しい値の組で埋める
  for(int i= l; i < r; i+= 2) {
   while(used[f]) ++f;
   b[i]= b[i + 1]= f++;
  }
 };
 for(const Seg& sg: R.segs) {
  const int s= sg.s;
  if((sg.r - sg.l + 1) & 1) {
   const int m= mate[s];
   if(m >= 0 && sg.x > 0 && m == sg.x - 1) b[sg.l]= sg.x, fill_pairs(sg.l + 1, sg.r);
   else if(m >= 0 && sg.y > 0 && m == sg.y - 1) b[sg.r]= sg.y, fill_pairs(sg.l, sg.r - 1);
   else fill_pairs(sg.l, sg.r - 1);
  } else if(sg.x > 0 && sg.y > 0 && mate[s] == sg.x - 1 && mate[s + 1] == sg.y - 1) {
   b[sg.l]= sg.x, b[sg.r]= sg.y, fill_pairs(sg.l + 1, sg.r - 1);
  } else fill_pairs(sg.l, sg.r);
 }
 for(int i= 0; i < n; ++i)
  if(b[i] == 0) b[i]= i > 0 ? b[i - 1] : 1;  // 余った 0 は何で埋めてもよい
 std::vector<char> buf;
 buf.reserve((size_t)n * 7 + 2);
 char tmp[12];
 for(int i= 0; i < n; ++i) {
  int len= 0, x= b[i];
  do tmp[len++]= char('0' + x % 10), x/= 10;
  while(x);
  while(len) buf.push_back(tmp[--len]);
  buf.push_back(i + 1 == n ? '\n' : ' ');
 }
 fwrite(buf.data(), 1, buf.size(), stdout);
}
}  // namespace cf1615g
