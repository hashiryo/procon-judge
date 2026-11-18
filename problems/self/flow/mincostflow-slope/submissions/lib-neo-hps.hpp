#pragma once
// 診断用。lib-neo-hp と同じく 2 MB 以上の確保に huge page を頼み、確保の先頭を取るたびに 2 KB + 64 byte の倍数だけずらす
// (_shared/pages/plain_new.hpp の PAGES_HP_STAGGER)。同じ添字で引く配列が、どれも 2 MB 境界から始まってキャッシュの同じ組を
// 取り合う分を、同じ束の lib-neo と lib-neo-hp と比べる。比べ終えたら消す。
#define PAGES_HP_STAGGER
#include "_shared/pages/plain_new.hpp"
#include "lib-neo.hpp"
