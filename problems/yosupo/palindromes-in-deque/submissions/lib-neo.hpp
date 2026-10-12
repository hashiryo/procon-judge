#pragma once
// NeoLibrary の PalindromeDeque (dq_tk2 を写したもの) を呼ぶ提出。クエリに現れる文字に 0 から番号を振り、種類の数を 2、4、8、26 の
// どれかに切り上げた幅で組む。答えの列は dq_tk2 と同じく、領域を取ってから huge page を頼む (lib-neo が 4 KB のページのまま 0 で埋めて
// いた run 38220397521 と 38222495066 では、random の組が dq_tk2 より 2.5 ms 遅かった)。
#include <array>
#include <cstdint>
#include <utility>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "pj.hpp"
#include "neo/string/PalindromeDeque.hpp"

namespace pdq_lib_neo {
// v に n 個分の領域を取り (中身はまだ書かない)、Linux ではその領域のページに huge page を頼む。
template <class T> inline void reserve_huge(std::vector<T> &v, size_t n) {
  v.reserve(n);
#ifdef __linux__
  const uintptr_t b = reinterpret_cast<uintptr_t>(v.data()) & ~uintptr_t(4095), e = (reinterpret_cast<uintptr_t>(v.data() + n) + 4095) & ~uintptr_t(4095);
  if (b < e) madvise(reinterpret_cast<void *>(b), e - b, MADV_HUGEPAGE);
#endif
}
}  // namespace pdq_lib_neo

struct Solver {
  vector<pair<int, char>> qs;
  vector<array<int, 3>> ans;

  explicit Solver(const vector<pair<int, char>> &qs) : qs(qs) {}

  template <int K> void body(const array<int, 256> &code) {
    int pushes = 0;
    for (const auto &[type, ch] : qs) pushes += type <= 1;
    PalindromeDeque<K> d(pushes);
    pdq_lib_neo::reserve_huge(ans, qs.size());
    ans.resize(qs.size());
    for (size_t t = 0; t < qs.size(); ++t) {
      const auto [type, ch] = qs[t];
      if (type == 0) d.push_front(code[(unsigned char)ch]);
      else if (type == 1) d.push_back(code[(unsigned char)ch]);
      else if (type == 2) d.pop_front();
      else d.pop_back();
      ans[t] = {d.distinct(), d.longest_prefix(), d.longest_suffix()};
    }
  }

  void run() {
    array<int, 256> code;
    code.fill(0);
    array<bool, 256> used{};
    int k = 0;
    for (const auto &[type, ch] : qs)
      if (type <= 1 && !used[(unsigned char)ch]) used[(unsigned char)ch] = true, code[(unsigned char)ch] = k++;
    if (k <= 2) body<2>(code);
    else if (k <= 4) body<4>(code);
    else if (k <= 8) body<8>(code);
    else body<26>(code);
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
