#pragma once
// 愚直解。ベクトルの部分集合を全部試し、線形独立で色ごとの上限を守るもののうち要素数が最大のものを返す (2^m m)。
// 小さい入力 (gen.py の seed 1000 以上、ベクトル 16 本まで) でだけ使う。
struct Solver {
  const vector<u64> &v;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(const vector<u64> &vec, const vector<int> &color, const vector<int> &cap) : v(vec), color(color), cap(cap) {}

  void run() {
    const int m = v.size();
    unsigned best = 0;
    int best_size = 0;
    vector<int> cnt(cap.size());
    for (unsigned s = 0; s < (1u << m); ++s) {
      const int size = __builtin_popcount(s);
      if (size <= best_size) continue;
      fill(cnt.begin(), cnt.end(), 0);
      u64 basis[64] = {};
      bool ok = true;
      for (int i = 0; i < m && ok; ++i) {
        if (!(s >> i & 1)) continue;
        if (++cnt[color[i]] > cap[color[i]]) ok = false;
        u64 x = v[i];
        while (x && basis[63 - __builtin_clzll(x)]) x ^= basis[63 - __builtin_clzll(x)];
        if (!x) ok = false;
        else basis[63 - __builtin_clzll(x)] = x;
      }
      if (ok) best = s, best_size = size;
    }
    for (int i = 0; i < m; ++i)
      if (best >> i & 1) ans.push_back(i);
  }

  vector<int> answer() const { return ans; }
};
