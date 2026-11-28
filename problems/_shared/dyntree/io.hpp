#pragma once
// 動的木の提出用の入出力。数字は非負だけを読む (問題の入力がすべて 0 以上)。
// 入力を最初に全部読み、末尾に 8 byte の 0 を置く。数字は 4 byte (u32) か 8 byte (u64) をまとめて取り、
// 全部が数字なら掛け算とシフトで一度に畳む (chaihf の 270626 と同じ)。数字が 4 桁を超えるときは、
// 畳んだあと残りを 1 文字ずつ足す。書きは 1 MB のバッファに 4 桁ずつの表を写す。
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#ifdef __linux__
#include <sys/stat.h>
#include <unistd.h>
#endif

struct FastIn {
 std::vector<char> buf;
 const char* p = nullptr;
 // 4 byte か 8 byte がまとめて数字か (0x30 を引いて、上の位の bit が立たないこと)。
 static constexpr bool all_digit(std::uint32_t x) { return !((x ^ 0x30303030u) & 0xf0f0f0f0u); }
 static constexpr bool all_digit(std::uint64_t x) { return !((x ^ 0x3030303030303030ull) & 0xf0f0f0f0f0f0f0f0ull); }
 FastIn() {
  std::size_t cap = std::size_t(1) << 20;
#ifdef __linux__
  struct stat st;
  if (fstat(0, &st) == 0 && st.st_size > 0) cap = std::size_t(st.st_size) + 8;
#endif
  buf.resize(cap);
  std::size_t n = 0;
  for (;;) {
   if (n + 8 >= cap) cap *= 2, buf.resize(cap);
   const std::size_t got = fread(buf.data() + n, 1, cap - n - 8, stdin);
   n += got;
   if (!got) break;
  }
  std::memset(buf.data() + n, 0, 8);
  p = buf.data();
 }
 [[gnu::always_inline]] unsigned read_u32() {
  std::uint32_t x = 0, a;
  std::memcpy(&a, p, 4);
  if (all_digit(a)) {
   a ^= 0x30303030u;
   a = (a * 10u + (a >> 8)) & 0x00ff00ffu;
   a = (a * 100u + (a >> 16)) & 0x0000ffffu;
   x = a, p += 4;
  }
  while (*p >= '0') x = x * 10u + std::uint32_t(*p - '0'), ++p;
  ++p;
  return x;
 }
 [[gnu::always_inline]] unsigned long long read_u64() {
  std::uint64_t x = 0, a;
  std::memcpy(&a, p, 8);
  if (all_digit(a)) {
   a ^= 0x3030303030303030ull;
   a = (a * 10u + (a >> 8)) & 0x00ff00ff00ff00ffull;
   a = (a * 100u + (a >> 16)) & 0x0000ffff0000ffffull;
   a = (a * 10000u + (a >> 32)) & 0x00000000ffffffffull;
   x = a, p += 8;
  }
  while (*p >= '0') x = x * 10u + std::uint64_t(*p - '0'), ++p;
  ++p;
  return x;
 }
 [[gnu::always_inline]] int read_int() { return int(read_u32()); }
};

struct FastOut {
 static constexpr int BUF = 1 << 20;
 char b[BUF + 64];
 int n = 0;
 ~FastOut() { flush(); }
 void flush() {
  if (n) fwrite(b, 1, std::size_t(n), stdout), n = 0;
 }
 // 0 から 9999 までの 4 桁 (先頭の 0 も書く)。
 static constexpr auto tab = [] {
  struct T {
   char d[10000][4];
  } t{};
  for (int i = 0; i < 10000; ++i)
   t.d[i][0] = char('0' + i / 1000), t.d[i][1] = char('0' + i / 100 % 10), t.d[i][2] = char('0' + i / 10 % 10), t.d[i][3] = char('0' + i % 10);
  return t;
 }();
 void put4(unsigned x) { std::memcpy(b + n, tab.d[x], 4), n += 4; }
 void write_u64(unsigned long long x) {
  if (n > BUF - 64) flush();
  if (x > 99999999ull) {
   char t[24];
   int k = 0;
   do t[k++] = char('0' + x % 10), x /= 10;
   while (x);
   while (k) b[n++] = t[--k];
  } else if (x > 9999) {
   char t[8];
   int k = 0;
   unsigned hi = unsigned(x / 10000);
   do t[k++] = char('0' + hi % 10), hi /= 10;
   while (hi);
   while (k) b[n++] = t[--k];
   put4(unsigned(x % 10000));
  } else {
   char t[8];
   int k = 0;
   do t[k++] = char('0' + x % 10), x /= 10;
   while (x);
   while (k) b[n++] = t[--k];
  }
 }
 void write_ll(long long x) {
  if (x < 0) b[n++] = '-', x = -x;
  write_u64((unsigned long long)x);
 }
 void nl() { b[n++] = '\n'; }
};
