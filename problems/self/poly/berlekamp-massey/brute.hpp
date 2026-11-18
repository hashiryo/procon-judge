#pragma once
// 愚直解: 長さ d = 0, 1, 2, ... の順に、連立一次方程式 (掃き出し) で漸化式があるかを調べ、最初に見つかったものを返す。
// 最短の漸化式が 1 つに決まる入力 (gen.py の 1000 以上の seed) だけを比べる前提で、一意でなければ参照実装と食い違う。
inline vector<u32> run(const vector<u32>& a) {
 constexpr long long M= 998244353;
 auto inv= [](long long x) {
  long long r= 1, e= M - 2;
  for(x%= M; e; e>>= 1, x= x * x % M)
   if(e & 1) r= r * x % M;
  return r;
 };
 const int n= int(a.size());
 for(int d= 0; d <= n; ++d) {
  // 方程式: i = d..n-1 で Σ_j c_j a_{i-j} = a_i
  const int rows= n - d;
  vector<vector<long long>> A(rows, vector<long long>(d + 1));
  for(int r= 0; r < rows; ++r) {
   for(int j= 1; j <= d; ++j) A[r][j - 1]= a[d + r - j];
   A[r][d]= a[d + r];
  }
  int row= 0;
  vector<int> piv(d, -1);
  for(int col= 0; col < d && row < rows; ++col) {
   int p= -1;
   for(int r= row; r < rows; ++r)
    if(A[r][col]) {
     p= r;
     break;
    }
   if(p < 0) continue;
   swap(A[p], A[row]);
   const long long iv= inv(A[row][col]);
   for(auto& v: A[row]) v= v * iv % M;
   for(int r= 0; r < rows; ++r)
    if(r != row && A[r][col]) {
     const long long f= A[r][col];
     for(int c= 0; c <= d; ++c) A[r][c]= ((A[r][c] - f * A[row][c]) % M + M) % M;
    }
   piv[col]= row++;
  }
  bool ok= true;
  for(int r= row; r < rows; ++r) ok&= A[r][d] == 0;
  if(!ok) continue;
  vector<u32> c(d, 0);
  for(int col= 0; col < d; ++col)
   if(piv[col] >= 0) c[col]= u32(A[piv[col]][d]);
  return c;
 }
 return {};
}
