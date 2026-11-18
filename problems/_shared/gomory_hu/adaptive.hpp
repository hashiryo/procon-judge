#pragma once
// Gusfield の方法の最大流の部分を、flow ごとに Dinic か push-relabel かを選んで流すもの (gusfield.hpp の Engine)。
// 中身は hybrid.hpp の 2 つの方式 (1 回の flow の中では移らない)。Dinic で流した flow の段の数 (t に届いた BFS の回数)
// の指数移動平均 (重み 1/8) を持ち、平均が T を超えている間は push-relabel で流す。ただし R 回に 1 回は Dinic で流して
// 平均を測り直し、下がれば Dinic に戻る。self-flow-gomory-hu の手元の値では、段の平均が 3.5 以下の網 (乱択、密、木に
// 近い形、単位容量) は Dinic が 1.1〜2.2 倍速く、5.6 以上の網 (格子、塊、輪) は push-relabel が 1.8〜1.9 倍速かった。
// 木に近い形は、初めのほうの flow (遠い組) だけ段が多いので、一度移っても戻れるようにしている。
#include "_shared/gomory_hu/hybrid.hpp"

namespace gomory_hu_shared {
template <int R = 8, int T10 = 45> struct AdaptiveEngine : HybridEngine<1> {
  using Base = HybridEngine<1>;
  using i64 = long long;
  double ema = 0;
  long long cnt = 0;
  AdaptiveEngine(int n, const std::vector<std::array<i64, 3>> &edges) : Base(n, edges) {}
  i64 flow(int s, int t) {
    std::memcpy(cap.data(), cap0.data(), sizeof(i64) * m2);
    if (ema * 10 > T10 && ++cnt % R != 0) return pr = true, push_relabel(s, t);
    pr = false;
    i64 f = 0;
    int ph = 0;
    while (bfs(s, t)) {
      ++ph;
      for (int v = 0; v < n; ++v) it[v] = st[v];
      f += dfs(t, s, std::numeric_limits<i64>::max());
    }
    ema += (ph - ema) / 8;
    return f;
  }
};
}  // namespace gomory_hu_shared
