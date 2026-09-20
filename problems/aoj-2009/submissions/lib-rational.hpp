#pragma once
#include "common.hpp"
#include "mylib/algebra/Rational.hpp"

// 有理数で座標を持つ。交点の座標が正確に出るので、一致判定が誤らない。
using Solver = GeoSolver<Rational<long long, false>>;
