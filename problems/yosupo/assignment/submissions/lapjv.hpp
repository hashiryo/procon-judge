#pragma once
// Jonker と Volgenant の LAPJV。列のポテンシャル v だけを持ち、4 段で解く。
// 1. 列ごとに最小の行を探して v に入れ、その行が空いていれば割り当てる (column reduction)。
// 2. ちょうど 1 列の最小だった行について、その列の v を、行の 2 番目に小さい縮約費用の分だけ下げる (reduction transfer)。
// 3. 空いている行ごとに、縮約費用の最小と 2 番目を探して最小の列を奪い、v を 2 番目との差だけ下げる。奪われた行は、
//    差があればその場で続け、無ければ次の回へ回す。これを 2 回する (augmenting row reduction)。
// 4. 残った空いている行ごとに、列への距離で Dijkstra をして最短の増加路を流す。同じ最小の距離の列をまとめて
//    取り出し、空いている列がその中にあればすぐ止める。
// 乱択の密な行列では 1 から 3 でほとんどの行が割り当たり、4 の Dijkstra は一部の行だけで済む。行列は行優先の 1 本の
// 配列のまま読む。
#include <limits>
#include <vector>
namespace asg_lapjv {
using i64= long long;
// 行 i の列を返し、費用の和を total に入れる。
inline std::vector<int> solve(int n, const std::vector<i64>& a, i64& total) {
 const i64 BIG= std::numeric_limits<i64>::max() / 4;
 auto c= [&](int i, int j) { return a[(size_t)i * n + j]; };
 std::vector<int> rowsol(n, -1), colsol(n, -1), matches(n, 0), freerows(n), collist(n), pred(n);
 std::vector<i64> v(n), d(n);
 // 1. column reduction
 for(int j= n; j--;) {
  i64 mn= c(0, j);
  int imin= 0;
  for(int i= 1; i < n; ++i)
   if(c(i, j) < mn) mn= c(i, j), imin= i;
  v[j]= mn;
  if(++matches[imin] == 1) rowsol[imin]= j, colsol[j]= imin;
  else if(v[j] < v[rowsol[imin]]) {
   const int j1= rowsol[imin];
   rowsol[imin]= j, colsol[j]= imin, colsol[j1]= -1;
  } else colsol[j]= -1;
 }
 // 2. reduction transfer
 int numfree= 0;
 for(int i= 0; i < n; ++i) {
  if(matches[i] == 0) freerows[numfree++]= i;
  else if(matches[i] == 1) {
   const int j1= rowsol[i];
   i64 mn= BIG;
   const i64* row= a.data() + (size_t)i * n;
   for(int j= 0; j < n; ++j)
    if(j != j1 && row[j] - v[j] < mn) mn= row[j] - v[j];
   v[j1]-= mn;
  }
 }
 // 3. augmenting row reduction (2 回)
 for(int loop= 0; loop < 2; ++loop) {
  int k= 0;
  const int prv= numfree;
  numfree= 0;
  while(k < prv) {
   const int i= freerows[k++];
   const i64* row= a.data() + (size_t)i * n;
   i64 umin= row[0] - v[0], usub= BIG;
   int j1= 0, j2= -1;
   for(int j= 1; j < n; ++j) {
    const i64 h= row[j] - v[j];
    if(h < usub) {
     if(h >= umin) usub= h, j2= j;
     else usub= umin, umin= h, j2= j1, j1= j;
    }
   }
   int i0= colsol[j1];
   if(umin < usub) v[j1]-= usub - umin;
   else if(i0 >= 0 && j2 >= 0) j1= j2, i0= colsol[j2];
   rowsol[i]= j1, colsol[j1]= i;
   if(i0 >= 0) {
    rowsol[i0]= -1;
    if(umin < usub) freerows[--k]= i0;
    else freerows[numfree++]= i0;
   }
  }
 }
 // 4. augmentation
 for(int f= 0; f < numfree; ++f) {
  const int fr= freerows[f];
  const i64* frow= a.data() + (size_t)fr * n;
  for(int j= 0; j < n; ++j) d[j]= frow[j] - v[j], pred[j]= fr, collist[j]= j;
  int low= 0, up= 0, last= -1, endofpath= -1;
  i64 mn= 0;
  bool found= false;
  do {
   if(up == low) {
    last= low - 1;
    mn= d[collist[up++]];
    for(int k= up; k < n; ++k) {
     const int j= collist[k];
     const i64 h= d[j];
     if(h <= mn) {
      if(h < mn) up= low, mn= h;
      collist[k]= collist[up], collist[up++]= j;
     }
    }
    for(int k= low; k < up; ++k)
     if(colsol[collist[k]] < 0) {
      endofpath= collist[k], found= true;
      break;
     }
   }
   if(!found) {
    const int j1= collist[low++];
    const int i= colsol[j1];
    const i64* row= a.data() + (size_t)i * n;
    const i64 h= row[j1] - v[j1] - mn;
    for(int k= up; k < n; ++k) {
     const int j= collist[k];
     const i64 v2= row[j] - v[j] - h;
     if(v2 < d[j]) {
      pred[j]= i;
      if(v2 == mn) {
       if(colsol[j] < 0) {
        endofpath= j, found= true;
        break;
       }
       collist[k]= collist[up], collist[up++]= j;
      }
      d[j]= v2;
     }
    }
   }
  } while(!found);
  for(int k= 0; k <= last; ++k) {
   const int j1= collist[k];
   v[j1]+= d[j1] - mn;
  }
  for(int i;;) {
   i= pred[endofpath];
   colsol[endofpath]= i;
   const int j1= endofpath;
   endofpath= rowsol[i], rowsol[i]= j1;
   if(i == fr) break;
  }
 }
 total= 0;
 for(int i= 0; i < n; ++i) total+= c(i, rowsol[i]);
 return rowsol;
}
}
struct Solver {
 int n;
 const vector<i64>& a;
 i64 total= 0;
 vector<int> p;
 Solver(int n, const vector<i64>& a): n(n), a(a) {}
 void run() { p= asg_lapjv::solve(n, a, total); }
 i64 cost() const { return total; }
 const vector<int>& assignment() const { return p; }
};
