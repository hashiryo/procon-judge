# procon-judge

自分用のオンラインジャッジです。`problems/` の下に問題を、その `submissions/` に提出を置いて main に push します。GitHub Actions が 4 つの環境 (x64 と arm、それぞれ gcc と clang) で提出をコンパイルして走らせ、正誤と時間とメモリを記録します。記録は `results` ブランチに溜まり、順位表と提出ページを https://hashiryo.github.io/procon-judge/ に出します。設計と実装の記録は [DESIGN.md](DESIGN.md) にあり、この README はそこから日常の操作だけを抜き出したものです。

AtCoder の提出ページに無いものが 1 つあります。提出はリポジトリの中のファイルなので、include している自分のライブラリ (hashiryo/Library) が変わると自動で測り直され、同じ URL のページが最新の記録を指し続けます。

## 手元の用意

`uv` があれば動きます。依存は `pyproject.toml` にあり、`uv run pj --help` で CLI が出ます。

```
git clone https://github.com/hashiryo/procon-judge.git
cd procon-judge
git submodule update --init third_party/simde
git clone --depth=1 https://github.com/hashiryo/Library.git lib
uv run pj problems list | head
```

`lib/` はライブラリの置き場で、git 管理外です。CI は実行のたびに最新を clone するので、手元も自分で用意します。無いままだと `mylib/...` を include する提出が include 未解決になって走りません。

Library の作業ツリーが隣にあるなら、clone せず、`ln -s ../Library lib` でシンボリックリンクを張ってもかまいません。Library でまだ commit していない変更を、手元の `pj repro` などでそのまま試せます。CI は `lib/` を見ずに Library の既定のブランチを取るので、CI に測らせるときは Library を先に push します。

手元の環境は `environments.toml` の `local` で、`c++` (macOS では Apple clang) を使います。CI の 4 環境は GitHub のランナーでしか動きません。

テストデータの保管庫は private リポジトリ hashiryo/procon-judge-testdata の Release アセットです。読み書きするには fine-grained PAT を環境変数 `TESTDATA_TOKEN` に入れます。無くても、判定サイトから直接取れる取得元 (Library Checker、AOJ、yukicoder、LOJ) は原本から取れます。yukicoder は `YUKICODER_TOKEN` も要ります。

`.results/` は `results` ブランチの中身 (問題ごとの jsonl) を置く場所で、これも git 管理外です。手元で `pj plan` や `pj site build` を試すときや、CI で測り済みのものを飛ばして走らせたいときに用意します。無ければ全部が未計測として扱われるだけです。

```
git fetch origin results
rm -rf .results && mkdir .results
git archive origin/results | tar -x -C .results
```

## リポジトリの構成

| 場所 | 中身 |
| --- | --- |
| `problems/<グループ>/<名前>/problem.toml` | 問題の定義。id、制限、ハーネスの種別、テストデータの取得元、出力比較 |
| `problems/<グループ>/<名前>/base.cpp` | ハーネス。`harness.kind = "base"` のときだけ |
| `problems/<グループ>/<名前>/common.hpp` | その問題の提出だけが共有するもの。省略可 |
| `problems/<グループ>/<名前>/gen.py` | 自作テストデータのジェネレータ。`testdata.source = "local"` のときだけ |
| `problems/<グループ>/<名前>/submissions/` | 提出。base なら `.hpp`、raw なら `.cpp` |
| `problems/_shared/<名前>/` | 問題をまたいで提出が使うヘッダ。problem.toml が無いので問題ではない |
| `harness/pj.hpp` | 全問題のハーネスと提出が共有するもの |
| `environments.toml` | 環境 (コンパイラとフラグ) の定義 |
| `testdata.toml` | Library Checker の問題集のコミットの pin |
| `libraries.toml` | 提出が include するライブラリと、そのページへのリンクの形 |
| `pj/` | 実装 (Python)。`tests/` に pytest |
| `.github/workflows/judge.yml` | CI |

## 問題を足す

### 1. ディレクトリと problem.toml

問題のディレクトリを作り、`problem.toml` を置きます。`problems/` の下は何段でも掘れて、そこからの各段を `-` で繋いだものが id です。`problems/atcoder/abc172-d/` なら `atcoder-abc172-d`、`problems/gf2-64/pow/` なら `gf2-64-pow` で、`problem.toml` の `id` はこれと一致させます。判定サイトの問題は接頭辞のディレクトリ (`atcoder/`、`aoj/`、`yosupo/`、`yuki/`、`loj/` など) に置きます。自作の問題は族のディレクトリ (`gf2-64/`、`modulo-test/` など) に置き、族の無いものは `problems/warshall-floyd/` のように 1 段で置きます。id は `<出どころ>-<問題>` の形で、出どころはテストデータの取得元ではなく、問題そのものがどこの問題かです。

| 出どころ | 接頭辞 | 例 |
| --- | --- | --- |
| Library Checker | `yosupo-` | `yosupo-unionfind` |
| AOJ | `aoj-` | `aoj-DSL_2_B` |
| yukicoder | `yuki-` | `yuki-1234` |
| AtCoder | `atcoder-` | `atcoder-abc172-d` |
| LOJ | `loj-` | `loj-6620` |
| HackerRank、CSES、Codeforces など | `hackerrank-`、`cses-`、`cf-` | `cses-2132` |
| 自作 | 付けない | `gf2-64-pow` |

id は記録のキーと保管庫のアセット名に入るので、あとから変えると記録が全部測り直しになり、保管庫のアセットも孤児になります。付けるときに決めます。

Library Checker の問題を、自分のライブラリと手書きの実装で比べる形の例です。置き場所は `problems/yosupo/unionfind/` です。

```toml
id = "yosupo-unionfind"
title = "Unionfind"

[limits]
tle_sec = 5.0
mle_mb = 1024

[harness]
kind = "base"

[testdata]
source = "library_checker"
name = "data_structure/unionfind"

[compare]
kind = "checker"
```

自作のテストデータで速さを比べる形の例です。置き場所は `problems/gf2-64/pow/` です。

```toml
id = "gf2-64-pow"
title = "GF(2^64) の冪"

[limits]
tle_sec = 10.0
mle_mb = 512

[harness]
kind = "base"

[testdata]
source = "local"
generator = "gen.py"
count = 7
reference = "submissions/reference.hpp"

[compare]
kind = "tokens"
```

`title` は表示にだけ使います。判定サイトから取る問題は `uv run pj problems titles --problem <id> --fix` で判定サイトの名前に揃えます。手で書くと違う名前になりがちです。`local` や `manual` や `none` の問題には判定サイトの名前が無いので手で付けます。`url` は元の問題のページで、`source` が `none` か `manual` の問題だけに書きます。ほかは `source` と `name` から組めます。

`[limits]` の `tle_sec` はケースごとの実時間の上限で、超えたら kill して TLE にします。`mle_mb` はピーク RSS の後判定です。入力を全部メモリに読むハーネスの形なので、元の判定サイトの制限に合わせる意味はなく、このハーネスでの制限として決めます。RSS には実行ファイルと libc のぶんで何もしないプログラムでも数 MB が乗るので、きつくしすぎないでください。既定は 5 秒と 256 MB です。

`[harness]` の `kind` は `base` か `raw` です。`base` は問題が `base.cpp` を持ち、提出はそれが決めたインターフェースを実装します。入出力はハーネスが担当し、計測区間の時間 (algo 時間) が順位表の基準になります。`raw` は提出が `main()` ごと持ち、判定サイトの入出力をそのまま読み書きします。実装が 1 本しか無い verify の置き場で、計測区間が無いので順位表にはなりません。

`[testdata]` の `source` はテストデータの取得元です。

| source | 説明 | 追加の項目 |
| --- | --- | --- |
| `library_checker` | yosupo06/library-checker-problems を `testdata.toml` のコミットで clone して生成します | `name` に問題のパス |
| `aoj` | judgedat の API から取ります | `name` に問題 ID |
| `yukicoder` | API から取ります。`YUKICODER_TOKEN` が要ります | `name` に問題 ID |
| `loj` | API から取ります | `name` に問題の番号 |
| `manual` | 手で取り込んで保管庫に置いたものだけを使います | `name` に識別子 (通常は id) |
| `local` | リポジトリ内のジェネレータと参照実装で作ります | `generator`、`count`、`reference` |
| `none` | テストデータを使いません | なし |

`[compare]` の `kind` は出力の比べ方です。

| kind | 説明 |
| --- | --- |
| `tokens` | 空白と改行を無視してトークン列を比べます。既定です |
| `float` | トークン比較で数値の誤差を許します。`abs_tol` か `rel_tol` を書きます |
| `checker` | Library Checker が同梱するチェッカに委ねます |
| `exit_code` | 期待出力を使わず、終了コードが 0 なら AC です。`source = "none"` の自己検証用です |
| `compile_only` | 実行せず、コンパイルが通れば AC です。テストデータが公開されていない AtCoder の問題はこれにします |

書いたら `uv run pj problems check --problem <id>` で検証します。id とディレクトリ名の不一致や、`source` と接頭辞の食い違いはここで分かります。

### 2. ハーネス base.cpp

`kind = "base"` の問題は `base.cpp` を書きます。入力を全部読んでから計測区間に入り、提出が実装する型か関数を呼び、出力は文字列に溜めて最後に書き、末尾で `report_metrics` に計測区間のナノ秒を渡します。

```cpp
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  Solver uf(n);
  string out;
  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) uf.unite(e[1], e[2]);
    else out += uf.same(e[1], e[2]) ? "1\n" : "0\n";
  }
  auto t1 = chrono::steady_clock::now();

  fputs(out.c_str(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
```

`SUBMISSION_HPP` は `pj` がコンパイルのとき `-D` で提出ごとに差し替えます。`#ifndef` の既定はエディタで単体表示したときの補完のためです。

インターフェースは問題ごとに決めますが、形は 2 つに寄せます。クエリを順に処理する問題は、構築に初期状態だけを渡して計測区間の中でクエリを 1 つずつ渡す `Solver` にします。先の入力を構築へ渡すと、オフラインの前処理を計測区間の外に置ける実装だけが有利になるからです。1 回の計算で答えが出る問題は、構築と `run()` と `answer()` の 3 段に分けます。内部表現への変換が計測区間の外に出ます。

ハーネスからは `mylib/...` を include しません。ライブラリを取れなかった回に問題ごと止まるのと、ライブラリを直しただけで手書きの実装まで測り直しになるのを避けるためです。剰余を扱う問題でも入出力は素の整数でやりとりして、提出が自分の型に直します。同じ問題の提出だけで共有したいもの (作用素の定義など) は `common.hpp` に置きます。こちらはライブラリを include してかまいません。

`harness/pj.hpp` には `must_scan`、`read_token`、`read_ints`、`print_all`、`report_metrics` があります。ここを変えると全問題が測り直しになるので、問題ごとの事情は `common.hpp` に書きます。

### 3. テストデータ

判定サイトから取る取得元 (`library_checker`、`aoj`、`yukicoder`、`loj`) は何もしなくてよいです。CI が最初に測るとき原本から取って保管庫へ上げ、以後は保管庫から取ります。手元で先に確かめるなら `uv run pj fetch --problem <id>` です。

`local` は `gen.py` と参照実装を書きます。`gen.py` は seed を 1 つ引数に取り、そのケースの入力を stdout に出します。`count` 個の seed を 0 から順に使うので生成は決定的です。`uv run --script` で動かすので、依存があれば PEP 723 のメタデータで書きます。参照実装は `kind = "base"` なら提出と同じ形の `.hpp` で、`submissions/reference.hpp` に置いて `reference` にそのパスを書けば、期待出力を作るものがそのまま順位表の 1 行になります。ジェネレータかハーネスか参照実装を直せば、期待出力も作り直されます。手元で作って中身を見るなら `uv run pj fetch --problem <id>` で、置き場が表示されます。

`manual` は手元で `.in` と `.out` を同じ名前で 1 つのディレクトリに並べ、取り込んで保管庫へ上げます。保管庫に無い `manual` の問題は CI が測らないので、push の前に上げます。ケース名は記録のキーに入るので、あとから変えません。

```
uv run pj testdata import --problem <id> --dir <ディレクトリ>
uv run pj mirror push --problem <id>
```

`none` はテストデータを使いません。自己検証の提出 (assert で自分を検証して 0 で終わるもの) は `compare.kind = "exit_code"` にします。入力を読む提出でテストデータが無いもの (AtCoder) は `compare.kind = "compile_only"` にします。

### 4. 提出を置く

問題のディレクトリの `submissions/` にファイルを置きます。`base` の提出はハーネスから include されるので `.hpp`、`raw` の提出はそれ自体が翻訳単位なので `.cpp` です。先頭が `_` のファイルは提出として扱いません。

自分のライブラリを使う提出は `#include "mylib/data_structure/UnionFind.hpp"` のように `lib/` からの相対パスで書きます。全環境のフラグに `-Ilib` が入っています。慣習として、ライブラリを使う提出は `lib.hpp`、同じ問題に複数あれば `lib-<実装>.hpp`、手書きの素朴な実装は `naive.hpp` と名付けますが、`pj` はこの名前に意味を持たせません。ライブラリとの紐付けは include の一覧から作ります。

提出ではない共通のヘッダは 3 つの置き方があり、どれも名前ではなく置き場所で区別します。その問題の提出だけで共有するものは `problems/<グループ>/<名前>/common.hpp` のように `submissions/` の外に置きます。問題のディレクトリが `-I` に入っているので、提出からは `#include "common.hpp"` で引けます。`submissions/` の中に置くなら先頭を `_` にします (`submissions/_impl.hpp`)。問題をまたいで使うものは `problems/_shared/<名前>/` に置き、`-Iproblems` が入っているので `#include "_shared/gf2-64/_common.hpp"` の形でどの問題からも同じ書き方で引けます。`common.hpp` という名前自体に意味はなく、慣習です。

提出のパスはそのまま識別子です。順位表と提出ページとライブラリ側のサイトからのリンクがこのパスを指すので、リネームすると別の提出として測り直しになります。

x64 の環境は、土台の命令をコンパイラのオプションで渡します。x86-64-v3 の命令 (AVX2、BMI2、FMA、popcnt など) に pclmul と vpclmulqdq を足したもので、フラグは `-march=x86-64-v3 -mpclmul -mvpclmulqdq` です。土台の命令は、提出で宣言しなくても使えます。判定サイトへ出すときは、algo-workspace のバンドラが、同じ一覧の `#pragma GCC target` を提出の先頭に入れます。GCC 13 と 14 のバグを避けるため、その前に `<bits/allocator.h>` だけを読みます。一覧と理由は DESIGN.md の「土台の命令をオプションに戻しました」にあります。

vpclmulqdq を使うコードは、`__builtin_cpu_supports("vpclmulqdq")` で実行時に分け、持たない CPU 向けの経路も書いてください。Codeforces の判定機が vpclmulqdq を持たないためです。vpclmulqdq の intrinsics はすべて分岐の内側に置き、分岐はループの外に置いて判定を 1 回で済ませます。GitHub の x64 ランナーはどれも vpclmulqdq を持つので、持たない側の経路は procon-judge では測られません。

土台に無い命令 (AVX-512 や GFNI) を使う提出は、使う命令をソースで宣言してください。宣言しないと CE になるか、遅い経路で測られます。宣言した命令は土台に足されます。これらの提出は、その命令を持つ CPU でだけ走ります。x64 のランナーの EPYC 7763 はどちらも持たないので、そこに当たると RE になります。

関数ごとに宣言するなら、`[[gnu::target("avx512f")]]` を関数に付けます。GCC でも clang でも効きます。小さい関数に付けるときは、呼び出す側のループにも同じものを付けてください。付けないとインライン展開されず、1 回ずつの呼び出しになります。

ファイルごとに宣言するなら、`#pragma GCC target("avx512f")` を書きます。ただしこれは GCC にしか効きません。clang 向けには `#pragma clang attribute push(__attribute__((target("avx512f"))), apply_to = function)` を並べます。push は同じ翻訳単位の中で pop します。Library Checker の写しの先頭と末尾に例があります。clang の push は `__AVX512F__` などのマクロを立てないので、マクロで経路を分けるコードは、x86 かどうかを `__x86_64__` で見る形にします。

ハーネスは提出より先に標準ライブラリを読むので、提出で宣言した命令は標準ライブラリの関数には付きません。std::sort に渡したラムダがソートの中へ展開されないなど、判定サイトで先頭に宣言した場合より遅く測られることがあります。

gf2-64 の家族の提出は、土台の命令を宣言しません。土台に無い命令 (gfni など) を使う提出だけが、`#define GF2_64_EXTRA_TARGETS "gfni"` のように書きます。書く場所は、共通ヘッダ `_shared/gf2-64/_common.hpp` を初めて include するより前です。共通ヘッダがその命令の領域を開き、各問題の base.cpp の最後で閉じます。include したあとで `#pragma clang attribute push` を重ねても、clang は一番外側の target だけを使うので効きません。

### 5. 手元で確かめる

```
uv run pj problems check --problem <id>
uv run pj run --env local --problem <id> --dry-run
uv run pj run --env local --problem <id>
uv run pj repro --problem <id> --submission submissions/<name>.hpp --case <ケース名>
uv run pj repro --problem <id> --submission submissions/<name>.hpp --cases 3
```

`--dry-run` は走らせる対象を出すだけです。`pj run` は記録を `.results/` に書くので、別の場所に書きたければ `--out` を渡します。`pj repro` はテストデータを取って手元のコンパイラで組み、そのケースだけ走らせて、完全な差分と入力、期待出力、実際の出力のファイルの場所を出します。記録は書きません。`--case` の代わりに `--cases N` を付けると、ケースを名前順に並べた先頭から N ケースだけを走らせます。CI を待つ前に、組めていくつかのケースが通るかを手早く見るときに使います。テストデータは問題ごとにまとめて取るので、取る量は変わりません。

macOS の `local` は Apple clang と libc++ なので、CI の 4 環境 (どれも libstdc++) と結果が同じとは限りません。`std::__lg` を使う提出 (Library Checker の写しなど) は、手元では CE になります。

サイトを手元で見るなら `uv run pj site build --out /tmp/pj-site` で作り、そのディレクトリを `python3 -m http.server` などで開きます。

### 6. push すると起きること

main に push すると `judge.yml` が動きます。`test` が `pj` 自身の pytest を回し、`plan` が組 (x64 と arm) ごとにジョブの本数を決めます。`run` のジョブはそれぞれ CPU モデルを検出して、問題を 1 つずつ宣言しては測ります。`collect` が記録を `results` ブランチに追記してサイトを Pages に出します。同じ問題を同じ CPU モデルで二重に測ることはありません。

run にはモードが 2 つあります。push と Library の dispatch は網羅モードで、(問題, 環境) ごとに今の全提出が現行になっている CPU モデルが 1 つでもあれば飛ばし、無ければ宣言を取ったジョブが乗ったモデルで測ります。環境ごとに 1 モデルだけが新しくなるので、1 問の push なら組ごとに 1 本のジョブで、5 分から 10 分で順位表に出ます。他のモデルは 1 日 2 回の schedule (全モデルモード) が埋めます。順位表の CPU モデルの既定は揃っているものになり、揃っていないモデルには選択肢に欠けの数が付きます。

測り直しになるのは、ソース (提出とその include 閉包、ハーネス、problem.toml) とテストデータと環境と CPU モデルから作るキーの記録が無いものだけです。整形やコメントの変更ではキーが変わりません。問題のディレクトリを `problems/` の下で動かしてもキーは変わりません。`base` の問題は (問題, 環境, CPU モデル) を束として扱い、1 本でも未計測なら全提出を同じジョブで測り直します。同じ CPU モデルでも VM ごとに速さが 2 割ほど違うので、順位表は同じジョブで測った記録どうしでだけ比べます。提出を 1 本足すと、その問題の他の提出も測り直されるのはこのためです。`raw` の問題はキーごとに測ります。

ライブラリ (hashiryo/Library) の master への push もこちらを起こし、変わったヘッダを閉包に持つ提出だけが測り直されます。取りこぼしは 1 日 2 回の schedule が拾います。

結果はサイトの問題一覧と順位表、提出ページに出ます。順位表は環境と CPU モデルを選んで見ます。参考の印が付いた行は測ってからソースが変わったもので、次の計測で入れ替わります。

## 既存の問題に提出を足す

問題のディレクトリの `submissions/` にファイルを置いて push するだけです。`base` の問題なら、`base.cpp` が呼ぶ型と関数を実装します。手元で `uv run pj run --env local --problem <id> --submission submissions/<name>.hpp` と 1 本だけ走らせて確かめられます。1 本だけ指定したときは束を作らないので、手元の記録が順位表を乱すことはありません。

## 問題を消す

問題のディレクトリを消しても、`results` ブランチの記録は残ります。サイトは記録のある問題も一覧に出すので、ディレクトリを消しただけでは、その問題が判定の無いまま残ります。サイトから消すときは、`results` ブランチの `problems/<id>.jsonl` も手で消して push します。

```
git clone --depth 1 --branch results "$(git remote get-url origin)" /tmp/results
cd /tmp/results
git rm problems/<id>.jsonl
git commit -m "<id> の記録を消す"
git push origin results
```

サイトに反映されるのは、次に collect が走ったとき (main への push か schedule) です。collect の push と重なって弾かれたら、pull し直してから押します。問題の id を変えたときも、古い id の記録は同じように残ります。提出を消すだけなら、サイトは消えた提出の記録を隠すので、`results` に触る必要はありません。

## コマンド一覧

| コマンド | 何をするか |
| --- | --- |
| `pj problems list` | 問題の一覧。`--json` で機械向けの形 |
| `pj problems check [--problem ID]` | problem.toml の検証 |
| `pj problems titles [--problem ID] [--fix]` | 題名を判定サイトの名前と突き合わせる |
| `pj submissions list [--problem ID]` | 提出の一覧 |
| `pj fetch --problem ID [--refresh] [--from-origin]` | テストデータを取る。`--refresh` はキャッシュを無視し、`--from-origin` は保管庫も飛ばして原本から取る |
| `pj testdata import --problem ID --dir PATH` | 手元で落としたテストデータを取り込む |
| `pj mirror status [--problem ID]` | 保管庫の状態 |
| `pj mirror push --problem ID [--force]` | 保管庫へ上げる。`--force` は取り直したデータに入れ替えるとき用 |
| `pj run --env ENV [--problem ID] [--submission PATH] [--dry-run] [--out DIR]` | 実行して記録を出す |
| `pj repro --problem ID --submission PATH [--case NAME]` | 1 提出を手元で走らせて差分を見る |
| `pj records list` | 記録の一覧 |
| `pj records append DIR...` | ほかの jsonl を記録に取り込む |
| `pj site build [--out DIR] [--store DIR]` | 記録からサイトを作る |
| `pj plan` | CI の matrix を出す |
| `pj problems import PATH...` | competitive-verifier 形式の test を raw の問題として取り込む。移行の道具 |

`uv run pj <コマンド> --help` で引数が出ます。
