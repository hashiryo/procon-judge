#pragma once
// 今の Library の SortedPerBucket (mylib/data_structure/SortedPerBucket.hpp)。列をバケットに分け、バケットごとに
// 並べた値と累積を持つ平方分割。raw のときの lib-sqrtdc.cpp と同じ中身。
#include "pj.hpp"
#include "mylib/data_structure/SortedPerBucket.hpp"
struct Solver {
 SortedPerBucket<long long> spb;
 static vector<long long> wide(const vector<int>& a) { return vector<long long>(a.begin(), a.end()); }
 explicit Solver(const vector<int>& a): spb(wide(a)) {}
 i64 sum_le(int l, int r, int x) { return spb.sum(l, r, x + 1); }
};
