#pragma once
#include "common.hpp"
#include "neo/algo/optimize/line/MonotoneCHT.hpp"

// NeoLibrary の MonotoneCHT (傾きが単調な両端キューの CHT)。問いの x は増えるので query_monotone で引く。
struct Lines {
  MonotoneCHT<i64> c;
  void insert(i64 a, i64 b) { c.add_line(a, b); }
  i64 query(i64 x) { return c.query_monotone(x); }
};

using Solver = FrogSolver<Lines>;
