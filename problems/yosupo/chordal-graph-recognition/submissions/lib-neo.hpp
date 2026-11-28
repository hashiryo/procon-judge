#pragma once
#include "pj.hpp"
#include "neo/algo/decide/graph/chordal_recognition.hpp"

// NeoLibrary の chordal_recognition を使う提出。MCS は遅延バケット。
struct Solver {
  int n;
  vector<array<int, 2>> edges;
  ChordalRecognition res;
  Solver(int n_, const vector<array<int, 2>> &e) : n(n_), edges(e) {}
  void run() { res = chordal_recognition(n, edges); }
  bool chordal() const { return res.chordal; }
  const vector<int> &order() const { return res.order; }
  const vector<int> &cycle() const { return res.cycle; }
};
