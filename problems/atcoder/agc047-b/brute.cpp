// agc047-b の愚直解。文字列ごとに、先頭の 2 文字のどちらかを消す操作を幅優先でたどって、作れる文字列を
// 全部集める。そのうち入力にあるものを数える (操作で短くなるので、組は長い方からだけ数えられる)。
// 「先頭を除いた残りが末尾に一致し、先頭の文字が前の方にある」という言い換えもハッシュも使わないので、
// 提出とは別の考え方になる。短い文字列でだけ使う。
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <vector>

int main() {
  int n;
  if (!(std::cin >> n)) return 1;
  std::vector<std::string> s(n);
  for (auto &w : s) std::cin >> w;
  std::set<std::string> given(s.begin(), s.end());
  long long pairs = 0;
  for (const auto &w : s) {
    std::set<std::string> seen = {w};
    std::queue<std::string> bfs;
    bfs.push(w);
    while (!bfs.empty()) {
      std::string x = bfs.front();
      bfs.pop();
      for (int k = 0; k < 2 && k < (int)x.size(); ++k) {
        std::string y = x.substr(0, k) + x.substr(k + 1);
        if (seen.insert(y).second) bfs.push(y);
      }
    }
    for (const auto &y : seen)
      if (y != w && given.count(y)) ++pairs;
  }
  std::cout << pairs << '\n';
}
