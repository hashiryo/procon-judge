#pragma once
// 動的木の提出用の入出力。数字は非負だけを読む (問題の入力がすべて 0 以上)。
#include <cstdio>
#include <cstdint>

struct FastIn {
 static constexpr int BUF = 1 << 16;
 char b[BUF];
 int n = 0, i = 0;
 [[gnu::always_inline]] char peek() {
  if (i == n) n = int(fread(b, 1, BUF, stdin)), i = 0;
  return i < n ? b[i] : 0;
 }
 [[gnu::always_inline]] void bump() { ++i; }
 [[gnu::always_inline]] unsigned read_u32() {
  char c;
  while ((c = peek()) <= 32) bump();
  unsigned x = 0;
  while (c >= '0') x = x * 10 + unsigned(c - '0'), bump(), c = peek();
  return x;
 }
 [[gnu::always_inline]] unsigned long long read_u64() {
  char c;
  while ((c = peek()) <= 32) bump();
  unsigned long long x = 0;
  while (c >= '0') x = x * 10 + static_cast<unsigned long long>(c - '0'), bump(), c = peek();
  return x;
 }
 [[gnu::always_inline]] int read_int() { return int(read_u32()); }
};
struct FastOut {
 static constexpr int BUF = 1 << 16;
 char b[BUF];
 int n = 0;
 ~FastOut() { flush(); }
 void flush() {
  if (n) fwrite(b, 1, n, stdout), n = 0;
 }
 [[gnu::always_inline]] void put(char c) {
  if (n == BUF) flush();
  b[n++] = c;
 }
 void write_u64(unsigned long long x) {
  char t[20];
  int k = 0;
  do t[k++] = char('0' + x % 10), x /= 10;
  while (x);
  while (k) put(t[--k]);
 }
 void write_ll(long long x) {
  if (x < 0) put('-'), x = -x;
  write_u64((unsigned long long)x);
 }
 void nl() { put('\n'); }
};
