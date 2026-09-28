// cf17-final-e の愚直解。操作 j を回数 x_j (26 を法とする) だけ施すと、左右の組 (i, |S| - 1 - i) の差
// S_i - S_{|S|-1-i} は Σ_j x_j ([i ∈ 操作 j] - [|S| - 1 - i ∈ 操作 j]) だけ変わる。全部の組の差を 0 にできるかを、
// この連立 1 次方程式が 26 を法として解けるかで決める。26 = 2 × 13 なので、2 を法と 13 を法の両方で
// 掃き出して、両方で解があれば YES。差分の列の折り返しもグラフも使わないので、incidence_matrix_equation
// とは別の考え方になる。O(|S| N min(|S|, N)) なので、小さい入力でだけ使う。
#include <cstdio>
#include <string>
#include <vector>

// 行列 a (各行の最後の列が右辺) の方程式が p を法として解けるか。p は素数。
bool solvable(std::vector<std::vector<int>> a, int p) {
  const int rows = a.size(), cols = rows ? a[0].size() - 1 : 0;
  auto inv = [p](int x) {
    for (int y = 1; y < p; ++y)
      if (x * y % p == 1) return y;
    return 0;
  };
  int r = 0;
  for (int c = 0; c < cols && r < rows; ++c) {
    int pivot = -1;
    for (int i = r; i < rows; ++i)
      if (a[i][c] % p != 0) pivot = i;
    if (pivot < 0) continue;
    std::swap(a[r], a[pivot]);
    int iv = inv(a[r][c]);
    for (int& x : a[r]) x = x * iv % p;
    for (int i = 0; i < rows; ++i)
      if (i != r && a[i][c] != 0) {
        int f = a[i][c];
        for (int k = 0; k <= cols; ++k) a[i][k] = ((a[i][k] - f * a[r][k]) % p + p) % p;
      }
    ++r;
  }
  for (int i = r; i < rows; ++i)
    if (a[i][cols] != 0) return false;
  return true;
}

int main() {
  char buf[200001];
  int m;
  if (scanf("%200000s %d", buf, &m) != 2) return 1;
  std::string s = buf;
  const int n = s.size(), h = n / 2;
  std::vector<int> left(m), right(m);
  for (int j = 0; j < m; ++j)
    if (scanf("%d %d", &left[j], &right[j]) != 2) return 1;
  bool ok = true;
  for (int p : {2, 13}) {
    std::vector<std::vector<int>> a(h, std::vector<int>(m + 1));
    for (int i = 0; i < h; ++i) {
      for (int j = 0; j < m; ++j) {
        int in_i = left[j] - 1 <= i && i <= right[j] - 1;
        int in_mirror = left[j] - 1 <= n - 1 - i && n - 1 - i <= right[j] - 1;
        a[i][j] = ((in_i - in_mirror) % p + p) % p;
      }
      a[i][m] = (((s[n - 1 - i] - s[i]) % p) + p) % p;
    }
    ok = ok && solvable(a, p);
  }
  puts(ok ? "YES" : "NO");
}
