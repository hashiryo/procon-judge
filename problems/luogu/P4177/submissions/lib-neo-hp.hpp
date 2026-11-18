#pragma once
// 診断用。lib-neo と同じ本体に、2 MB 以上の確保を 2 MB 境界の mmap に置いて huge page を頼む operator new を足す
// (_shared/pages/plain_new.hpp)。NeoLibrary の MaxFlow の大きい配列を 4 KB のページで持つ分を、同じ束の lib-neo と比べる。
// 入力を読む配列も同じく載せ替わるので、ライブラリだけに入れたときの上限になる。比べ終えたら消す。
#define PAGES_HP
#include "_shared/pages/plain_new.hpp"
#include "lib-neo.hpp"
