// 診断用。lib-neo-antichain-hp と同じく 2 MB 以上の確保に huge page を頼み、確保の先頭を取るたびに 2 KB + 64 byte の倍数だけ
// ずらす (_shared/pages/plain_new.hpp の PAGES_HP_STAGGER)。網の頂点ごとの配列が同じ添字で同じキャッシュの組を取り合う分を、
// 同じジョブの lib-neo-antichain と lib-neo-antichain-hp と比べる。比べ終えたら消す。
#define PAGES_HP_STAGGER
#include "_shared/pages/plain_new.hpp"
#include "lib-neo-antichain.cpp"
