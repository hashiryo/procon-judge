#pragma once
#include "../common.hpp"
// =============================================================================
// yosupo Stern-Brocot Tree の最速提出 (cuixiang19, https://judge.yosupo.jp/submission/327748)
// を移植したもの。入出力はハーネスの形 (common.hpp) に合わせ、手順はそのまま。
//
// アルゴリズム:
//   各クエリで Stern-Brocot 木上を mediant 計算で root から目標 rational に
//   到達する経路を辿る。LCA, ANCESTOR, RANGE 等は同じ traversal の派生。
//   全部が i64 整数の比較・加減算・除算で実装されている。
//
// 注: 元提出は luogu P1797 用 (= 同問題の中国版)、yosupo の入出力フォーマットと
//     同じ。
// =============================================================================
inline u32* run(const Input& in, u32* out) {
 for (const auto& q : in.qs) {
  if (q.type == DECODE) {
   i64 lx = 0, ly = 1, rx = 1, ry = 0;
   for (u32 i = 0; i < q.len; ++i) {
    const i64 x = in.path[q.off + i];
    if (i & 1) {  // L
     rx += lx * x;
     ry += ly * x;
    } else {  // R
     lx += rx * x;
     ly += ry * x;
    }
   }
   *out++ = (u32)(lx + rx), *out++ = (u32)(ly + ry);
  } else if (q.type == ENCODE) {
   const i64 x = q.x[0], y = q.x[1];
   i64 lx = 0, ly = 1, rx = 1, ry = 0;
   u32* head = out++;
   while (true) {
    i64 mx = lx + rx, my = ly + ry;
    if (mx == x && my == y) break;
    if (x * my < y * mx) {
     i64 k = (y * rx - x * ry - 1) / (x * ly - y * lx);
     if (out == head + 1) *out++ = 0;  // 最初の連が L なら、長さ 0 の R の連を置く
     *out++ = (u32)k;
     rx += lx * k;
     ry += ly * k;
    } else {
     i64 k = (x * ly - y * lx - 1) / (y * rx - x * ry);
     *out++ = (u32)k;
     lx += rx * k;
     ly += ry * k;
    }
   }
   *head = (u32)(out - head - 1);
  } else if (q.type == LCA) {
   i64 ax = q.x[0], ay = q.x[1], bx = q.x[2], by = q.x[3];
   if (ax * by > bx * ay) { std::swap(ax, bx); std::swap(ay, by); }
   i64 lx = 0, rx = 1, ly = 1, ry = 0;
   while (true) {
    i64 mx = lx + rx, my = ly + ry;
    if (ax * my <= ay * mx && mx * by <= my * bx) {
     *out++ = (u32)mx, *out++ = (u32)my;
     break;
    }
    if (bx * my < by * mx) {
     i64 k = (by * rx - bx * ry - 1) / (bx * ly - by * lx);
     rx += lx * k;
     ry += ly * k;
    } else {
     i64 k = (ax * ly - ay * lx - 1) / (ay * rx - ax * ry);
     lx += rx * k;
     ly += ry * k;
    }
   }
  } else if (q.type == ANCESTOR) {
   i64 z = q.x[0], x = q.x[1], y = q.x[2];
   i64 lx = 0, ly = 1, rx = 1, ry = 0;
   std::vector<std::pair<char, i64>> ans;
   while (true) {
    i64 mx = lx + rx, my = ly + ry;
    if (mx == x && my == y) break;
    if (x * my < y * mx) {
     i64 k = (y * rx - x * ry - 1) / (x * ly - y * lx);
     ans.emplace_back('L', k);
     rx += lx * k;
     ry += ly * k;
    } else {
     i64 k = (x * ly - y * lx - 1) / (y * rx - x * ry);
     ans.emplace_back('R', k);
     lx += rx * k;
     ly += ry * k;
    }
   }
   lx = 0; ly = 1; rx = 1; ry = 0;
   for (auto [c, n] : ans) {
    if (c == 'L') {
     if (z <= n) { rx += lx * z; ry += ly * z; z = 0; break; }
     z -= n; rx += lx * n; ry += ly * n;
    } else {
     if (z <= n) { lx += rx * z; ly += ry * z; z = 0; break; }
     z -= n; lx += rx * n; ly += ry * n;
    }
   }
   if (z) *out++ = 0, *out++ = 0;
   else *out++ = (u32)(lx + rx), *out++ = (u32)(ly + ry);
  } else {  // RANGE
   const i64 x = q.x[0], y = q.x[1];
   i64 lx = 0, rx = 1, ly = 1, ry = 0;
   while (true) {
    i64 mx = lx + rx, my = ly + ry;
    if (mx == x && my == y) {
     *out++ = (u32)lx, *out++ = (u32)ly, *out++ = (u32)rx, *out++ = (u32)ry;
     break;
    }
    if (x * my < y * mx) {
     i64 k = (y * rx - x * ry - 1) / (x * ly - y * lx);
     rx += lx * k;
     ry += ly * k;
    } else {
     i64 k = (x * ly - y * lx - 1) / (y * rx - x * ry);
     lx += rx * k;
     ly += ry * k;
    }
   }
  }
 }
 return out;
}
