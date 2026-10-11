#pragma once
// snca_pk_hp の DFS の隣接を、yosupo-scc の gabow_split と同じく、最初の行き先と残りの出辺の連結リストの先頭の別々の配列にし、辺の列を 1 回なめて組む。DFS が下りる 1 歩が読み込み 1 段で済む。核は _dom.hpp。
#include "_dom.hpp"
using Solver= dom::Solver<4>;
