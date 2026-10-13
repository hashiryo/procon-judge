// harness: 各提出が定義する run(in, out) を計測する。問と答えの形は common.hpp にある。
// 答えを書く場所はハーネスが大きさの上限だけ確保して 0 で埋めておく (ページに触るのは計測の外)。
// I/O 変換は計測外で行い、純粋なアルゴリズム時間のみを ALGO_TIME_NS に含める。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/fastest.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int T;
 cin >> T;
 Input in;
 in.qs.resize(T);
 size_t bound= 0;
 for(auto& q: in.qs) {
  string op;
  cin >> op;
  q= Query{};
  if(op == "ENCODE_PATH") q.type= ENCODE, cin >> q.x[0] >> q.x[1], bound+= 1 + MAX_RUNS;
  else if(op == "DECODE_PATH") {
   q.type= DECODE;
   int k;
   cin >> k;
   q.off= (u32)in.path.size();
   for(int i= 0; i < k; ++i) {
    string c;
    u32 n;
    cin >> c >> n;
    if(i == 0 && c == "L") in.path.push_back(0);
    in.path.push_back(n);
   }
   q.len= (u32)in.path.size() - q.off;
   bound+= 2;
  } else if(op == "LCA") q.type= LCA, cin >> q.x[0] >> q.x[1] >> q.x[2] >> q.x[3], bound+= 2;
  else if(op == "ANCESTOR") q.type= ANCESTOR, cin >> q.x[0] >> q.x[1] >> q.x[2], bound+= 2;
  else q.type= RANGE, cin >> q.x[0] >> q.x[1], bound+= 4;
 }
 vector<u32> buf(bound);

 auto t0= chrono::steady_clock::now();
 const u32* end= run(in, buf.data());
 auto t1= chrono::steady_clock::now();

 string out;
 out.reserve(bound * 6);
 const u32* p= buf.data();
 auto put= [&](u32 v) {
  char s[16];
  auto r= to_chars(s, s + 16, v);
  out.append(s, r.ptr);
 };
 for(const auto& q: in.qs) {
  if(q.type == ENCODE) {
   const u32 m= *p++;
   if(m > MAX_RUNS) {
    fprintf(stderr, "run() wrote %u runs for ENCODE_PATH\n", m);
    return 1;
   }
   u32 k= 0;
   for(u32 i= 0; i < m; ++i) k+= p[i] != 0;
   put(k);
   for(u32 i= 0; i < m; ++i)
    if(p[i]) out+= i & 1 ? " L " : " R ", put(p[i]);
   p+= m;
  } else if(q.type == RANGE) {
   for(int i= 0; i < 4; ++i) out+= i ? " " : "", put(p[i]);
   p+= 4;
  } else if(q.type == ANCESTOR && p[0] == 0) out+= "-1", p+= 2;
  else put(p[0]), out+= ' ', put(p[1]), p+= 2;
  out+= '\n';
 }
 if(p != end) {
  fprintf(stderr, "run() wrote %td values, expected %td\n", end - buf.data(), p - buf.data());
  return 1;
 }
 fwrite(out.data(), 1, out.size(), stdout);
 report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
 return 0;
}
