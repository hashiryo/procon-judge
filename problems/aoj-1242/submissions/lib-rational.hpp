#pragma once
#include "common.hpp"
#include "mylib/algebra/Rational.hpp"

// 有理数で座標を持つ。交点が正確に出るので、切り上げと切り捨てに許容誤差が
// 要らない。
using Solver = GeoSolver<Rational<long long, false>>;
