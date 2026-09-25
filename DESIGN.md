# procon-judge 設計

## これは何か

競技プログラミング用の自作ジャッジシステムの設計です。新しいリポジトリをゼロから作る前提で書いています。実装を担当するセッションは、このファイルを新リポジトリの `DESIGN.md` としてコピーしてから作業を始めてください。

リポジトリ名は `procon-judge`、CLI 名は `pj` とします。名前は後から変えられるので、気に入らなければ置き換えてかまいません。

既存の `hashiryo/judge` と `hashiryo/Library` は参照元として残します。消しません。

## 始め方

1. `/algo/procon-judge` を作って `git init` します。GitHub 側も public で作ります。
2. このファイルを `DESIGN.md` としてコピーします。
3. 「最小の第一版」と「実装の順番」を先に読んでください。残りは必要になったところで引きます。
4. M1 から始めます。

参照元のリポジトリは `/algo` の下に並んでいます。`Library`、`judge`、`CPtools`、`algo-workspace`、`my-docs` です。本文でこれらのパスを挙げているところは、この並びを指しています。

実装しながら決めたことは `DESIGN.md` に書き戻してください。ここが真実の源でないと、すぐに古くなります。

### GitHub 側の用意

**`gh` の書き込み系を使わないでください。** `gh` は `ryomaHashimoto` で認証されていて `hashiryo` ではありません。`gh repo create` を実行すると別アカウントの下に作られます。

GitHub 側の用意はブラウザで人間がやります。必要になる順です。

| いつ | 何を |
| --- | --- |
| M3 の前 | `procon-judge` を public で作ります |
| M4 の前 | `procon-judge-testdata` を private で作り、`testdata` タグのリリースを 1 本置きます |
| M4 の前 | fine-grained PAT を発行し、`procon-judge` の secret に `TESTDATA_TOKEN` として入れます |
| M6 の前 | Pages の source を GitHub Actions にします |

M1 と M2 はローカルだけで済むので、GitHub は要りません。先に動くものを作ってから用意して大丈夫です。

push は SSH で通ります。remote は `git@github.com.hashiryo:hashiryo/procon-judge.git` です。`~/.ssh/config.d/` に alias があり、`hashiryo_id_rsa` を使う設定になっています。

CI のログは読み取りで引けます。`gh run list -R hashiryo/procon-judge` と `gh run view --log` です。public なら別アカウントの認証でも読めることを実測しました。読み取りだけに使ってください。

## 何を作るのか

自分用のオンラインジャッジです。AtCoder の提出ページで見られるものを自分の手元に持つのが目的です。正誤、実行時間、使用メモリ、コード長、ソース、同じ問題の他の提出との順位。

AtCoder に無い機能を 1 つだけ足します。提出が動くソースを追いかけることです。AtCoder の提出は不変なので、ライブラリを更新すると貼ったリンクが古い実装を指したままになります。この判定システムでは、提出はリポジトリ内のファイルなので、ライブラリが変わると新しい採点が走って記録が増えます。URL は固定で、中身は最新を指し続けます。

このリポジトリはライブラリという概念を持ちません。`submissions/` に置いてあるソースを走らせて記録を出すだけです。ライブラリとの紐付けは、記録に入れた include の一覧を使って、ライブラリ側のサイトが自分で探します。

## 3 つの実体

| 実体 | 何か | どこにあるか |
| --- | --- | --- |
| 問題 | 入出力の形式、制限、テストデータの取得元 | `problems/<id>/` |
| 提出 | ある問題に対するソースファイル | `problems/<id>/submissions/*` |
| 採点 | ある提出をある条件で走らせた結果 | `results` ブランチ |

問題と提出はリポジトリの中身です。ある SHA のリポジトリを見れば決まるので、別に保存しません。蓄積して保存が必要なのは採点だけです。

## リポジトリ構成

```
procon-judge/
  pyproject.toml            uv で管理。CLI 名は pj
  environments.toml         コンパイラとフラグの定義
  pj/                       実装本体 (Python)
    __init__.py
    cli.py                  コマンドの入口
    problem.py              problem.toml の読み込みと検証
    key.py                  キーの計算
    include.py              include 閉包の解決
    fetch/                  テストデータ取得
      library_checker.py
      aoj.py
      yukicoder.py
      manual.py
      local.py
      mirror.py             保管庫の読み書き
    build.py                コンパイル
    execute.py              実行と計測
    compare.py              出力比較
    bundle.py               提出を 1 ファイルに展開する
    record.py               採点レコード
    store.py                記録の読み書き
    site/                   サイト生成
      build.py
      templates/
  problems/
    <グループ>/<名前>/     問題。problems/ からの各段を - で繋いだものが id (atcoder/abc172-d なら atcoder-abc172-d)。1 段で置けば名前がそのまま id
      problem.toml
      base.cpp              ハーネス (kind = "base" のとき)
      common.hpp            その問題だけが共通で使うもの (省略可)
      submissions/
        lib.hpp
        naive.hpp
        dirty_simd.hpp
      gen.py                ジェネレータ (source = "local" のとき)
      reference.hpp         期待出力を作る参照実装 (同上)
      checker.cpp           独自チェッカ (自分で書くときだけ)
    _shared/<名前>/         問題をまたいで提出が使うヘッダ (gf2-64 のベンチ、modulo-test の族)。problem.toml が無いので問題ではない。-Iproblems が入っているので #include "_shared/gf2-64/_common.hpp" で引く
  harness/
    pj.hpp                  全問題のハーネスと提出が共有するもの
  third_party/
    simde/                  submodule
  tests/                    pj 自身のテスト (pytest)
  .github/workflows/
    judge.yml
  .cache/                   テストデータのキャッシュ。git 管理外
```

テストデータは git に置きません。リポジトリに入るのは、問題の定義、ハーネス、提出、ジェネレータ、チェッカだけです。テストデータの用意は 2 通りです。その場で生成するか、外から落とすかです。

`local` の問題では `gen.py` と参照実装が git に入ります。これはデータではなくコードです。

## 問題の定義

### problem.toml

```toml
id = "yosupo-point-add-range-sum"
title = "Point Add Range Sum"

[limits]
tle_sec = 5.0
mle_mb = 256

[harness]
kind = "base"          # "base" または "raw"

[testdata]
source = "library_checker"
name = "data_structure/point_add_range_sum"

[compare]
kind = "checker"
```

`id` は置き場所から決まる名前と一致させます。`problems/` の下は何段でも掘れて、そこからの各段を `-` で繋いだものが id です (`problems/atcoder/abc172-d/` なら `atcoder-abc172-d`、`problems/gf2-64/pow/` なら `gf2-64-pow`)。1 段で置けばディレクトリ名がそのまま id です。検証で一致しなければエラーにし、同じ id が 2 か所から出てもエラーにします。判定サイトの問題は接頭辞のディレクトリ (`atcoder/`、`aoj/`、`yosupo/`、`yuki/`、`loj/` など) に、自作の問題は族のディレクトリ (`gf2-64/`、`modulo-test/` など) に置いてあります。族を持たない自作の問題は 1 段で置きます (2026-09-23 に平らな置き方から移しました。「問題ディレクトリの階層化とキーの -I の正規化の記録」)。

`title` は表示にだけ使い、鍵には入りません。判定サイトから取っている問題は、`pj problems titles --fix` で判定サイトの名前に揃えます。手で書くと違う名前を付けてしまいます。移植のとき AOJ の 36 問のうち 24 問と yukicoder の 3 問がそうなっていて、AOJ の旧 API が消えていたので気づくのが遅れました。AOJ は新しいサイトの一覧の API から、yukicoder は問題の API から、Library Checker は `info.toml` から取ります。AtCoder はページから取りますが、Library の test の URL も 12 件が間違っていて 404 でした。間違いは 3 種類です。1 つ目はコンテストの slug の `_` と `-` の違いです (s8pc-1、cf17-final、nikkei2019-2-qual)。2 つ目は Good Bye rng_58 Day 2 が agc051 であることです。3 つ目は 2016 年から 2018 年の ARC で、問題 C から F の id が `a` から `d` になっていることです (arc060 の F は `arc060_d`)。test の実装 (使うヘッダ) と問題の内容を照らして直しました。id は URL の名前から作るので、旧 ARC の 6 問は `atcoder-arc060-d` のように改名しています。
`local` や `manual` や `none` の問題には判定サイトの名前が無いので、手で付けます。

id は `<出どころ>-<問題>` の形にします。出どころはテストデータの取得元ではなく、問題そのものがどこの問題かです。AtCoder の問題は `source = "none"` になりますが、id は `atcoder-` で始めます。

| 出どころ | 接頭辞 | 例 |
| --- | --- | --- |
| Library Checker | `yosupo-` | `yosupo-point-add-range-sum` |
| AOJ | `aoj-` | `aoj-DSL_2_B` |
| yukicoder | `yuki-` | `yuki-1234` |
| AtCoder | `atcoder-` | `atcoder-abc123-d` |
| LOJ | `loj-` | `loj-6620` |
| HackerRank | `hackerrank-` | `hackerrank-cube-loving-numbers` |
| CSES | `cses-` | `cses-2132` |
| JOI 春合宿 | `joisc-` | `joisc-2019-examination` |
| Codeforces | `cf-` | `cf-1140-f`、`cf-gym102576-c` |
| oj.uz | `ojuz-` | `ojuz-APIO16_fireworks` |
| CodeChef | `codechef-` | `codechef-CCDSAP` |
| Kattis | `kattis-` | `kattis-conquertheworld` |
| Luogu | `luogu-` | `luogu-P5055` |
| 自作 | 付けません | `gf2-64`、`warshall-floyd`、`constexpr-modint` |

`pj problems import` はこの表のとおりに URL から id を作ります。yosupo と atcoder は URL の名前の `_` を `-` に、AOJ と yukicoder は判定サイトの id をそのまま使います。入出力を持たない自己検証のテスト (`STANDALONE`) は、普通のコメントに書いてある元の問題の URL から id を取り、無ければファイル名をそのまま id にします。

`url` は元の問題のページで、表示にだけ使います。判定サイトから取る問題は `source` と `name` から組めるので書きません。書くのは `source = "none"` (AtCoder、自己検証) と `source = "manual"` の問題だけです。AtCoder の題名は API が無いのでこの URL のページから取ります。

`pj problems check` は `testdata.source` が `library_checker`、`aoj`、`yukicoder`、`loj` のときに接頭辞が合っているかを見て、合わなければ警告します。判定サイトから取るテストデータはその出どころの問題にしか付かないからです。エラーにはしません。逆向きは縛りません。`aoj-` の問題のテストデータを `local` や `manual` で持つのは正しい形です。

id は保管庫のアセット名にもなります。アセット名は `<問題 id>.tar.zst` で、対応表を持たずに計算しているからです。あとから id を変えると、保管庫のアセットが孤児になり、キーも変わって記録が全部測り直しになります。付けるときに決めてください。

### ハーネスの種別

`kind = "base"` が既定です。問題が `base.cpp` を持ち、提出は問題が決めたインターフェースを実装します。入出力はハーネスが担当し、計測区間を挟みます。

```cpp
// problems/yosupo-point-add-range-sum/base.cpp
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  scanf("%d %d", &n, &q);
  vector<long long> a(n);
  for (auto &x : a) scanf("%lld", &x);
  vector<array<long long, 3>> qs(q);
  for (auto &e : qs) scanf("%lld %lld %lld", &e[0], &e[1], &e[2]);

  Solver s(a);                     // 提出が実装する型
  string out;
  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.add(e[1], e[2]);
    else out += to_string(s.sum(e[1], e[2])) + "\n";
  }
  auto t1 = chrono::steady_clock::now();

  fputs(out.c_str(), stdout);
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
```

要点を順に書きます。

インターフェースは問題ごとに決めます。共通の signature を 1 つ決めるのは無理なので、各問題の `base.cpp` が期待する型と関数を宣言します。形は自由で、構造体と自由関数のどちらでもかまいません。クエリを順に処理する問題なら、状態のある型が自然です。1 回の計算で答えが出る問題なら関数 1 本で足ります。

```cpp
// 1 回の計算で答えが出る問題の例。
// 提出側に vector<mint> solve(const vector<mint>&, const vector<mint>&) を実装させる。
  auto t0 = chrono::steady_clock::now();
  auto c = solve(a, b);
  auto t1 = chrono::steady_clock::now();
```

形よりも、計測区間の中身の方が本質です。入力の解析と出力の整形は区間の外に置きます。区間の中には比べたい処理だけを残してください。クエリ問題で入力を全部先読みするのは、この制約から来ています。

構築の費用を区間へ入れるかは問題ごとに決めます。UnionFind の構築のように無視できるものは外に置きます。接尾辞配列のように構築そのものが比較対象になる場合は中へ入れます。

構築に何を渡すかも同じ基準です。`yuki-649` では、これから追加される値の一覧を構築へ渡していません。渡すと座標圧縮を計測区間の外でできますが、その前処理はクエリ列と同じ O(Q log Q) なので、外に出すと圧縮を使う実装だけが有利になります。この問題のインターフェースはオンラインの実装だけを相手にしています。

特定の実装だけが有利になるインターフェースは避けてください。比較のための土台です。ある方針だけが余計なコピーや変換を強いられる形だと、その分まで測定に乗ります。

入力は全部読んでから計測区間に入ります。出力も文字列に溜めてから最後に書きます。こうすると入出力の時間が計測から外れます。

計測値は stderr に 1 行の JSON で出します。`PJ_METRICS ` という接頭辞を付けて、実行側はその接頭辞で始まる行だけを拾います。stdout に混ぜると出力比較が壊れるので、stdout は使いません。

接頭辞で絞る形にしておくと、他の出力が stderr に混ざっても平気です。サニタイザを入れた環境や、ライブラリが警告を吐く場合に効きます。JSON にしておくと、項目を足すときも JSON のキーが増えるだけで済みます。

専用のファイルに出す方式は採りません。パスを環境変数で渡す必要があり、渡されなかったときの退避も要ります。ケースごとにファイルを作って読んで消す手間も増えます。stderr は既定でバッファされないので、あとで異常終了しても行が残ります。

メモリもハーネスが測って `max_rss_kb` として返します。実行側の `ru_maxrss` は使えません。`posix_spawn` した子のそれには親のピーク RSS が混ざるからです。カーネルが exec のときに古い mm の high-water を引き継ぐ仕組みで、Linux では `pj` 自身の RSS がそのまま下駄になります。`pj` が直前に何をしたかで下駄の高さが変わるので、記録どうしを比べられません。

Linux では `/proc/self/status` の VmHWM を読みます。exec 後の mm だけを見るので汚れません。macOS の `posix_spawn` はアドレス空間を共有しないので `getrusage(RUSAGE_SELF)` で足ります。どちらも `harness/pj.hpp` の `peak_rss_kb()` にあります。出力の整形まで含めたピークを読みたいので、`report_metrics` は `main` の末尾で呼びます。

打ち切られた実行はハーネスが報告できないので、そのときだけ `ru_maxrss` に落ちます。TLE と MLE の行のメモリには下駄が乗ったままです。

問題をまたいで同じものは `harness/pj.hpp` に置きます。include 一式、`i64` などの型別名、`must_scan`、`peak_rss_kb`、`report_metrics` です。`-Iharness` が入っているので `#include "pj.hpp"` で引けます。問題ごとの `common.hpp` は、その問題だけが共通で使うものを置く場所として残してあります。今の 3 問には該当するものが無いので、どれも持っていません。

探索の順は `lib`、問題のディレクトリ、`harness`、`problems`、`third_party/simde` です。問題のディレクトリが `harness` より先なので、問題ごとに同じ名前のヘッダを置けばそちらが勝ちます。`problems` は問題をまたぐヘッダ (`problems/_shared/`) を `#include "_shared/gf2-64/_common.hpp"` の形で引くためのもので、問題を何段掘って置いても同じ書き方で通ります。

既存の `judge` がこれを踏んでいないのは、`/usr/bin/time -v` 越しに走らせているからです。fork する親が 1 MB ほどの C プログラムなので下駄が見えません。24000 件の記録の最小が 3 MB 台で、それがその高さです。

判定サイトに貼っても影響しません。AtCoder、Library Checker、AOJ、yukicoder のいずれも stderr を判定に使いません。

メモリは実行側が外から測ります。`base.cpp` の中で測ろうとしないでください。

内側からでも `getrusage(RUSAGE_SELF)` の `ru_maxrss` で取れますが、プロセスが死んだケースで値が失われます。外から `wait4(pid, &status, 0, &ru)` を呼べば、子が異常終了したあとでもピーク RSS が返ります。OOM による強制終了や segfault のあとでも取れます。MLE や RE のときにメモリが分かるのはこちらだけです。`raw` の提出でも扱いが揃うという利点もあります。

`ru_maxrss` の単位はプラットフォームで違います。**Linux は KB、macOS はバイト**です。手元が Mac で CI が Linux なので、変換をプラットフォームごとに分けてください。

RSS には実行ファイルと libc の共有ページが入るので、何もしないプログラムでも数 MB の下駄を履きます。既存の judge のレコードで素の naive が 3792 KB になっているのがそれです。`mle_mb` をきつく設定しないでください。

メモリ制限は強制しません。`wait4` で取ったピークが `mle_mb` を超えていたら MLE にする後判定で足ります。`setrlimit(RLIMIT_AS)` は触っていない予約まで引っかかるのでアロケータと相性が悪く、cgroup v2 はランナー上で扱う手間が大きいからです。暴走した確保が怖い場合は、`mle_mb` よりずっと大きい値で `RLIMIT_AS` を保険に入れてください。

既存の判定サイトのテストデータがそのまま使えます。ハーネスが判定サイトの入力形式で読んで出力形式で書くので、テストデータを加工する必要がありません。この形を選んだ一番の理由です。

`kind = "raw"` は、実装が 1 本しかない verify の置き場です。提出が `main()` ごと持ち、判定サイトの入出力をそのまま読み書きします。比べる相手がいないのでハーネスを書いても速さの情報は出ず、正しさと「この環境でも通る」だけを持ちます。2 本目の実装が来た問題だけをハーネスに書き直します。計測区間が無いので `algo_time_ns` は記録されず、プロセス全体の時間だけ取ります。入出力を持たない自己検証の提出 (`compare.kind = "exit_code"`) もこの形です。Library の `test/**/*.test.cpp` は `pj problems import` で機械的にこの形になります。

対話式の問題は raw でも動きません。実行はファイルから stdin を流すだけで、判定側と会話する配管が無いからです。オンラインクエリの問題は base で扱えます (`yuki-649` がその例)。以前はこの 2 つを raw の用途に挙げていましたが、どちらも成り立っていませんでした。

入力を全部メモリに読むので、メモリ使用量は元の問題の想定と変わります。`mle_mb` は「このハーネスでの制限」として扱ってください。元の判定サイトの制限と一致させる意味はありません。

### インターフェースの 2 つの形

形は問題ごとに決めますが、実際に出てくるのは 2 つです。移植で同じ判断を何度もするので、どちらかに寄せます。

クエリを順に処理する問題では、構築に初期状態だけを渡して、計測区間の中でクエリを 1 つずつ渡します。

```cpp
struct Solver {
  explicit Solver(const vector<i64> &a);
  void add(int p, i64 x);
  i64 sum(int l, int r);
};
```

先の入力を構築へ渡さないのが要点です。渡すと、オフラインでできる前処理を計測区間の外に置けてしまい、その手が使える実装だけが有利になります。

1 回の計算で答えが出る問題では、構築と計算と取り出しの 3 段に分けます。

```cpp
struct Solver {
  explicit Solver(const string &s);
  void run();
  const vector<int> &answer() const;
};
```

`solve()` 1 本で返すより段が 1 つ多いですが、内部表現が入出力と違う問題では変換が計測区間の外に出ます。O(N) の変換を O(N log N) の計算と一緒に測ると、実装がどの表現を選んだかで乗り方が変わります。

クエリを全部読んでから解く構造 (kd 木や 2 次元セグメント木のように、点の集合を先に固めてから更新と問い合わせを受けるもの) は、後ろの形に寄せます。クエリの列ごと構築に渡して `run()` の中で全部処理し、答えの列を `answer()` で返します。クエリを 1 つずつ渡す形にすると、先の入力を知らない実装が構造を作れないからです。`yuki-1216` と `yuki-1625` がこの形です。

### ハーネスはライブラリを include しません

`base.cpp` と、そこから辿るヘッダは `mylib/...` を include しません。理由が 2 つあります。

ライブラリを取れなかった回に、その問題の提出が全部落ちます。閉包に未解決の include がある提出は走らせない決まりなので、ハーネスが依存していると、提出の側の事情と関係なく問題ごと止まります。

`harness_hash` にライブラリが入ります。`ModInt.hpp` を直しただけで、それを使っていない手書きの実装まで測り直しになります。提出の側で include していれば、動くのはその提出の `submission_hash` だけです。

そのため、剰余を扱う問題でも入出力は素の整数でやりとりして、提出が自分の内部表現へ直します。変換はクエリあたり O(1) か、1 回の計算の形なら計測区間の外なので、比較は歪みません。

問題ごとの `common.hpp` はライブラリを include してかまいません。提出しか読まないので、上の 2 つはどちらも起きません。同じ問題に載せる実装が同じ作用素を使うときは、ここに 1 つ置いて揃えます。揃えないと、比べたいのはコンテナなのに、作用素を `array` で持つか `pair` で持つかの違いまで測定に乗ります。

### harness/pj.hpp が持つもの

include 一式と `i64` などの型別名のほかに、次の 5 つがあります。

| | 何をするか |
| --- | --- |
| `must_scan(got, want)` | `scanf` の戻り値を見て、違っていたら落とします |
| `read_token()` | 空白区切りの次のトークンを読みます。長さの分からない文字列用です |
| `read_ints(n)` | 整数を n 個読みます |
| `print_all(v, sep)` | 数の列を書きます。既定は 1 行 1 個です |
| `report_metrics(ns)` | 計測値を stderr へ出します。`main` の末尾で呼びます |

ここを変えると全問題の `harness_hash` が変わって、記録が全部測り直しになります。足すなら問題が少ないうちにしてください。

### テストデータの取得元

| source | 説明 | 追加の項目 |
| --- | --- | --- |
| `library_checker` | `yosupo06/library-checker-problems` を `testdata.toml` のコミットで clone して生成します | `name` に問題のパス |
| `aoj` | judgedat の API から取得します | `name` に問題 ID |
| `yukicoder` | API から取得します。トークンが必要です | `name` に問題 ID |
| `loj` | API から取得します。ログインは要りません | `name` に問題の番号 |
| `manual` | 保管庫と手動の取り込みだけを使います。原本を叩きません | `name` に識別子 |
| `local` | リポジトリ内で生成します | `generator`、`count`、`reference` |
| `none` | テストデータを使いません | なし |

`library_checker` の clone は `testdata.toml` に書いたコミットに固定します。master の HEAD を追うと、上流がジェネレータを直したときにケースの中身が変わり、`cases_hash` が動いてその問題の記録が黙って測り直しになります。ジェネレータは seed 固定なので、同じコミットなら同じバイト列が出ます (`partition_function` の in と out を消して作り直し、同じ `cases_hash` が出ることを確かめました)。pin を動かすのは意図的な操作で、そのときも中身が変わった問題だけが測り直しになります。GitHub はコミットを名指しで fetch できるので、履歴は 1 段しか取りません (2.7 MB、1 秒ほど)。

`local` の書き方です。

```toml
[testdata]
source = "local"
generator = "gen.py"      # 既定値
count = 200
reference = "reference.hpp"   # kind = "base" なら提出と同じ形
```

`gen.py` が seed を引数に取ってランダムな入力を stdout に出します。参照実装が期待出力を作ります。`count` 個の seed を 0 から順に使うので、生成は決定的になります。

`local` は `name` を持ちません。キャッシュのハッシュは問題 id から作ります。加えて `gen.py` の内容、参照実装の内容、`count` を混ぜてください。`kind = "base"` なら `base.cpp` の内容も混ぜます。参照実装はハーネスと一緒に組むので、ハーネスの定数を直せば期待出力も変わるからです。ジェネレータやハーネスを直したときに、古い生成結果が使い回されるのを防ぐためです。

`kind = "base"` の問題では、参照実装も提出と同じ形にしてください。`reference` に `.hpp` を指定します。ハーネスと一緒にコンパイルして期待出力を作ります。こうすると入力の解析を二重に書かなくて済みます。`kind = "raw"` の問題なら、参照実装は単体で動く `.cpp` になります。参照実装を `submissions/reference.hpp` に置いて `reference` にそのパスを書けば、期待出力を作るものがそのまま順位表の 1 行になります (`gf2-64-*` はこの形)。

参照実装はそのジョブの環境のコンパイラで組みます (`pj fetch` は `--env`、既定は `local`)。期待出力は決定的な計算なのでコンパイラに依らず、キャッシュは環境をまたいで共有します。参照実装は素朴に書いてよいので、実行の打ち切りは提出の `tle_sec` ではなく 600 秒です。ケースの名前は `seed_000` からの連番です。

`none` はテストデータを持たない問題です。使い方が 2 通りあります。自己検証の提出なら `compare.kind = "exit_code"` にして、空の標準入力で走らせて終了コードを見ます。入力を読む提出なら `compare.kind = "compile_only"` にします。実行せず、コンパイルだけを見ます。

AtCoder は 2024 年 12 月からテストケースの公開が停止しています。告知は https://atcoder.jp/posts/1376 です。再開の見通しは無いので、AtCoder の問題は `source = "none"`、`harness.kind = "raw"`、`compare.kind = "compile_only"` にします。実行はしません。入力が無いので、走らせても落ちるだけです。

取得したテストデータは `.cache/testcases/<source>/<name>/` に置きます。取得する前に引ける必要があるので、置き場は取得元と問題の名前だけで決めます。中身は関係しません。取り直して別の内容になっても、同じ場所に上書きされます。内容から作るのは `cases_hash` の方で、こちらはキーの材料です。この 2 つを混ぜないでください。置き場が変わってもキーは動きませんし、置き場が同じでもキーは動きえます。`local` だけは、同じ問題 id からジェネレータ次第で別の内容が出るので、ジェネレータと参照実装の中身でさらに分けます。

置き場を読める形にしてあるのは、ログに出るからです。ハッシュにすると `cases_hash` と見分けがつかず、内容から決まっていると読み違えます。

CI では持ち越しません。走らせる問題のぶんだけ保管庫から落とし、その問題を測り終えたら捨てます (`pj run --evict-testdata`)。ランナーの disk は 14 GB ほどしか無く、1 ジョブで数十問を測るからです。以前は全問題ぶんを actions/cache に 1 つの塊で置いていましたが、Library Checker の問題を 128 問入れると 10 GB を超えて載らず、1 問しか要らない回でも塊ごと復元することになるのでやめました。手元のキャッシュはそのまま残ります。

取得の実装は既存のコードを移植してください。判定サイトごとの癖が入っているので、書き直すと同じ罠を踏みます。参照するファイルは次のとおりです。

```
hashiryo/Library     scripts/lib/download.py
hashiryo/judge       scripts/download-testcases.py
hashiryo/CPtools     loj_download.py
                     url_hash.md
                     yukicoder-spjudge-guide.md
```

LOJ は API から取れます。`getProblem` が `testData` (ファイルの一覧) と `judgeInfo` (制限と、サブタスクごとの in/out の組) を返し、`downloadProblemFiles` がファイルごとの署名付き URL を返します。ログインは要りません。`CPtools/loj_download.py` は提出の結果からファイル名を集めていたので、当初は「自分が提出していないと取れない」と読み違えて `manual` にしていました (2026-09-22 に `loj` へ直しました。「LOJ の取得元の実装の記録」)。IOI (JOI 春合宿)、CSES、Codeforces、HackerRank は実装がありません。これらは `source = "manual"` にします。Library の Release (`v0.0.0`) に付いている `tc.zip` は、verify が oj で取れないサイトのために以前に手で落としたデータの束で、`tc/<URL の md5>/test/` に `.in` / `.out` が並んでいます (URL の一覧は CPtools の `url.csv`)。HackerRank 10 問、CSES 1 問、Codeforces 2 問、UTPC 2 問はそこから取り込みました (「Library の tc.zip からの取り込みの記録」)。

AOJ の judgedat は大きいケースを切り詰めて返すことがあります。取得したあとに header のケース数とサイズを突き合わせてください。合わなければ警告にします。切り詰められたデータで判定すると、結果の意味が変わります。

### テストケースの形

取得元が何であれ、キャッシュの中は同じ形にします。平らな 1 ディレクトリに `<ケース名>.in` と `<ケース名>.out` を並べます。`in/` と `out/` のディレクトリは作りません。

```
.cache/testcases/library_checker/data_structure/point_add_range_sum/
  00_small_00.in
  00_small_00.out
  04_maximum_03.in
  04_maximum_03.out
  manifest.json
```

組にするのは拡張子を除いた部分が一致するかどうかだけです。対になる `.out` が無い `.in` は無視します。

取得元の形はここへ揃えます。Library Checker は `in/` と `out/` に分けて出すので平らにします。yukicoder の zip は `test_in/` と `test_out/` なので同じです。AOJ は serial 番号で来るので、header の name をケース名に使います。

手で取り込むときも同じ形にしてから渡します。1 つのディレクトリに `.in` と `.out` を名前をそろえて並べて、`pj testdata import --problem ID --dir PATH` です。

ケース名は `cases_hash` に入ります。あとから名前を変えると `cases_hash` が動いて、キーが変わって、その問題の記録が全部測り直しになります。取り込む時点で決めてください。

### テストデータの保管

リアルタイムで取得できない取得元があります。取得できるものでも、毎回原本を叩くのは遅くなります。レート制限と障害が経路に入り込みます。だから一度落としたテストデータは自分の置き場に保管します。

生成し直せる `library_checker` も保管します。1 問 100 MB 前後で 128 問なら 10 GB を超え、再生成は 1 問 10 秒から 1 分かかります。再現性は pin が持ち、速さと問題ごとの粒度は保管庫が持つ、という分担です。保管庫に入れると中身が凍るという心配は pin が解きます。アセットの manifest には生成に使った上流のコミットが書いてあり、pin と違えば作り直して置き換えます。`local` だけは保管しません。ジェネレータも参照実装もリポジトリの中にあって、生成が安いからです。

置き場は private リポジトリの Release アセットにします。リポジトリ名は `procon-judge-testdata` にして private で作ります。タグは 1 本だけ用意して、そこへアセットを足していきます。判定システム側からは PAT で読みます。secret 名は `TESTDATA_TOKEN` にします。

この形を選ぶ理由は 4 つです。

- 新しいサービスも新しい認証も増えません。`gh` の PAT だけで済みます。
- 無料で、egress の課金もありません。
- private なので公開ミラーになりません。
- アセットは git の外にあるので、リポジトリが太りません。

リリースは `testdata` の 1 本だけ作って、そこに 1 問 1 アセットで貼ります。名前は `<問題 id>.tar.zst` にします。問題 id がディレクトリ名なので、アセット名は対応表なしで計算できます。中身は入出力のペアと `manifest.json` です。manifest にはケース名、サイズ、内容のハッシュ、取得元、取得した日時を入れます。

アップロードとダウンロードはアセット単位です。1 問足すときは upload 1 回で済みます。他のアセットに触らず、コミットも発生しません。落とすときは `-p` でパターンを指定すると、その 1 個だけ取れます。

**`--clobber` は使いません。** あれは消してから上げるので、run のジョブが同時に走ると、片方が消した先へもう片方が上げて 404 になり、アセットが無い状態で残ります。実際に `yuki-649` がそうなって、翌日の実行が yukicoder の原本を叩き直していました。既にあるものは上げ直さず、無いときだけ `--clobber` なしで上げます。競り負けても何も消えていないので、負けた側は通すだけで済みます。取り直したデータに入れ替えるのは人が 1 本で叩く操作なので、`pj mirror push --force` に残してあります。例外は `library_checker` の pin が動いたときで、古い pin のアセットは CI が作り直して置き換えます。これも意図した置き換えで、取り直しの競り合いではありません。

取れなかったときは理由を出します。まだ無いのか、あるのに取れなかったのかで意味が違います。どちらでも呼ぶ側は原本へ落ちますが、後者は保管庫を置いた目的と逆なので黙らせません。上の `yuki-649` が 1 日気づかれなかったのは、ここが黙っていたからです。

```bash
gh release upload testdata aoj-DSL_2_A.tar.zst --clobber -R hashiryo/procon-judge-testdata
gh release download testdata -p 'aoj-DSL_2_A.tar.zst' -D .cache/ -R hashiryo/procon-judge-testdata
gh release view testdata --json assets -R hashiryo/procon-judge-testdata
```

1 問 1 リリースにはしません。保管済みの一覧を取るのに `gh release list` のページングが必要になります。1 問足すのもリリース作成と upload の 2 手になります。タグも問題数だけ生えます。

数百のアセットが 1 リリースに並ぶと Web UI は見づらくなります。見づらい場合は取得元ごとにタグを分けてください。`problem.toml` の `source` からタグが決まるので、追加の情報は要りません。後から移すのも安く済みます。

`pj fetch` の探索順です。

| 段 | 置き場 | 備考 |
| --- | --- | --- |
| 1 | `.cache/testcases/` | 手元のキャッシュ。CI のランナーは持たずに始まります |
| 2 | 保管庫 | private リポジトリの Release アセット |
| 3 | 原本 | 判定サイトの API またはジェネレータ |

3 まで到達したら、成功したあとに 2 へ上げます。一度保管すれば、CI が原本を叩かなくなります。`library_checker` は 1 と 2 で見つけたものが pin と違えば 3 へ進み、2 のものは置き換えます。

`cases_hash` はケースの内容から計算してください。取得経路に依存させてはいけません。保管庫と原本のどちらから取っても同じ値になる必要があります。違う値になると、キーが一致しません。キャッシュが無駄に消えます。

`.` で始まるファイルはケースに数えず、アーカイブにも入れず、展開したら捨てます。macOS の tar は拡張属性を `._<名前>` (AppleDouble) の別エントリにして書き、Linux で展開するとそれが実ファイルになります。`*.in` に当たるので、放っておくと中身がメタデータの偽のケースになり、`cases_hash` が変わって提出が全部落ちます (下の「AppleDouble の掃除の記録」)。

手動の取り込みの流れです。

1. 手元でテストデータを落とします。JOI 春合宿は www2.ioi-jp.org の配布 zip (`<課題>-data.zip` か日ごとの `Day<n>-data.zip`) で、`in/` と `out/` の同名ファイルを `.in` / `.out` に並べ直します。原題の制限は同じページの OVS (概要) の PDF にあり、既定の 5 秒を超えるもの (2012 Day4 の Copy & Paste は 17 秒) だけ `tle_sec` に写します。
2. `pj testdata import --problem ID --dir PATH` で取り込みます。
3. `pj mirror push --problem ID` で保管庫へ上げます。

以降は CI が保管庫から取るので、原本に触りません。

容量の見積もりです。Library Checker は 1 問 100 MB 前後 (最大は `convolution_mod_2_64` の 565 MB) が 128 問で 12 GB ほど、AOJ 222 と yukicoder 192 とその他 50 ほどは圧縮して 1 問あたり数 MB から数十 MB です。AtCoder の問題は入手できません (UTPC の 2 問だけは公開データが `tc.zip` にあったので `manual` で入れました)。Release アセットは 1 ファイル 2 GiB までで、リリース全体の上限はありません。超えるものは上げずに要るたびに作り直します (`mirror.ASSET_MAX_BYTES`)。Library Checker の `convolution_mod_large` は 12 GB 出るので、問題として入れていません。

actions/cache は使いません。7 日で消えるので durable ではなく、全問題ぶんの塊は 10 GB の上限に当たります。既存の Library が `tc.zip` を Release へ置いているのは、前者に気づいた結果です。

Cloudflare R2 は 10 GB まで無料で egress も無料なので候補になります。ただし認証が 1 つ増えます。GitHub で足りるので使いません。

### 実行と打ち切り

`tle_sec` はケースごとの制限です。測るのはプロセス全体の実時間で、`algo_time_ns` とは別です。

超えたらそのケースでプロセスを kill して `TLE` にします。メモリと違って後判定にできません。無限ループの提出があると、GitHub の 6 時間の上限までジョブが埋まるからです。

TLE か MLE が出たら、残りのケースは走らせません。1 ケースに時間がかかるからです。WA と RE は安いので最後まで走らせて、AC でなかったケースの名前を `failed_cases` に残します (上限 10 件)。小さいケースだけ落ちるのか最大ケースだけ落ちるのかが、原因の見当を付ける材料になります。`failed_case` は最初の 1 つの明細です。打ち切った場合、`time_max_ms` と `time_total_ms` はそこまでの値になります。

スタックの上限は硬い方まで上げてから走らせます。既定の 8 MB だと、再帰で木を辿る実装が深さ数十万で落ちます。判定サイトはどこもスタックを縛らないので、そこに合わせます。縛ったままだと、問題とは関係のない理由で再帰の実装だけが RE になり、比較になりません (`aoj-ITP2_2_D` の SplayTree が実際にそうなりました)。`posix_spawn` には exec 前に差し込む口が無いので、`pj` 自身の上限を上げて子へ継承させます。使うメモリが増えるわけではないので、`mle_mb` の後判定は変わりません。

コンパイルにも上限を置いてください。constexpr を重く回す実装があるとコンパイルが長くなりますし、`-flto` のリンクも伸びます。既存の judge が callgrind に 180 秒の timeout を置いていたのと同じ理由です。超えたら `CE` として記録します。

### 出力比較

| kind | 説明 |
| --- | --- |
| `tokens` | 空白と改行を無視してトークン列を比較します。既定値です |
| `float` | トークン比較で、数値は誤差を許します。`abs_tol` と `rel_tol` を指定します |
| `checker` | チェッカのバイナリに委ねます。引数は入力、提出の出力、期待出力の順です |
| `exit_code` | 期待出力を使わず、終了コードが 0 なら AC とします |
| `compile_only` | 実行しません。コンパイルが通れば AC とします |

`checker` は Library Checker が同梱するものをそのまま使えます。その場合 `problems/<id>/checker.cpp` は作りません。M1 の題材がこれに当たります。独自に書くときだけ置いてください。

## 提出

`problems/<id>/submissions/` に置いたものが現役です。

拡張子は種別で分けます。`kind = "base"` の提出はハーネスから include されるので `.hpp` にします。`kind = "raw"` の提出はそれ自体が翻訳単位なので `.cpp` にします。`.cpp` を include する形にすると、エディタや clangd が単体でコンパイルしようとして誤ります。

実行対象はこれだけにします。過去に置いていたものを再実行する仕組みは作りません。消した提出の記録は jsonl に残りますが、サイトには出しません。測り直せないので、参考のまま順位表に並び続けることになるからです (raw の問題をハーネスに作り替えると `lib.cpp` がこれになります)。

提出のパスがそのまま安定した識別子になります。ライブラリの解説から張るリンクはこのパスで、そのページが最新の記録を描きます。

先頭が `_` のファイルは提出として扱いません。共通ヘッダの置き場に使います。

慣習として、`lib` を「自分のライブラリを使った提出」の名前にします。ただし `pj` はこの名前に意味を持たせません。ライブラリとの紐付けは include の一覧から導きます。`pj problems import` が verify のファイルから作る raw の提出も `lib.cpp` で、同じ問題に実装が複数あれば `lib-<実装>.cpp` です。

## ライブラリの取得

ライブラリは submodule で pin しません。pin すると最新を指すという目的が達成できないので、実行のたびに取り直します。

CI は既定ブランチを `lib/` へ clone して、その SHA を記録の `library_sha` に入れます。全環境のフラグに `-I lib` を足すので、提出は `#include "mylib/data_structure/SegmentTree.hpp"` の形で書けます。

記録の `includes` は `lib/` を外した相対パスで書きます。ライブラリ側のサイトが自分のパスと突き合わせるためです。

ライブラリが取得できなくても問題は走ります。ライブラリを使わない提出があるからです。取得に失敗したら、ライブラリを include する提出だけを飛ばします。

ライブラリ側の push からこちらを起こすには `repository_dispatch` を使います。`GITHUB_TOKEN` では別リポジトリが起こせないので、PAT をライブラリ側の secret に置く必要があります。実装は M9 の記録に書いてあります。

このリポジトリの結果はリポジトリの状態だけからは再現しません。ライブラリを HEAD で取るからです。再現性は記録の側が持ちます。`library_sha` と `judge_sha` があれば、そのときの入力が git から復元できます。

## 採点レコード

1 レコードが 1 行の JSON です。

```json
{
  "key": "3f2a1b...",
  "problem": "yosupo-point-add-range-sum",
  "submission": "submissions/lib.hpp",
  "status": "AC",
  "env": "x64-gcc",
  "cpu_arch": "x86_64",
  "cpu_model": "AMD EPYC 7763 64-Core Processor",
  "compiler_version": "g++ (Ubuntu 15.1.0-1ubuntu1) 15.1.0",
  "cxxflags": "-std=gnu++23 -Wall -Wextra -O2 -flto=auto -pthread ...",
  "cases_hash": "b851e7bd8718d0d6",
  "case_count": 42,
  "submission_hash": "9c4e...",
  "includes": ["mylib/data_structure/SegmentTree.hpp", "mylib/algebra/ModInt.hpp"],
  "library_sha": "abc1234...",
  "judge_sha": "def5678...",
  "time_max_ms": 180,
  "time_total_ms": 620,
  "algo_time_max_ns": 128321585,
  "algo_time_total_ns": 472920690,
  "memory_max_kb": 12480,
  "source_bytes": 4820,
  "binary_bytes": 11464,
  "harness_hash": "1a7f...",
  "problem_hash": "5d02...",
  "file_hashes": {
    "submissions/lib.hpp": "9f23d2d57a42e7d4",
    "mylib/data_structure/SegmentTree.hpp": "b1c9a0e4f2d37c58",
    "base.cpp": "0c4d7e2a91b6f3e5",
    "pj.hpp": "e7a2c4d1f0b98365"
  },
  "failed_cases": [],
  "failed_case": null,
  "batch": "35702347473/x64/3",
  "timestamp": "2026-09-19T10:00:00Z"
}
```

`batch` は束の id です (「順位表を束で測る実装の記録」)。base の問題で、全提出を同じジョブで測ったときに付きます。CI では `<run id>/<組>/<ジョブ番号>`、手元では `local/<12 桁>` で、raw の問題と、手元で 1 本だけ測った記録には付きません。

`status` は `AC`、`WA`、`TLE`、`MLE`、`RE`、`CE` のいずれかです。

`includes` はライブラリとの紐付けに使います。「このコンパイルはこのパスを含んだ」という中立な事実なので、ライブラリ固有の概念をこのリポジトリに持ち込みません。

`harness_hash` と `problem_hash` はキーの成分です。`file_hashes` は提出とハーネスの閉包にあるファイルごとの、正規化したあとのハッシュの先頭 16 桁です。記録が参考に落ちたとき、どの成分のどのファイルが動いたかを名指しするために持ちます。`includes` をラベルだけのまま残しているのは、ライブラリ側のサイトが自分のパスと突き合わせるためです。

`failed_case` は失敗したときだけ埋めます。中身はケース名、状態、時間、メモリ、出力の差分の先頭 (2000 字まで) です。`failed_cases` は AC でなかったケースの名前の一覧で、WA と RE は最後まで走らせるので複数になります。AC のときはケースごとの明細を捨てます。1 レコードの大きさが 10 分の 1 ほどになります。

記録の件数を抑えているのはスキップの方です。件数はキーの種類の数で決まり、実行の頻度では増えません。明細を捨てるのは 1 レコードの大きさの話で、別の軸です。

## キーとスキップ

CPU モデルが同じなら実行時間は再現します。judge の履歴 17705 件で測ったところ、変動係数の中央値は 0.69% でした。逆に CPU モデルを無視すると中央値比で 1.14 倍ずれます。だから CPU モデルをキーに入れると、過去の記録をそのまま比較に使えます。

キーの計算はこうします。

```
normalize(src) = C++ ならトークンの列 (コメントと空白を捨てる)、
                 それ以外は行末空白除去 + 空行除去

submission_hash = sha256( normalize(提出のソース)
                          + Σ sorted(include 閉包) の (パス + normalize(内容)) )
harness_hash    = sha256( normalize(base.cpp)
                          + Σ sorted(include 閉包) の (パス + normalize(内容)) )
problem_hash    = sha256( problem.toml を正規化したもの )

key = sha256( 提出のパス, submission_hash, harness_hash, problem_hash, cases_hash,
              env, compiler_version, cxxflags, cpu_model )
```

キーの材料の `cxxflags` は、実際のフラグのうち問題自身のディレクトリの `-I` を `-I@problem` に置き換えたものです (`build.key_cxxflags`)。記録の `cxxflags` の欄には実際のフラグをそのまま残します。問題をどこに置くかをキーから切り離すためで、これが無いと `problems/` の下でディレクトリを動かしただけでその問題の記録が全部測り直しになります (2026-09-23 に入れました。「問題ディレクトリの階層化とキーの -I の正規化の記録」)。

提出のパスをキーに入れるのは、中身が同じ提出を別物として数えるためです。`submission_hash` は中身と include 閉包だけから作るので、バイト単位で同じ提出が 2 本あると同じ値になります。提出ページはパスごとに描くので、パスが違えば別の記録が要ります。リネームすると測り直しになりますが、新しいパスには記録が無いので、そちらの方が欲しい動きです。

`problem_hash` は実行に影響する項目だけから作ります。`limits`、`harness`、`testdata`、`compare` です。`title` のような表示用の項目は外します。題名を直しただけで全部が走り直すのを避けるためです。

`source = "local"` のときは、ジェネレータと参照実装の中身も入れます。どちらもリポジトリの中にあって、書き換えれば出るケースが変わるので、ファイル名だけでは足りません。`cases_hash` でも拾えますが、あちらは記録から借りることがあって、借りた値は古いままです。リポジトリの中にある原因は `problem_hash` へ、外にある原因 (判定サイトや上流のジェネレータ) は `cases_hash` へ、と分けておきます。

`local` のときだけ足します。すべての問題に足すと、既存の記録が丸ごと測り直しになります。

`harness_hash` が閉包まで見るのは、共有のハーネスヘッダを書き換えたときに測り直しを起こすためです。名前を挙げて足す形だと、`harness/pj.hpp` のような外のファイルが抜けます。問題ごとの `common.hpp` も `base.cpp` が include していれば閉包から入るので、名指しは要りません。

`kind = "raw"` の問題には `base.cpp` がありません。その場合の `harness_hash` は空文字列のハッシュにします。

**このキーの記録が既にあれば実行をスキップします。** 記録の状態は見ません。同じソースを同じ条件で測り直しても結果は変わらないので、WA でも TLE でも CE でも再実行しません。直せばソースが変わってキーも変わるので、そのときに測り直されます。

これは raw の問題の規則です。順位を出す base の問題は 2026-09-22 から束で測ります。(問題, 環境, CPU モデル) のいちばん新しい束が今の全提出のキーを揃えていれば全部飛ばし、1 本でも欠けていれば全提出を同じジョブで測り直します。キーが記録にあっても、それが最新の束の外にあれば測り直します (「順位表を束で測る実装の記録」)。

整形を落としてからハッシュを取るのが要点です。ライブラリを磨いている時期は整形だけの変更が頻発するので、これを入れないと中身が同じなのに全部走り直します。

コメントも落とします。C++ のソースは小さな字句解析器でトークンの列に直してからハッシュを取ります。字句解析器は、間違えるなら余計に測り直す側に倒すように作ってあります。避けるのは逆向きの誤りだけで、違うコードが同じトークン列になると変更が見えなくなり、記録が静かに古くなります。それが起きうる場所は 3 つで、文字列と文字のリテラル (raw string を含む) はコメントより先に判定する、記号はコンパイラと同じ最長一致で切る (`a + +b` と `a ++b` を区別する)、プリプロセッサの指令の行はトークンに分けずコメントを外して空白を潰した行のまま 1 トークンにする (`#define F(x)` と `#define F (x)` を区別する)、でそれぞれ塞いであります。行継続と CRLF は先に揃えます。`>>` と `> >` のように意味が同じで列が違うものは別扱いになりますが、余計に測り直すだけです。

トークン単位の正規化は C++ のファイルだけに使います。`local` のジェネレータのような Python は字下げに意味があるので、空白を捨てると違うコードが同じになります。そちらは行末空白と空行を落とすだけです。

正規化を変えると全キーが変わって、記録が全部一度参考に落ちます。最初は空白の正規化だけで始めて、トークン単位にしたのは記録が少ないうちです。

include 閉包は `#include "..."` を再帰的に辿って作ります。角括弧の include は辿りません。閉包の解決は `pj/include.py` に置いて、pytest で固めてください。

閉包を辿るときの include パスは、コンパイル時と同じにしてください。`lib/`、問題のディレクトリ、`harness/`、`problems/`、`third_party/simde` です。ずれているとライブラリのヘッダを解決できず、閉包が欠けます。欠けた閉包はキーを誤らせるので、ライブラリを直しても再実行されなくなります。

この設計から出る結果を 4 つ書きます。

順位を出すために同じジョブへ同居させる必要がありません。同じ CPU モデルと環境の記録を集めて並べるだけで順位が出ます。問題単位で全提出をまとめて走らせる仕組みは要りません。(2026-09-22 に撤回しました。同じ CPU モデルでも VM ごとに速さが 2 割ほど違い、pclmul 系の問題では variant の差より大きいことが分かったので、base の問題は問題単位で全提出を同じジョブで測る束にしました。「順位表を束で測る実装の記録」を見てください。raw の問題はこの文のままです)

広く依存されたヘッダを触っても、走るのはそれを含む提出だけです。他の提出の記録は既にあるので再実行しません。

CPU モデルはジョブが始まるまで分かりません。したがって「何を走らせるか」の判定はジョブの中で行います。ジョブの外で決められるのは、環境ごとの本数と問題を見る順番だけです。どの問題をどのジョブが測るかも、ジョブが起動してから宣言で取ります (「CI」の run の節)。

### cases_hash は借りてから確かめます

キーに `cases_hash` が要りますが、そのために毎回テストデータを落とすのは高くつきます。手元に manifest があればそれが本物なので使い、無ければ記録から借りた値で仮に判定します。借りた値で「走らせるものが 0 件」と出た問題には、テストデータを 1 バイトも触りません。

1 件でも走らせるなら、そこで初めて取得します。取得した本物が借りた値と違っていたら、その問題の判定を本物で組み直します。どの記録にも無いキーになるので、その問題の提出が全部測り直しに戻ります。借りた値のままキーを作って走らせると、測った相手と記録の `cases_hash` が食い違うので、そこだけは必ず確かめます。

借りるのはその問題のいちばん新しい記録の値で、規則は `Store.cases_hashes` が持ちます。`plan` も同じものを使います。二度書くと片方を直し忘れて、記録の意味が静かにずれます。

引き換えに、判定サイトや上流がテストデータを直したことには、別の理由でその問題が走るまで気づきません。疑ったときに叩く口は `pj run --refresh` で、こちらは借りた値で早じまいせず必ず取り直します。保管庫の中身も疑うなら `pj fetch --from-origin` を使うと、原本まで戻れます。

**raw の問題では、1 つのキーは 1 回しか測られません。** 同じ条件での再測定が起きないからです。記録が増えるのはキーが変わったときだけで、それは別の測定です。だから測定値の質はその 1 回で決まります。変えたくなったときの手は「採らなかった選択肢」に書いてあります。base の問題は束で測るので、同じキーの記録が束ごとに 1 件ずつ付きます。順位表に出るのはいちばん新しい束の値です。

## 環境

`environments.toml` に定義します。環境名はキーの一部なので、一度決めたら変えないでください。

```toml
[[env]]
name = "x64-gcc"
runs_on = "ubuntu-24.04"
cxx = "g++-15"
cxxflags = "-std=gnu++23 -Wall -Wextra -O2 -march=x86-64-v3 -mpclmul -mvpclmulqdq -flto=auto -pthread"

[[env]]
name = "x64-clang"
runs_on = "ubuntu-24.04"
cxx = "clang++-21"
cxxflags = "-std=gnu++23 -Wall -Wextra -O2 -march=x86-64-v3 -mpclmul -mvpclmulqdq -flto=auto -pthread -fuse-ld=lld"

[[env]]
name = "arm-gcc"
runs_on = "ubuntu-24.04-arm"
cxx = "g++-15"
cxxflags = "-std=gnu++23 -Wall -Wextra -O2 -mcpu=neoverse-n2+aes -flto=auto -pthread -DUSE_SIMDE -DSIMDE_ENABLE_NATIVE_ALIASES"

[[env]]
name = "arm-clang"
runs_on = "ubuntu-24.04-arm"
cxx = "clang++-21"
cxxflags = "-std=gnu++23 -Wall -Wextra -O2 -mcpu=neoverse-n2+aes -flto=auto -pthread -fuse-ld=lld -DUSE_SIMDE -DSIMDE_ENABLE_NATIVE_ALIASES"

[[env]]
name = "local"
runs_on = "self"
cxx = "c++"
cxxflags = "-std=gnu++23 -Wall -Wextra -O2 -DUSE_SIMDE -DSIMDE_ENABLE_NATIVE_ALIASES"
```

`runs_on = "self"` は手元専用で、CI のマトリクスには出しません。Mac の Apple clang を使うので `-march` は付けません。

x64 の 2 環境は、土台の命令を `-march=x86-64-v3 -mpclmul -mvpclmulqdq` で渡します。x86-64-v3 の命令に pclmul と vpclmulqdq を足したもので、GitHub の x64 ランナーの 6 モデルはどれも持っています。土台に無い AVX-512 と GFNI は、使う提出がソースで `#pragma GCC target` や関数の target の属性として宣言します。2026-09-25 に `-march` を一度外し、同じ日に土台だけをオプションに戻しました (「-march を外してソースで宣言する記録」)。

引き金ごとに走らせる環境を絞る仕組みは入れません。4 環境を並列で回しても 1 分半なので、変更ごとの費用を削る意味がありません。

`pj` は `environments.toml` の `cxxflags` に `-Ilib`、問題のディレクトリ、`-Ithird_party/simde` の 3 つを足してからコンパイルします。記録の `cxxflags` には足したあとの文字列をそのまま入れてください。キーの一部なので、記録と実際のコンパイルがずれてはいけません。`-I` を `environments.toml` の側にも書くと二度出るので、書かないでください。

arm では SIMDe を使います。手元の Mac が arm で提出先が x86 なので、x86 の intrinsics をそのまま書いて arm で動かすために必要です。SIMDe は `third_party/simde` に submodule で入れます。

arm の 2 環境は `-mcpu=neoverse-n2+aes` で組みます。GitHub の arm のランナーで観測されているのは Neoverse-N2 だけで、SVE2 や AES と PMULL を持っています。`+aes` を足すのは、SIMDe が `__ARM_FEATURE_AES` のあるときだけ `_mm_clmulepi64_si128` を PMULL で組むからです。ただし clang で PMULL を使うのは 22 からで、clang 21 は今までどおり SIMDe の汎用の実装を通ります (「-march を外してソースで宣言する記録」)。

## CLI

```
pj problems list [--json]
pj problems check                            problem.toml の検証
pj problems titles [--fix]                   題名を判定サイトの名前と突き合わせる
pj problems import PATH... [--single-only] [--dry-run]
                                             competitive-verifier のテストを raw の問題として取り込む
pj submissions list [--problem ID]
pj fetch [--problem ID] [--all]              テストデータ取得
pj testdata import --problem ID --dir PATH   手元で落としたものを取り込む
pj mirror push --problem ID                  保管庫へ上げる
pj mirror pull --problem ID                  保管庫から落とす
pj mirror status                             問題ごとの保管状況
pj run  --env NAME [--minutes N]             実行して記録を出す
pj run  --env NAME --claim-run RUN_ID --job J --jobs N
                                             CI のジョブ。問題を 1 つずつ宣言して測る
pj claims clean --run-id RUN_ID              この run 以前の宣言を消す (collect 用)
pj claims list                               今ある宣言の ref を出す
pj run  ... --evict-testdata                 測り終えた問題のテストデータを捨てる (CI のランナー用)
pj run  --dry-run --env NAME                 走らせる対象を出すだけ
pj run  --problem ID --submission PATH --env NAME   1 件だけ走らせる
pj records append <dir>...                   まとめて results の jsonl へ追記する
pj records squash                            results ブランチの履歴を畳む
pj bundle --problem ID --submission PATH     1 ファイルに展開する
pj repro --problem ID --submission PATH [--case NAME] [--env NAME]
                                             手元で走らせて、落ちたケースの差分とファイルの場所を見る
pj site build [--out DIR] [--store DIR]   記録から静的なサイトを作る
```

手元と CI が同じコマンドを使います。CI 前提の分岐を CLI の中に入れないでください。手元では `--problem` と `--submission` で 1 件に絞り、CI では `--claim-run` で宣言しながら測ります。違うのは引数だけです。

`pj run` の流れはこうです。

1. CPU モデルを検出します。
2. 対象の提出を列挙します。手元では `--problem` と `--submission` で絞ったもの、CI では宣言した問題のものです。
3. 各提出のキーを計算します。
4. 既存の記録にあるキーを除きます。
5. 実行します。CI では宣言と実行を時間の上限まで繰り返します。
6. 記録を JSON Lines で出力します。

担当は `plan` が事前に配るのではなく、ジョブが宣言で取ります。キーに `cpu_model` が入っているので、当たったモデル次第で「未計測」の中身が変わり、事前に配ると当たり外れが出るからです。当初は `plan` が束を配っていました (2026-09-22 に変えました。「宣言による割り当ての実装の記録」)。

## CI

ワークフローは 1 つです。`.github/workflows/judge.yml`。

引き金は 4 つです。

| 引き金 | 用途 | モード |
| --- | --- | --- |
| `push` (main) | 問題や提出を足したとき | 網羅 (`cover`) |
| `repository_dispatch` | ライブラリ側の更新から起こすとき。種別は `library-push` | 網羅 (`cover`) |
| `schedule` | 全モデルの穴を埋めるとき。1 日 2 回 (03:00 と 15:00 JST) | 全モデル (`all`) |
| `workflow_dispatch` | 手動。入力 `mode` で選ぶ | 既定は全モデル |

モードは 2 つです。網羅モード (`cover`) は (問題, 環境) ごとに「今の全提出が現行になっている CPU モデルが 1 つでもあれば飛ばし、どのモデルにも欠けがあれば測る」で決めます。測るのは宣言を取ったジョブが乗ったモデルで、環境ごとに 1 モデルだけが新しくなります。全モデルモード (`all`) は (問題, 環境, モデル) ごとにそのモデルの欠けを埋めます。push の直後に見るのはその提出の順位表 1 つなので、そこに出るまでの時間を優先し、全モデルに揃えるのは schedule に任せます。2026-09-23 からで、設計は my-docs の「procon-judge の push の run を網羅モードにする設計」にあります。

```yaml
concurrency:
  group: judge-<モード>
  cancel-in-progress: false
```

記録を失いたくないので、実行中のものを打ち切らずに待たせます。group はモードごとに分けます。push の run (網羅モード、数分) が schedule の run (全モデルモード、数時間) の後ろで待たないためです。同時実行の枠 (20) は group をまたいで共通で、待ち行列のジョブは古い順に枠を取るので、全モデルモードの本数は 1 run 16 本に抑えて push の run のための枠を残します (「上限」の節)。2 つの run が同時に走ることで当たる場所は 3 つあります。`results` への push は collect の取り直しで済みます。同じ (問題, モデル) を両方が測る重複は (キー, 束) の重複排除と最新の束だけを並べる表示が守るので無害です。宣言の掃除は走っている run のぶんを残すようにしました (「collect がやること」)。

ジョブは 4 つです。`test` は `pj` 自身の pytest を回すだけで、ほかとは独立に走ります。キーの計算や出力比較が壊れると記録の意味が変わるので、手元だけで見るのはやめました。環境の定義と matrix がずれていないかもここで見ています。

残りの 3 つは段になっています。`plan` が組 (x64 / arm) ごとの本数を決め、`run` がその本数だけ立って測り、`collect` が記録をまとめてサイトを出します。組は同じマシン (runs_on) に載る環境の束で、`x64-gcc` と `x64-clang` は `x64` です。1 本のジョブは組の全環境を測ります。稀な CPU モデルに当たった 1 本が、その場で gcc と clang の両方を埋めるためです (2026-09-22 に環境ごとのジョブから変えました)。

### plan が決めること

CPU モデルはジョブが始まってマシンが割り当たった瞬間に決まります。始まる前には分かりません。だからモデルに依存しない判断だけを `plan` で済ませて、依存する判断は `run` の中に残します。

`plan` は `results` と問題定義と `lib/` を読んで、環境ごとに次を決めます。

1. その環境の既知の CPU モデルそれぞれについて、未計測の提出を数えます。キーに要る `cases_hash` は既存の記録から借ります。記録が無ければそれは未計測なので、借りる必要もありません。テストデータは 1 バイトも要りません。
2. 既知のモデルすべてで 0 件なら、その環境のジョブを立てません。未知のモデルに当たれば仕事はありますが、それは計画された仕事ではないからです。
3. 残った仕事を問題ごとに数えて重い順に並べます。費用の見積もりは、記録のある問題は `time_total_ms` を、無い問題は `case_count × tle_sec` を使います。この並びは `run` の全ジョブが同じものを組み直して、問題を見ていく順番に使います。
4. ジョブの本数を組ごとに決めます。組の全環境と全モデルを合わせた未計測の件数を 1 本あたりの見込み (50 件) で割った切り上げで、1 組 40 本を上限にします。どの問題をどのジョブが測るかは決めません。組の問題の並びは、環境ごとの費用を足した重い順です。

網羅モードでは 1 と 2 を (問題, 環境) の単位で見ます。既知のモデルのどれか 1 つで今の全提出が現行なら仕事は無く、どのモデルにも欠けがあれば 1 モデルぶん (base は全提出、raw は欠けの平均) を仕事に数えます。現行とは、base なら最新の束が全提出のキーを揃えていること、raw なら全提出のキーの記録があることです。「全提出がどこかのモデルにある」ではなく「1 つのモデルが全提出を揃えている」で見ます。順位表は同じモデルの中で比べるので、提出がモデルをまたいで散っていても揃ったとは言えないからです。matrix の各行にモードが載り、`run` は同じ判定で測ります。

出力は matrix の JSON で、`run` が `fromJSON` で受けます。

**新しい CPU モデルを探しには行きません。** 仕事が 0 件でも探りを 1 本立てる形にすると、GitHub のプールが回すモデルを順に引き当ててしまいます。モデルを 1 つ見つけるたびに、そのモデルでの未計測が問題数ぶん生えます。列が 1 本増えるために毎晩それを起こすのは割に合いません。

探さなくても、本当に仕事がある回にジョブが立って、当たったマシンが新しければそこで記録されます。仕事が無い期間に見つからないのは、測るものが無いので困りません。

宣言の単位を問題にするのは、同じ問題の提出が同じマシンに載ると順位表の 1 行がその回で埋まるからです。提出の単位で割ると、5 本ある問題のうち 2 本をモデル X、3 本をモデル Y が測る形に散って、どちらのモデルでも穴が残ります。テストデータの取得も、その問題につき 1 回で済みます。

`plan` にはほかに落とせるものが 2 つあります。include を解決できない提出は、閉包を作る時点で分かるので候補から外して 1 回だけ報告します。今のソースがある環境で CE になる記録を持っていれば、同じ環境の他のモデルでも必ず CE なので立てません。コンパイルに CPU モデルは影響しないからです。

この CE の判定は環境をまたぎません。同じソースでもコンパイラや規格が違えば通ることがあるので、`x64-gcc` で落ちたからといって `arm-clang` を立てないのは誤りです。

### run がやること

1. リポジトリを checkout します。
2. `results` ブランチを `fetch-depth: 1` で読みます。
3. ライブラリを `plan` が決めた commit で取得します。`plan` が clone した HEAD を出力に載せ、`run` の全ジョブと `collect` がその commit を fetch します。run の途中で Library に push があっても、同じ run の中で違う commit を測ったり判定したりしません。`plan` が取れていなければ最新を取ります。
4. マシンの CPU モデルを検出します。**ここで初めて分かります。**
5. `plan` と同じ重い順の並び (組の全環境を合わせたもの) を組み直し、ジョブ番号 j と本数 N から j/N の位置を開始点にして、末尾まで行ったら先頭に戻ります。組のコンパイラのうち入らなかったものの環境は飛ばし、残りで測ります。
6. 並びの問題を順に見て、宣言済みのものと、記録から借りた `cases_hash` で「このモデルではどの環境も走らせるものが無い」と分かるものは飛ばします。それ以外の問題を宣言します。宣言は `refs/claims/<run id>/<組>/<CPU モデル>/<問題 id>` の ref を作ることで、既にあれば弾かれて次へ進みます (`pj/claims.py`)。網羅モードでは飛ばす判定を plan と同じ「揃っているモデルの無い環境があるか」で行い (モデルに依らないので全ジョブが同じ集合を持ちます)、宣言の名前はモデルの位置が固定の `any` です。組の全ジョブが同じ名前空間を見るので、1 つの問題を測るのは run の中で 1 本だけになります。
7. 宣言できた問題のテストデータを保管庫から落とし、環境ごとに本物の `cases_hash` で判定を組み直して、そのモデルで未計測の提出を全部走らせます。base の問題は 1 本でも未計測なら全提出を走らせて束にします。組の全環境を測り終えたらテストデータを捨てます。網羅モードでは揃っているモデルの無い環境だけを測ります。base はその環境の全提出を自分のモデルで束にし、raw は自分のモデルに欠けている提出だけを測るので、測り終えたあとは自分のモデルが揃った状態になります。
8. 残り時間を見て 6 に戻ります。`--minutes` を過ぎたら次を宣言せず、記録をアーティファクトにアップロードします。宣言した問題は必ず測り切ります。

宣言は他のジョブに見えるので、同じ問題を同じモデルで二重に測ることはありません。同じ問題を同じ瞬間に取ろうとしても、ref の作成は片方しか通りません。万一同じものを 2 回測っても、`collect` が既にある (キー, 束) を飛ばすので記録は壊れません。

取得に失敗した問題は警告を出して飛ばします。1 問取れなかっただけで、走れる問題の記録まで落とさないためです。判定サイトが落ちている回でも他の問題の記録は残り、取りこぼしは次の実行が拾います。

### collect がやること

1. アーティファクトを集めて `pj records append` で問題ごとの jsonl へ追記します。
2. `results` ブランチを push します。
3. ライブラリを取得します。記録が古くなっていないかを閉包から判定するのに要ります。
4. `pj site build` でサイトを作って Pages にデプロイします。
5. `pj claims clean` で、この run と、終わっている古い run の宣言の ref を消します。group がモードごとに 2 つあるので古い run がまだ走っていることがあり、その宣言を消すとジョブが同じ問題を取り直して二重に測ります。古い run ごとに終わったかを gh で 1 回見て (`actions: read` が要ります)、走っているものは残して次の collect に回します。

並列で `results` に push すると衝突するので、押すのは `collect` だけにします。`run` は記録をアーティファクトに置くだけです。それでも手作業の push や、schedule と push の run の重なりで弾かれることはあります。弾かれたら最新の `results` を取り直し、その上に `pj records append` で記録を足し直してから押します (5 回まで)。git の rebase は同じ jsonl への追記同士が衝突するので、記録の単位でやり直します。append は同じ (キー, 束) の記録を飛ばすので二重には入りません。

### 上限

| 上限 | 値 |
| --- | --- |
| 1 ジョブの実行時間 | 6 時間 |
| 同時実行ジョブ数 | 20 (Free) |
| 1 実行あたりの matrix | 256 ジョブ |
| actions/cache | 1 リポジトリ 10 GB。7 日使われないと失効。全問題ぶんの塊は載らないので使っていません |

`--minutes` (時間) はこの 6 時間のためにあります。上限に当たるとジョブが打ち切られて、最後のアップロードまで辿り着きません。その回に測ったぶんを丸ごと落とします。必ず終わって記録を上げるところまで行かせるための上限で、過ぎたら次の問題を宣言しません。件数の上限 (budget) は割り当てを宣言に変えたときに消しました。ジョブは宣言が尽きるか時間が来るまで測ります。

全モデルモードの run は、組ごとの本数を出したあと、合計を 16 本に収めます (比で縮め、仕事のある組は 1 本を下回りません)。同時実行の枠は concurrency group をまたいで共通で、待ち行列のジョブは古い順に枠を取るので、20 を超えて並べると push の run (網羅モード) のジョブがその後ろで待つことになります。4 本を空けておけば、push の run は組ごとに 1 本ずつすぐ始まります。稀なモデルに当たる回数はそのぶん減り、穴が埋まるまでの日数は延びます。網羅モードの本数は 1 モデルぶんの提出数を 50 件で割ったもので、1 問の push なら組ごとに 1 本です。上限は 1 組 40 本のままです。

全モデルモードの本数は組の全環境と全モデルを合わせた未計測の件数を 1 本あたりの見込み (50 件) で割って決め、1 組 40 本を上限にします。同時実行の 20 を超えるぶんは GitHub が待たせます。上限を同時実行より多く取るのは CPU モデルの当たりを引き直すためです。未計測はよく当たるモデルでは埋まり、稀なモデルに残ります (2026-09-22 の実測で EPYC 7763 と 9V74 は 4 件、Xeon 8370C は 900 件超)。当たったモデルに仕事が無いジョブは 1 分ほどで終わって次のジョブに枠を渡すので、本数を増やすほど稀なモデルに当たる回数が増えます。arm は CPU モデルが 1 つで仕事が少ないので、本数は x64 の 2 環境に寄り、合わせて 20 前後に収まります。超えたぶんは GitHub が待たせるだけです。

## 記録の保存

`results` という orphan branch に置きます。main とは履歴を共有しません。

main を選ばない理由は 2 つです。CI の push で手元が遅れて毎回 pull が必要になるのが面倒だからです。それと自分の push と CI の push が競合するのが嫌だからです。orphan branch なら main は一切触られません。

```
results ブランチ
  problems/<id>.jsonl        採点レコードを追記
  state.json                 組み合わせごとの最新の状態 (あとから)
```

問題ごとにファイルを分けます。問題をまたぐ追記が衝突しないのと、サイトが必要な単位と一致するのが理由です。

`state.json` は `problems/<id>.jsonl` から導ける索引です。(問題, 提出, 環境, CPU モデル) ごとに 1 行で、最新の記録を持ちます。問題が数問のうちは jsonl を直接読めば足りるので、走査が遅くなってから作ってください。

索引が効くのは、行数が組み合わせの数で決まって履歴の長さに依存しないからです。提出が 1260 本、環境が 4 つ、実際に当たる CPU モデルが 2 つ前後とすると 1 万行ほどで、数 MB に収まります。履歴の方は数十 MB まで伸びて、しかも伸び続けます。

真実の源は jsonl です。`state.json` は壊れても再生成できるキャッシュだと思ってください。サイトと `pj run` が読むのはこれです。上書きなので大きくなりません。

実行した結果はそのまま追記します。raw の問題では記録が作られるのは必ず新しいキーのときです。base の問題は束ごとに同じキーの記録が 1 件ずつ増えます。取り込みの重複排除は (キー, 束) で見ます。

読むときは必ず `fetch-depth: 1` にします。先端の状態しか要らないので、履歴が増えても CI の時間が変わりません。

太ってきたら `pj records squash` で履歴を捨てて 1 コミットに force push し直します。orphan branch なので履歴に意味がありません。既存の Library は 13 MB の結果ファイルを 63 回コミットして `.git` が 158 MB になっているので、同じことを起こさないようにします。

## サイト

Pages のアーティファクトに入れます。`actions/upload-pages-artifact` と `actions/deploy-pages` を使うので、ブランチを経由しません。サイトはビルド出力なので保存しません。記録から作り直せるからです。

npm を使いません。Python でテンプレートを埋めて HTML を出し、対話は素の JavaScript で書きます。図も inline SVG をその場で組みます。既存の judge の `docs/index.html` が単一の HTML で `fetch` するだけの作りなので、その延長です。

```
site/
  index.html                 問題一覧
  problems/<id>.html         順位表
  style.css                  全ページで共有
  index.js                   問題一覧の中身
  problem.js                 順位表の中身
  submissions/<id>/<name>.html   提出ページ。切り替えが無いので静的
  data/index.json
  data/problems/<id>.json
  data/headers/<ラベル>.json     ヘッダごとの逆引き。ライブラリ側のサイトが読む
  data/headers/index.json       ヘッダごとの要約と、verify を畳むゲートの数字
```

提出ページは 1 提出を固定して (環境, CPU モデル) を並べる表で、順位表の転置です。順位表は 1 つの (環境, CPU モデル) を選んで提出を並べます。同じ畳み方 (`collapse`) と同じ現行判定から作るので、新しい機構はありません。載せるのは記録の表、include の一覧、提出ファイルの中身の 3 つです。時系列と `pj bundle` は載せません。時系列は点がまだ 1 つか 2 つで見るものが無く、bundle は「貼れる 1 ファイル」という別の用途のものです。記録は jsonl に溜まり続けるので、見たくなってから作れます。

記録の表の行は (環境, CPU モデル) で、記録の無い CI の環境も未計測の行として出します。参考の行には印だけを置き、何が変わったかは表の下の「参考の理由」に、同じ理由の行をまとめて出します。理由はファイルの一覧で長くなるので、幅を決めた列には入れません。理由の出し方は「参考の理由は Freshness.diff が出します」にあります。

AC でなかった行は「失敗」の節に明細を出します。ケース名、落ちたケースが複数ならその名前、差分の先頭、それと手元で再現するコマンド (`pj repro --problem ID --submission PATH --case NAME`) です。`pj repro` はテストデータを取り、手元のコンパイラで組み、そのケースだけ走らせて、完全な差分と入力・期待出力・実際の出力のファイルの場所を出します。記録は書きません。手元のコンパイラは CI と違うので結果が同じとは限りませんが、WA と RE の大半はここで再現します。

include の一覧は直接と間接に分けます。提出ファイルが直接 include しているものと、そこから辿って入るものです。分けないと `internal/detection_idiom.hpp` が SegmentTree の測定の手柄を持っていき、`pj.hpp` が全提出に出ます。いまの、共通の解法を `common.hpp` にテンプレートで置いて提出は型を選ぶだけにする型では、提出が直接引くのが比べたいエンジンで、`common.hpp` 経由で入るのが ModInt や Algebra になるので、この分け方がそのまま合います。

ライブラリのヘッダは説明ページへ、このリポジトリのファイルは GitHub の blob へ飛ばします。どちらかはラベルの接頭辞で決めて、対応は `libraries.toml` に書きます。pj はライブラリが何かを知りません。接頭辞が合えば説明ページへのリンクと逆引きの JSON を出し、合わなければこのリポジトリのファイルとして扱うだけです。

ソースは提出ファイルだけを埋め込みます。共通の解法を `common.hpp` に置く型の問題では使い方の実体がそちらにありますが、include の欄からリンクで飛べます。埋め込むのはビルド時の checkout のソースで、記録より新しいことがあります。行ごとの現行 / 参考の印がそれを言います。gist は使いません。

切り替えが無いので JavaScript は使わず、ビルド時に HTML まで埋めます。順位表の提出名の列からもこのページへリンクするので、入口はライブラリ側のサイトと順位表の 2 つです。

ヘッダごとの逆引きは `data/headers/<ラベル>.json` で、1 ヘッダ 1 ファイルです。ライブラリ側のサイトが自分のヘッダのパスから URL を組んで、表示時に fetch します。中身はそのヘッダを閉包に持つ提出の一覧で、提出ごとに問題、提出ページのパス、直接か間接か (`direct`)、環境ごとに畳んだ状態と現行かどうかです。ページのパスはサイトのルートからの相対で、CI では `site` に基底の URL が入ります。出すのは `libraries.toml` の接頭辞に合うラベルだけです。問題ごとの `common.hpp` は名前が問題をまたいで衝突するので出しません。

ヘッダごとの要約は `data/headers/index.json` で、1 ファイルに全ヘッダ分が入ります。ヘッダごとに、閉包に持つ提出の数、直接 include する提出の数、環境ごとの「現行の AC が 1 本以上あるか」(`verified`) と現行 AC、失敗、参考、記録なしの本数です。compile_only の提出はコンパイルが通っただけなので数えず、そのヘッダを使う提出が compile_only しか無いときだけそれで読み替えます。ライブラリ側のサイトは依存一覧のアイコンにこれを使い、判定は済んでいるので数を見て色を選ぶだけです。`gate` は全環境で `verified` なヘッダの数と総数で、総数に揃った回で Library の verify を畳みます (my-docs の「Library の verify を畳む設計」)。同じ数字を `pj site build` の summary にも出します。

リポジトリをまたいで固定する契約はこの JSON の置き場と形だけです。どちらの JSON にも `schema` (今は 1) を載せ、形を変えるときに上げます。ライブラリ側のサイトは知らない版なら読まずに節を隠します。ページの URL は同じ `pj site build` が作るので、後から変えても向こうは壊れません。

`style.css` と JavaScript を HTML から外しています。問題のページはテンプレート 1 枚から問題の数だけ作るので、埋め込むと 800 枚に同じものが複製されます。

テストデータが判定サイトのものでない問題には注意書きを出します。`local` は「テストケースは自作で、ここでの AC は元の問題の AC と同じ意味ではない」と言い、ジェネレータと参照実装へのリンクを添えます。`none` は「コンパイルが通るかだけを見ていて、AC はそれを表す」と言います。`manual` は手で取り込んだものだと言うだけです。順位表と提出ページの上に出し、問題一覧の取得元の列も `local` を「自作」、`none` を「無し (コンパイルのみ)」と言い換えて色を変えます。AtCoder のようにケースが公開されない問題は自作するしかありませんが、公式のケースで測ったように見えるのは避けたいからです。逆引き JSON にも `testdata` (取得元) と `official` を載せて、ライブラリ側の表にも印を出せるようにしています。

問題一覧は 1 枚の表のまま分けません。id が出どころの接頭辞で始まるので、id で並べればそれだけで出どころごとに固まって見えますし、800 行あってもブラウザには軽いです。探すのは上の入力欄の絞り込み (id と題名) で、状況を見るのは見出しを押す並べ替え (参考や最終更新) でやります。見出し行は画面に固定します。出どころごとの件数は表の上の 1 行に出します。ページを分けたりページングをしたりすると、探すときはどのページか分からず、状況は全体が見えなくなります。

配色と字体は Library のサイトに揃えます。変数の名前と値を同じにして、ダークモードも同じく OS の設定に従います。ヘッダページと提出ページを行き来するので、別の見た目にしません。最初は GitHub のダークテーマの色で作っていましたが、Library に繋いだときに揃えました。

提出ページのソースには色を付けます。Library は shiki で付けていますが、こちらは npm を使わないので、鍵の計算に使っている字句解析器 (`key.lex`) でトークンに分けて、キーワード、文字列、数、コメント、指令、呼び出しの名前に span を巻きます (`pj/site/highlight.py`)。文法は解かないので型名には色が付きません。色の値は Library と同じ shiki の github-light / github-dark から取っています。

**ページごとに必要な形の JSON をビルド時に生成します。** ブラウザは自分に必要な 1 個か 2 個だけ取ります。結合や反転はすべて生成側で済ませます。

1 つの大きい JSON を読ませてはいけません。既存の judge の `docs/index.html` は 48 MB の `history.jsonl` を `fetch` しています。これを JSON にパースすると JS のオブジェクトで数百 MB になるので、スマートフォンでは落ちます。

大きさの目安です。問題 800 問で 1 行 150 バイトなら `index.json` が 120 KB です。問題のページは提出 10 本 × 環境 4 × CPU モデル 4 で 160 行なので 20 KB です。問題が増えても 1 ページの読み込み量は変わりません。

並びを決める値は、raw の問題ではそのキーの記録の最小値です。base の問題では (環境, CPU モデル) のいちばん新しい束の値で、束をまたいで最小値を採りません。束をまたぐと行ごとに違う VM の値が混ざって比べられなくなるからです (「順位表を束で測る実装の記録」)。

問題のページには環境と CPU モデルの切り替えを置きます。組を 1 つ選ぶと、その組の記録だけが並びます。記録の無い提出は「未計測」として並べます。x86 は複数のモデルに散るので、穴を見せる方が正直です。

提出ページのソースは提出ファイルそのままで、`pj bundle` は使いません。貼れる 1 ファイルが欲しくなったら、それは別の用途なので別に作ります。

Pages のキャッシュヘッダは細かく制御できないので、データファイルの名前かクエリにビルドのハッシュを混ぜて、古いデータを見せないようにします。

Pages のデプロイはサイト全体の差し替えです。変わった問題の分だけ生成しても配信は全量になりますが、800 問ぶんの小さい JSON を作り直すだけなら一瞬なので気にしなくてよいです。

## 最小の第一版

先に全部作らないでください。次のものは第一版から外せます。どれもあとから足せます。

| 外すもの | 足す時期 |
| --- | --- |
| `--shard` と `--budget` | 1 ジョブで捌けなくなってから |
| `state.json` | jsonl の走査が遅くなってから |
| 保管庫 | AOJ や yukicoder の問題を足すとき |
| `aoj`、`yukicoder`、`loj`、`manual` の取得 | 同じとき |
| `compare.kind` の `float` | 必要な問題が出てから |
| ヘッダ別の JSON | 最初の `mylib/...` を使う提出を移すとき |
| 提出ページと `pj bundle` | 時系列を見たくなってから |
| コメント除去の正規化 | 余分な再実行が気になってから |

第一版に要るのは次のものだけです。

- `problem.toml` の読み込みと検証です。
- `base.cpp` のハーネスです。
- `library_checker` と `local` の取得です。
- `tokens`、`checker`、`compile_only` の比較です。
- キーの計算とスキップです。
- jsonl への追記です。
- 問題一覧と順位表のサイトです。

## 実装の順番

縦に 1 本通してから広げてください。基盤を先に全部作る順番は避けます。既存の Library は 2026 年 4 月に実行基盤を 215 コミットかけて作り直したあと、5 月の 1 コミットで止まっています。

| 段 | 内容 | 完了の判定 |
| --- | --- | --- |
| M1 | 1 提出を 1 問題に 1 環境で走らせて記録を 1 行出す | 手元で `pj run --problem X --submission Y --env local` が通る |
| M2 | キーの計算とスキップ。記録をローカルのディレクトリに置く | 2 回目の実行が 0 件になる |
| M3 | CI。`run` と `collect`、`results` ブランチへの追記 | push で記録が増える |
| M4 | テストデータの保管庫。AOJ と yukicoder の問題を足す | CI が原本を叩かずに走る |
| M5 | 環境マトリクス | 4 環境の記録が揃う |
| M6 | サイト生成と Pages | 順位表が見える |
| M7 | ライブラリを通す | `mylib/...` を include する提出が 4 環境で AC になる |
| M8 | 現行と参考と未計測を見分ける | ライブラリを触ると順位表の行が参考に落ちる |
| M9 | ライブラリの push でこちらが起きる | `mylib/` を直すと judge が走り出す |
| M10 | plan と budget | やることが 0 件の push で `run` が 1 本も立たない |

M1 の題材は Library Checker の `point_add_range_sum` を勧めます。テストデータがジェネレータから作れるので判定サイトへの依存が無く、チェッカが同梱されていて、`base.cpp` のインターフェースが素直です。

M1 の時点で `problem.toml`、`base.cpp`、`submissions/naive.hpp` を手で書いて、それを通してください。インターフェースは仕様書ではなく、この最初の 1 問に決めさせます。

## 実装の状況

M10 まで通っています。手元で `pj run --env local` が動き、2 回目は 0 件になります。main へ push すると `plan` が仕事のある環境だけを数えて、その本数ぶんの `run` が立ち、`results` ブランチに記録が増えて、サイトが https://hashiryo.github.io/procon-judge/ に出ます。仕事が 0 件の環境にはジョブが立ちません。AOJ のテストデータは保管庫から取るので、CI は judgedat を叩きません。Library Checker のテストデータも保管庫から取り、無ければ `testdata.toml` のコミットで生成して上げます。`mylib/...` を include する提出も 4 環境で走っていて、Library の `mylib/` を直すとこちらが起きます。

以下は段ごとの実装の記録です。決めたことと、踏んだ罠を書いてあります。

## M1 の実装の記録

提出は 2 本置きました。`submissions/fenwick.hpp` が 21 ケース AC で、Apple M2 Max の `local` 環境で `time_max_ms` が 181、`algo_time_max_ns` が 1440 万ほどです。`submissions/naive.hpp` は区間和を毎回足し直すので `max_random_00` で TLE になります。打ち切りと `failed_case` の確認に使えるので残してあります。

`pj run` は判定が何であっても 0 で終わります。WA や TLE は判定であって実行の失敗ではないので、ここを非ゼロにすると CI の step が落ちて記録を取りこぼします。異常終了はツール側が失敗したときだけです。

### base.cpp のインターフェース

最初の 1 問が決めた形です。

```cpp
struct Solver {
  explicit Solver(const vector<i64> &a);  // 構築。計測区間の外。
  void add(int p, i64 x);                 // a[p] += x
  i64 sum(int l, int r);                  // a[l] + ... + a[r-1]
};
```

構築は計測区間の外に置きました。Fenwick tree の構築は O(N) で、クエリ列の O(Q log N) に対して無視できるからです。

答えは計測区間の中で `vector<i64>` に溜めて、文字列への整形は区間の外でやります。上の `base.cpp` の例は区間の中で `to_string` を呼んでいますが、50 万件の整形は Fenwick tree の処理そのものと同じ桁の時間になります。区間の中には比べたい処理だけを残す方を採りました。区間の外に出すのは I/O だけでなく、出力の整形も同じです。

`common.hpp` では `bits/stdc++.h` を使いません。Apple clang に無いので手元の環境で通らなくなります。必要なヘッダを名指しで include しています。`i64` は `int64_t` ではなく `long long` にしました。`scanf` の `%lld` と型を揃えるためです。`int64_t` は Linux では `long` なので、揃えないとキャストが要ります。

入力の読み込みは `scanf` のままで足ります。13 MB の `max_random` で 149 ミリ秒でした。

### 計測の前に空打ちします

macOS では新しくコンパイルしたバイナリの初回 exec に 0.37 秒ほどかかります。署名の検証が入るためで、2 回目以降は数ミリ秒です。このぶんが最初のケースの実時間に乗ってしまい、`time_max_ms` が本来の 181 ミリ秒ではなく 376 ミリ秒になっていました。計測の前に空の標準入力で 1 回走らせて捨てています。

### 実行の作り

`os.posix_spawn` で起こして `os.wait4` で回収します。リダイレクトは posix_spawn の file_actions でやるので、パイプの読み取りで詰まりません。打ち切りは `threading.Timer` から `SIGKILL` を送ります。回収済みの pid を撃たないようにロックで守っていますが、送る直前に終わっていた場合は `ProcessLookupError` を無視します。

判定の順は TLE、MLE、RE、出力比較です。MLE を RE より先に見るのは、確保しすぎて殺された提出を RE ではなく MLE として残したいからです。

### チェッカのコンパイル

`environments.toml` のフラグは使いません。`-std=c++17 -O2 -w` で別に組みます。チェッカは計測対象ではないうえ、testlib.h は `-Wall -Wextra` で大量に警告を出しますし、`-march` や `-flto` を付ける意味もないからです。コンパイル済みのバイナリはテストデータのキャッシュに `checker.bin` として置いて使い回します。

### テストデータ

`.cache/testcases/<source>/<name>/` には in と out のペア、`manifest.json`、それにチェッカ一式 (`checker.cpp`、`testlib.h`、`params.h`) を置きます。`params.h` は生成のときに作られるので、clone しただけでは存在しません。チェッカのヘッダを一緒に置かないとコンパイルに失敗します。

生成は `uv run --with colorlog python generate.py` で呼びます。`generate.py` が Darwin のスタックサイズを自分で設定するので、`ulimit -s unlimited` で包む必要はありません。macOS では通らない指定でもあります。

`point_add_range_sum` は 21 ケースで、入力が 101 MB、出力が 28 MB でした。

### M1 で入れなかったもの (M2 で入れた分を除く)

`pj fetch` の探索順は、今は手元のキャッシュと原本の 2 段だけです。actions/cache は M3、保管庫は M4 です。

`source = "local"` の取得は実装していません。M1 の題材が `library_checker` なので要りませんでした。`problem.toml` の検証だけは通してあります。

`environments.toml` には `local` しかありません。残りの 4 環境は M5 で足します。`tier` も入れていません。

### そのほか

コンパイルエラーの詳細はログの先頭 400 文字を残します。最初の 1 件が本体なので、末尾を残すと include の連鎖だけが残ります。

`compile_only` の記録では `case_count` が 0、`cases_hash` が空文字列になります。

`pj` 自身のテストは pytest で 25 件あります。出力比較、`problem.toml` の検証、`PJ_METRICS` の読み取り、それと `compile_only` を通した縦 1 本です。include 閉包とキーの計算は M2 で足します。


## M2 の実装の記録

### キーの計算

設計の式どおりに作りました。`pj/key.py` に `normalize`、`submission_hash`、`harness_hash`、`problem_hash`、`compute` があります。

連結には NUL を挟みます。区切りが無いと、隣り合う項目の境目が動いても同じキーが出ます。`env = "ab"` と `compiler_version = "c"` の組と、`env = "a"` と `compiler_version = "bc"` の組が衝突する形です。

`problem_hash` は problem.toml の生の表ではなく、既定値を埋めたあとの値から作ります。`generator = "gen.py"` のような既定値と同じ行を書き足しただけで走り直すのを避けるためです。

手元で次の 5 つを確かめました。行末空白と空行を足しても走り直さない、提出の中身を変えるとその提出だけ走り直す、`common.hpp` を変えると両方走り直す、`title` を変えても走り直さない、`tle_sec` を変えると両方走り直す。

### include 閉包

`pj/include.py` に置きました。ファイルの呼び名は、探索パスを先頭から見て最初に含んでいたものからの相対パスにします。`lib/` が先頭なので、ライブラリのヘッダは `mylib/...` の形になります。記録の `includes` にもこの呼び名を使うので、設計が言う「`lib/` を外した相対パス」がそのまま出ます。

include の書かれ方ではなくファイルの位置で呼び名を決めるのは、どこから辿ったかで変わらないようにするためです。`"dep.hpp"` と `"sub/dep.hpp"` のどちらで書かれても同じ呼び名になります。ここがぶれるとハッシュが辿る順に依存します。

解決できなかった include は落とさずに集めて、警告として出します。`third_party/simde` をまだ入れていない状態で止まってほしくないからです。

`// ` でコメントアウトした include は当たりません。行頭から空白を挟んで `#` が来る形しか見ないためです。ブロックコメントの中は見分けられませんが、字句解析器を書く話ではないので放っています。

### 提出のパスをキーに足しました

最初は設計の式のとおりに作って、提出のパスをキーに入れていませんでした。中身が同じ提出を 2 本置くと 1 つのキーを共有してしまい、片方の記録が出ない状態でした。別のファイルは別の提出として数える方が自然なので、`key` の材料に提出のパスを足しました。「キーとスキップ」の式も直してあります。

`submission_hash` の方は変えていません。こちらは中身だけを表す値として置いておきます。中身が同じ提出の `submission_hash` が同じになるのは、それはそれで言いたいことが言えている値です。

足したのはキーを細かくする向きなので、走らせるべきものを走らせなくなる壊れ方はしません。

### 記録の置き場

`.results/problems/<id>.jsonl` に追記します。M3 でこのディレクトリが results ブランチの作業ツリーになります。`--store DIR` で移せます。

`pj run` は記録を標準出力にも 1 行ずつ流します。進行状況は標準エラーに出るので、標準出力はそのまま jsonl として使えます。

読むときに壊れた行があったら、その行だけ飛ばして警告を出します。追記の途中で落ちると半端な行が残るためです。

### テストデータを落とす前にスキップを決めます

キーには `cases_hash` が要りますが、スキップの判定のために毎回テストデータを落としたくありません。手元のキャッシュに `manifest.json` があれば、そこから `cases_hash` を読んで済ませます。CI では actions/cache が復元するので、全部スキップされる実行では原本を叩きません。

manifest が無いときはキーを計算できないので取りに行きます。`--dry-run` では取りに行かず、未取得として数えます。

### CLI

`pj run --env NAME` で全問題の全提出を見ます。`--problem` と `--submission` で絞れます。`--dry-run` は走らせる対象を出すだけです。`pj records list` で記録の一覧が見えます。

`--shard` と `--budget` はまだ入れていません。1 ジョブで捌けなくなってからです。`pj records squash` は太ってきてからです。


## M3 の実装の記録

### 読む場所と書く場所を分けます

`pj run` に `--out` を足しました。`--store` から記録を読んでスキップを決め、新しい記録は `--out` へ書きます。省略すると両方とも `.results/` です。

CI の `run` は `--store .results --out .artifacts` で呼びます。results ブランチから読んで、記録はアーティファクトへ置くだけです。results へ push するのは `collect` だけにしてあります。並列で押すと衝突するからです。

`--out` の書き先も `Store` なので、アーティファクトの中身は results と同じ `problems/<id>.jsonl` の形になります。`collect` はそれを取り込むだけで済みます。

### pj records append

渡されたディレクトリの下から `*.jsonl` を再帰で拾います。`actions/download-artifact` の展開先は run のジョブごとに 1 段深くなるので、階層を決め打ちしていません。

既にあるキーは飛ばします。設計は「記録が作られるのは必ず新しいキーのときなので、重複は起きません」と書いていますが、ワークフローを回し直しても二重に取り込まないための保険です。追加した件数と飛ばした件数を出します。

### results ブランチ

作業ツリーを触らずに、空の tree から `git commit-tree` で始点のコミットを作りました。`git switch --orphan` は作業ツリーを空にするので、手元のファイルを巻き込む危険があります。

CI では `actions/checkout` で `path: .results` に `fetch-depth: 1` で置きます。main の作業ツリーの中に別のリポジトリが入る形ですが、`.results/` は main の側で gitignore してあるので混ざりません。

### Linux ではスタックを広げます

library-checker の `generate.py` は Darwin と Windows のスタックサイズだけ自分で設定します。Linux は呼ぶ側の責任なので、`bash -c "ulimit -s unlimited && ..."` で囲んでから呼びます。深い再帰を書いたジェネレータがあり、既定の 8 MB では落ちます。移植元の `Library/scripts/lib/download.py` が同じことをしていました。macOS で `ulimit -s unlimited` は通らないので、そちらは囲みません。

### アーティファクトの置き場をドットで始めません

`upload-artifact` は隠しファイルを既定で除きます。最初 `--out .artifacts` にしていたら 1 件も上がらず、`collect` が 0 件で終わりました。`if-no-files-found: ignore` にしていたので、警告も出ずに黙って落ちていました。`${{ runner.temp }}/records` に置き換えました。全部スキップされた実行では 1 件も出ないので、その回はアップロードのステップごと飛ばします。上げると決めたのに空だったときは取りこぼしなので、`if-no-files-found` は `error` にしてあります。

### アクションのバージョン

`runs.using` が `node20` のアクションは、GitHub が Node 24 での強制実行に切り替えている最中です。警告が出るので、`node24` を宣言しているメジャーまで上げました。checkout v7、cache v6、upload-artifact v7、download-artifact v8、setup-uv v10.1.0 です。

`astral-sh/setup-uv` は v8 以降、浮動のメジャータグを出していません。`@v10` では解決できずにジョブが落ちます。正確な版で固定してください。`actions/*` の方は浮動メジャーがあります。

### Linux の RSS は下駄が高いです

ubuntu-24.04 のランナーで測ると、入力が 45 バイトの `example_00` でも 22760 KB 出ます。小さいケースは全部この値で揃います。手元の macOS は同じケースで 1408 KB でした。ケースの大きさで動くぶんはこの上に乗ります (`max_random` で 29820 KB)。設計が言うとおり `mle_mb` をきつく設定しないでください。

これをランナーの下限だと書いていましたが、違いました。`ru_maxrss` に `pj` 自身のピークが混ざっていただけです。yuki-649 を足したときに下駄が 138868 KB へ動いて分かりました。直し方は「ハーネスの種別」に書いてあります。

### マトリクスは今のところ手書きです

`.github/workflows/judge.yml` の matrix に `x64-gcc` と `ubuntu-24.04` を直に書いています。`environments.toml` と二重持ちですが、yml から toml を読むには matrix を作る前段のジョブが要ります。4 環境に広げる M5 でまとめて考えます。

### テストデータのキャッシュ

`actions/cache` で `.cache/testcases` を持ち越します。`point_add_range_sum` 1 問で 129 MB なので、問題を増やすと GitHub の 10 GB に当たります。M4 の保管庫を入れるときに、キャッシュへ何を残すかを決め直します。


## M4 の実装の記録

### 保管するのは作り直せないものだけです

`library_checker` と `local` は保管しません。ジェネレータから決定的に作れるので、保管庫から取ったものと生成したものが一致します。容量を食うだけです。`point_add_range_sum` 1 問で展開後 129 MB あるので、161 問ぶん置くと数 GB になります。

保管するのは `aoj`、`yukicoder`、`manual` です。設計の容量の見積もりが AOJ 222、yukicoder 192、その他 50 で数えているのと揃います。

### アーカイブの中身

`<問題 id>.tar.zst` に入れるのは `*.in`、`*.out`、`manifest.json` だけです。`checker.bin` のような手元でコンパイルした成果物は入れません。`aoj-DSL_2_B` で展開後 10.6 MB が 3.3 MB になりました。

固めて戻したときに `cases_hash` が変わらないことを pytest で押さえてあります。保管庫と原本のどちらから取っても同じ値にならないと、キーが一致せずに測り直しが起きます。

### 認証

`gh` に `GH_TOKEN` を渡して呼びます。keyring の認証は使いません。手元の `gh` は別アカウント (`ryomaHashimoto`) で認証されていて、`hashiryo` の private リポジトリが見えないからです。`GITHUB_TOKEN` が環境にあっても外します。CI の `GITHUB_TOKEN` は `procon-judge` のもので、保管庫のリポジトリには通りません。

手元で `pj mirror` を使うときは、環境変数 `TESTDATA_TOKEN` に PAT を入れてください。無いときは何をすればよいか言って止まります。

### AOJ の切り詰め

judgedat は大きいケースを切り詰めて返すことがあります。header の `inputSize` と `outputSize` に対して、落ちてきたバイト数が合わなければ警告を出します。切り詰められたデータで判定すると結果の意味が変わります。`DSL_2_B` は一番大きいケースが 1.28 MB で、切り詰めは起きませんでした。

### 判定サイトの癖はハーネスで吸収します

`aoj-DSL_2_B` の入出力は 1-indexed で区間が閉じていますが、提出には 0-indexed の半開区間で渡しています。提出の側は `point_add_range_sum` と同じ形で書けます。判定サイトごとの都合を提出へ持ち込むと、同じデータ構造でも問題ごとに実装を書き直す必要が出ます。

### キャッシュの世代

テストデータのキャッシュキーに `v1` のような世代を入れました。まっさらから取り直させたいときに上げます。保管庫から取る経路を通したときは、これを `v2` に上げてキャッシュを空にしました。

キーから `matrix.env` も外しました。テストデータはただのテキストで環境に依存しないので、4 環境ぶん同じものを持つ必要がありません。

### x86 は本当に 3 モデルに散ります

保管庫から取る経路を通したとき、記録済みのはずの 5 件が全部走り直しました。`cases_hash` は原本から取ったときと 1 ビットも変わっていなかったので、保管庫の往復は正しく動いています。変わったのは CPU モデルでした。

ここまでの数回で当たったランナーは AMD EPYC 7763、AMD EPYC 9V74、INTEL(R) XEON(R) PLATINUM の 3 つです。設計が「x86 は 3 モデルに散る」と見ていたとおりでした。CPU モデルはキーの一部なので、別のモデルに当たれば別の測定として記録が増えます。これは意図した動きです。

裏返すと、しばらくは push のたびに測り直しが起きます。モデルの数は有限なので、組み合わせが埋まれば収まります。

### yukicoder

取得は実装しましたが、問題はまだ足していません。`YUKICODER_TOKEN` を secret に入れたら足せます。トークンが無いときは何が要るか言って止まります。


## M5 の実装の記録

4 環境の記録が揃いました。x64 と arm に gcc と clang の組み合わせです。`local` は `runs_on = "self"` にして CI のマトリクスには出しません。設計にあった `tier` は入れませんでした。4 環境が並列で 1 分半なので、引き金ごとに走らせる環境を絞る意味がありません。手元専用かどうかは `runs_on` で分かるので、それ用の数字も要りません。

### -I は環境の定義に書きません

設計の `environments.toml` は arm と `local` の `cxxflags` に `-Ithird_party/simde` を書いていましたが、落としました。`pj` が `-Ilib`、問題のディレクトリ、`-Ithird_party/simde` の 3 つを足すので、両方に書くと記録の `cxxflags` に同じ `-I` が二度出ます。`environments.toml` に `-I` が無いことは pytest で見ています。

`include_dirs` は `third_party/simde` があるかどうかを見なくなりました。submodule を初期化していない手元と、初期化した CI とで `cxxflags` が変わってしまい、同じ提出が別のキーで二度測られるからです。存在しない `-I` はコンパイラが黙って無視します。

この 2 つで `cxxflags` の文字列が変わったので、M4 までの記録のキーは全部無効になりました。1 回ぶん測り直しています。

### 既存の judge から落としたフラグが 2 つあります

既存の `judge` は `-ftrivial-auto-var-init=zero` を 4 環境すべてに、`-fconstexpr-depth=1024` を clang の 2 つに付けています。設計の `environments.toml` はどちらも書いていないので、書いてあるとおりにしました。

`-ftrivial-auto-var-init=zero` は初期化していない自動変数を 0 で埋めます。環境ごとのスタックの中身の違いで判定が揺れなくなる代わりに、大きな自動変数を置く実装ではその 0 埋めのぶん遅くなります。`judge` から提出を移したときに、これに頼っていた実装が WA になることがあります。

`-fconstexpr-depth=1024` は clang の既定の 512 では足りない実装のためのものです。深い constexpr を書いた提出を移すと CE になります。

どちらも `environments.toml` の 1 行です。移してみて困ったら足してください。

### SIMDe は Library と同じコミットに留めます

`third_party/simde` は submodule です。`hashiryo/Library` が `include/simde` に留めているのと同じ `cb5b957` に合わせました。`judge` の algo 実装は `#include <simde/x86/avx2.h>` の形で書いてあるので、`-Ithird_party/simde` があればそのまま通ります。

CI の checkout は `submodules: true` です。`fetch-depth` の既定が 1 なので submodule も深さ 1 で取ります。作業ツリーは 60 MB あって、そのうち 49 MB は simde 自身のテストです。

`ruff` が simde の Python を見に行くので、`pyproject.toml` で `third_party` を除きました。

### arm の CPU モデルは lscpu から取ります

arm の `/proc/cpuinfo` にはモデル名が載りません。実装者と part 番号しか無いので、`cpu_model()` が "unknown" を返していました。CPU モデルはキーの材料なので、このままだと arm の記録が全部 "unknown" で並びます。

`lscpu` が part 番号を名前に直してくれるので、cpuinfo に無いときはそちらに落とします。`ubuntu-24.04-arm` は `Neoverse-N2` でした。x86 ではどちらも同じ文字列を返すので、先に読む cpuinfo の側で決まります。x64 の記録のキーはこの変更では動きません。

`lscpu` の見出しは訳されることがあるので `LC_ALL=C` で呼びます。出力には `Model name` のほかに `Model` の行もあるので、番号の方を拾わないようにしています。

### チェッカのバイナリに arch を入れました

テストデータのキャッシュは 4 環境で共有します。キーに `matrix.env` を入れていないのは、テストデータが環境に依存しないからです。ところが同じディレクトリに `checker.bin` を置いていたので、x86 で組んだチェッカが arm のジョブに復元されて、そのまま使われる形になっていました。名前を `checker-<arch>.bin` にして分けました。

### コンパイラの入れ方

gcc は `ppa:ubuntu-toolchain-r/test` の `g++-15` です。この PPA は arm64 のビルドも置いています。clang は `apt.llvm.org` の `llvm.sh 21 all` で、36 秒で終わります。`-fuse-ld=lld` は PATH の `ld.lld` を探しますが、apt.llvm.org は版つきの名前しか置かないので `/usr/local/bin/ld.lld` に別名を張ります。どれも既存の `judge` が 4 環境で回していた手順です。

clang でも `-flto=auto` が通ります。gcc 向けの綴りですが、clang も LTO の指定として受け取ります。

入らなかったらジョブを落とします。既存の `judge` は、Launchpad が落ちているときに `g++-14` へ切り替えます。こちらでは採りません。記録に残る環境名と、実際に測ったコンパイラがずれるからです。取りこぼしは次の実行が拾います。

### マトリクスは手書きのままにしました

M3 で先送りにした二重持ちの件です。`environments.toml` と `judge.yml` の両方に環境名とランナー名を書いています。

前段のジョブで `environments.toml` を読んで `fromJSON` で matrix に渡す形は採りませんでした。ジョブが 1 つ増えて全体が待たされるのと、yml を読んでも何が走るのか分からなくなるからです。環境を足すのは滅多に起きません。

代わりに、ずれたら pytest が落ちるようにしました。`runs_on` が "self" でない環境の集合と matrix の `include` の集合が一致すること、ランナー名が一致すること、`toolchain` が `cxx` と合っていることを見ています。

それを回す場所として `test` ジョブを足しました。`run` と `collect` からは独立に走ります。シャードで matrix が広がって手で書ききれなくなったら、生成する側へ切り替えてください。

### x86 の 4 つ目のモデルが出ました

M4 の時点では AMD EPYC 7763、AMD EPYC 9V74、INTEL(R) XEON(R) PLATINUM 8573C の 3 つでした。今回 `x64-clang` が `Intel(R) Xeon(R) 6973P-C` に当たって 4 つ目になりました。設計が「x86 は 3 モデルに散る」と見ていたより 1 つ多いです。

CPU モデルはキーの一部なので、当たったぶんだけ記録が増えます。arm は今のところ `Neoverse-N2` の 1 つだけです。

## M6 の実装の記録

`pj site build` が `site/` に問題一覧と順位表を出します。CI では `collect` がこれを呼んで Pages に上げます。公開先は https://hashiryo.github.io/procon-judge/ です。2 問 45 件で 44 KB、`collect` の所要は 21 秒でした。

提出ページとヘッダ別の JSON は入れていません。`pj bundle` も要るので、ライブラリ側のサイトを作るときにまとめて足します。

### 記録を (提出, 環境, CPU モデル) ごとに畳みます

同じ組でもソースを書き換えればキーが変わって、別の測定になります。畳むときは新しい方のキーだけを残して、そのキーに属する記録の最小値を採ります。残った記録が複数あるのは同じ条件を何度か測ったときなので、そこで最小値が効きます。設計が「順位はそのキーの記録の最小値で決めます」と書いているとおりで、あとで標本を積み始めても表示の側を書き直さずに済みます。

どのキーが新しいかは記録の時刻で決めます。同じ時刻が並んだときはファイルの後ろにある方を新しいとみなします。

### 「検証済み」は現在のキーでは判定していません

サイトが見ているのは記録があるかどうかだけで、いま計算されるキーの記録があるかどうかではありません。キーには `cases_hash`、`compiler_version`、CPU モデルが要ります。テストデータを持っていない `collect` のジョブでは前者が出せませんし、CPU モデルはそのジョブのものであって測定したマシンのものではありません。

そのため、提出を書き換えた直後のサイトは古い測定を出します。次の実行で新しいキーの記録が入れば置き換わります。表の「計測」の列に記録の時刻を出してあるので、そこで見分けてください。

(M8 で判定するようにしました。機械側の値を記録から借りればキーは組み直せます。)

### 順位は algo の最大ケースで決めます

記録には実時間 (`time_max_ms`) と計測区間の時間 (`algo_time_max_ns`) の両方が入っています。順位には後者を使います。計測区間の外の I/O と出力の整形を含まないので、実装どうしを比べるならこちらです。`algo` が無い記録は実時間で代用します。

打ち切られた実行の `algo` は、通ったケースまでの値でしかありません。TLE の行に小さい値が出て紛らわしいので、AC でない行では数字を落として表示します。

### CPU モデルの切り替えは環境と組にしました

設計は「4 モデルから 1 つ選ぶ」と書いていましたが、実際には x64-gcc と x64-clang が同じ x86 のモデルに当たるので、モデルだけでは順位表が決まりません。環境と CPU モデルの 2 つを選ぶ形にして、環境を選ぶと、その環境で記録のあるモデルだけが CPU の側に出ます。環境を変えても同じモデルがあれば選び直しません。

記録の無い提出はその組で「未計測」として並べます。問題一覧には計測済みの枠と全部の枠を「14 / 14」の形で出すので、穴があればそこで分かります。

### テンプレートは str.replace で埋めます

`string.Template` を使っていません。`$` と `${}` が JavaScript のテンプレートリテラルと衝突します。テンプレートは素の HTML のままで、`{{TITLE}}` のような目印だけを置き換えます。置き換えたあとに `{{...}}` が残っていたら埋め忘れなので落とします。

### キャッシュ破りは中身のハッシュです

Pages のキャッシュヘッダは細かく制御できないので、参照する側の URL に `?v=` を付けます。値はそのファイルの中身の sha256 の先頭 12 桁です。変わっていないファイルは URL も変わらないので、キャッシュがそのまま効きます。

### --out は空でないディレクトリを消しません

作り直すたびに書き先を消すので、`--out .` を打ち間違えるとリポジトリが消えます。書いたときに `.pj-site` という目印を置きます。目印の無い空でないディレクトリは消さず、そのまま落とします。

### Pages のデプロイ

`collect` に `upload-pages-artifact` と `deploy-pages` を足しました。どちらも v5 です。`deploy-pages` はジョブに `github-pages` という名前の environment を要求します。権限は `pages: write` と `id-token: write` です。

Pages の source を GitHub Actions にする設定は人がブラウザで入れます。入っていないとこのステップだけが落ちます。

### 順位の番号は置きません

設計の「何を作るのか」が AtCoder の提出ページの項目として「同じ問題の他の提出との順位」を挙げていたので、最初は順位の列を出していました。落としました。

AtCoder で順位に意味があるのは、他人の提出が何千と並ぶからです。ここに並ぶのは同じ問題への自分の提出が 3 本から 10 本で、しかも時間順なので、番号は並び順が言っていることを繰り返しているだけです。列で並べ替えられるようにすると「メモリで並べたときの順位とは何か」という答えの無い問いも出ます。

順位そのものは残っています。表の並びがそれです。

### 見出しを押すと並べ替わります

どの列でも並べ替えられます。既定は algo の昇順で、同じ見出しをもう一度押すと逆になります。

どの列で並べても AC の行が先に来ます。通っていない実装を混ぜると、TLE の行が「メモリが少ない」だけで上に来てしまうからです。未計測はいつも最後です。

### 表は幅を固定します

組み合わせを切り替えると数字の桁数も状態の文字も変わるので、幅を中身から決めさせると表が動きます。`table-layout: fixed` にして列ごとに幅を書きました。入りきらない中身は省略記号にして、元の値を `title` に残します。

同じ理由で、並べ替えの印は出ていないときも場所を取ります。失敗したケース名は状態の 2 行目に置くのをやめて、横に並べました。行の高さが揃うので縦にも動きません。

計測の時刻は分までにして、UTC であることは見出しに移しました。秒まで出すとその列が伸びて、提出のパスの幅を食います。元の値は `title` にあります。

### ソースはその記録を測ったコミットへ飛ばします

ソース列のバイト数が `blob/<judge_sha>/problems/<id>/<提出>` へのリンクです。行ごとに違うコミットを指すので、その数字を出したソースが読めます。いまの main ではありません。

リポジトリの URL は書き込んでいません。CI では `GITHUB_REPOSITORY` から、手元では origin の remote から作ります。remote は host に SSH の別名 (`github.com.hashiryo`) が入るので、そこは見ずに末尾の owner/repo だけ拾います。

このリンクで見えるのは提出ファイルだけで、そこから include しているヘッダは見えません。1 ファイルに展開したものは提出ページと `pj bundle` の仕事です。

### library_sha はサイトに出しません

一度 `lib` の列を作って Library の該当コミットへ飛ばしてみましたが、外しました。今ある提出はどれも `common.hpp` しか include していないので、全行が同じ値を指すうえ、その中身は測定に関わっていません。

`library_sha` はキーに入っていません。ライブラリの影響は `submission_hash` を通して入るので、測り直しが起きるかどうかを決めるのはヘッダの中身であって、この値ではありません。記録に持っている理由は再現です。CI がライブラリを pin せず HEAD で取るぶんを、あとから git で戻せるようにしてあります。

出すなら、記録の `includes` と組にして、提出が include しているヘッダを 1 本ずつ `Library/blob/<library_sha>/<path>` へ飛ばす形です。押したいのはリポジトリのルートではなくそこです。`mylib/...` を使う提出が入ってから作ってください。ライブラリの URL はこのリポジトリが持たないので、そのときは呼ぶ側から渡すことになります。

### gitignore の site/ は pj/site/ にも当たります

書き先を `site/` にしたので `.gitignore` に `site/` を足したら、`pj/site/` まで無視されてパッケージが commit から漏れかけました。`/site/` に直してあります。

## M7 の実装の記録

`mylib/...` を include する提出を 2 本入れて、ライブラリの経路に初めて電気を通しました。`lib/` を clone して `-I` に足す仕組みは M4 の時点で組んでありましたが、それを使う提出が 1 本も無かったので一度も通っていませんでした。提出は `problems/yosupo-point-add-range-sum/submissions/` に置きました。`lib-bit.hpp` が使うのは `mylib/data_structure/BinaryIndexedTree.hpp` です。`lib-segtree.hpp` は `mylib/data_structure/SegmentTree.hpp` を使います。手書きの `fenwick.hpp` と `segtree.hpp` が同じ構造なので、そのまま比較になります。4 環境とも AC でした。

### 閉包はライブラリの中まで辿れています

`lib-segtree.hpp` の記録の `includes` が 3 つになりました。`mylib/data_structure/SegmentTree.hpp`、`mylib/internal/detection_idiom.hpp`、`pj.hpp` です。`SegmentTree.hpp` が internal のヘッダを引いているので、ライブラリの中で 2 段辿っています。ここが繋がっていることが「ライブラリを直したら、それに依存する提出だけ測り直しになる」の前提です。

### 手元の lib/ は自分で clone します

`.gitignore` に `/lib/` を足しました。CI は実行のたびに clone しますが手元には無いので、無いままだとライブラリを使う提出が全部 include 未解決になります。

```
git clone --depth=1 https://github.com/hashiryo/Library.git lib
```

### include を解決できない提出は走らせません

判定は clone の成否ではなく閉包の `unresolved` で行います。閉包が欠けたままキーを作ると別の意味のキーになりますし、そのまま走らせても CE の記録が残るだけだからです。ライブラリと関係ない綴り間違いも同じ扱いになりますが、走らせても CE なので困りません。飛ばした分は `Plan` の `blocked` に入って、要約に「include 未解決 N 件」、`--dry-run` では `block` の行として出ます。

### local は Library の一部をコンパイルできません

`lib-bit.hpp` は `local` (Apple clang + libc++) で CE になります。`BinaryIndexedTree.hpp` が `std::__lg` を使っていて、これは libstdc++ の拡張なので libc++ にありません。修飾名なのでテンプレートの定義時に解決されて、`find` を呼んでいなくても落ちます。CI の 4 環境は Ubuntu の clang も libstdc++ を使うので通ります。

`mylib` 146 ファイルのうち、`std::__lg` を使うのは `BinaryIndexedTree.hpp` と `DiscreteLogarithm.hpp` の 2 つです。修飾なしの `__lg` は `Factors.hpp` と `mod_kth_root.hpp` の 2 つです。Library 側に `include/clang_compat.hpp` があって、`-include` で渡せば修飾なしの 2 つは埋まりますが、`std::` 付きの方は埋まりません。ここは Library の側の話なので、このリポジトリでは直しません。移植で Apple clang だけ通らない提出が出たら、この 4 ファイルを疑ってください。

### 最初の比較

`algo` の最大ケースをミリ秒で並べます。x64 は AMD EPYC 7763、arm は Neoverse-N2 です。

| 提出 | x64-gcc | x64-clang | arm-gcc | arm-clang |
| --- | --- | --- | --- | --- |
| `fenwick.hpp` | 16.43 | 20.33 | 21.45 | 19.97 |
| `lib-bit.hpp` | 18.03 | 17.47 | 19.11 | 19.21 |
| `segtree.hpp` | 38.32 | 34.07 | 30.92 | 29.44 |
| `lib-segtree.hpp` | 38.70 | 39.38 | 51.29 | 34.88 |

BIT は手書きとライブラリがほぼ互角で、どちらが速いかは環境によって入れ替わります。SegmentTree はライブラリの方が遅く、arm-gcc では 1.66 倍です。差の出どころは調べていませんが、点加算に使っている `mul` が `set(i, op(get(i), x))` の形で `get` を 1 回余分に通るあたりが素直な疑いです。こういう差が判定として出てくるのが、このリポジトリで欲しかったものです。

## M8 の実装の記録

順位表の 1 行が、現行と参考と未計測の 3 つに分かれました。現行は今のソースで測った記録です。参考は測ってからソースが変わった記録です。未計測はその組でまだ一度も測っていない提出です。参考と未計測はどちらも「次にその CPU モデルのランナーが当たったら埋まる」という同じ意味を持ちます。

### 機械側の値は記録から借ります

M6 では「`collect` のジョブは `cases_hash` も CPU モデルも持たないのでキーを作れない」として判定を見送っていました。ただ、キーを一から作る必要はありません。知りたいのは「測ってからソース側が変わったか」だけなので、機械側の値はその記録から借りれば足ります。

借りるのは `cases_hash`、`compiler_version`、`cpu_model`、`env` の 4 つです。作り直すのは `submission_hash`、`harness_hash`、`problem_hash`、`cxxflags` の 4 つで、どれもリポジトリと `lib/` だけから出ます。組み直したキーが記録の `key` と違えば、その記録は古いということです。

キーの計算は `pj.key.compute` をそのまま呼びます。同じ判定を二度書くと、片方を直し忘れたときに記録の意味が静かにずれます。記録のスキーマは変えていません。`harness_hash` と `problem_hash` は記録に入っていませんが、キーそのものと突き合わせるので要らないからです。

### 判定できないときは現行の側に倒します

3 つの場合に `None` を返します。提出のファイルが消えている、閉包に未解決の include がある、環境が `environments.toml` から消えている、です。分からないものを古い側に倒すと、ライブラリを取れなかった回に表が丸ごと参考になります。サイトの側は `None` を現行と同じに描きます。

閉包を作り直すのにライブラリが要るので、`collect` のジョブでも `lib/` を clone します。手元で `pj site build` を叩いたときは、`lib/` が無ければ警告を出します。

### 見逃すもの

`cases_hash` も記録から借りているので、テストデータだけが上流で作り直された場合は古いと分かりません。次に `run` が走ったときに測り直されて入れ替わるので、表示が遅れるだけで済みます。

`run` と `collect` は別のジョブなので、その間にライブラリが動くと、たった今測った記録が参考として出ます。これは誤判定ではありません。今のライブラリで測り直せば別の値になるので、古いという表示の方が正しいです。

### 現行のキーの記録を優先します

畳むときは、現行のキーの記録があれば時刻が古くてもそちらを採ります。書き換えたものを元に戻すと前のキーが復活するので、いちばん新しい記録が現行とは限りません。現行が 1 つも無ければ、これまでどおりいちばん新しいキーを採って、参考として出します。

### 未計測の行は列を残します

これまでは 1 セルを `colSpan` で潰していましたが、列ごとに `-` を置く形にしました。行ごと落とさないのは前からで、落とすと「まだそのランナーが当たっていないだけ」なのか「提出が無い」のかを見分けられなくなるからです。

### 表示

参考の行は薄くして、提出の名前の後ろに「参考」の印を付けます。状態の色も落とすので、AC が緑のまま並ぶことはありません。並べ替えはどの列を押しても参考が下に落ちます。順位に混ぜないためです。

提出の列は、名前を span に包んで flex で並べます。セル全体に ellipsis を掛けると、名前が長いときに後ろの印の方から先に消えるからです。flex は `td` ではなく中の `div` に掛けます。`td` を flex にすると table-cell でなくなって行の高さに伸びず、この列だけ罫線が上にずれます。

列に出す名前からは `submissions/` を落としました。全行で同じなので情報がありませんし、その幅のぶん印が入らなくなります。元のパスは `title` と、ソースの列のリンクに残ります。

環境と CPU モデルの下に「現行 N / 参考 M / 未計測 K」を出します。問題一覧の方は、計測済みの分子を現行だけにして、参考の列を足しました。

### 入れた時点で 10 件が参考でした

手元で作り直したところ、189 件の記録のうち 10 件が参考になりました。中身は x64-gcc の EPYC 9V45 と 9V74 に残っていた 9 月 19 日の記録で、ハーネスを `harness/pj.hpp` に出した回とメモリの測り方を変えた回よりも前のものです。x86 は 4 つのモデルに散るので、その 2 つにまだランナーが当たっていません。

これは「汚染されたメモリ記録が一部の CPU モデルに残っている」として宿題に挙げていたものと同じ記録です。現役の顔で出ていたものが、参考として出るようになりました。

## M9 の実装の記録

`hashiryo/Library` の `master` に `mylib/` を含む push があると、向こうの `.github/workflows/procon-judge.yml` が `repository_dispatch` を投げます。こちらの `judge.yml` はそれで走ります。種別は `library-push` で、`judge.yml` の側でもその種別だけを受けるようにしてあります。トークンは Library の secret `JUDGE_DISPATCH_TOKEN` です。

### dispatches の口に要るのは Contents の write です

fine-grained な PAT で `POST /repos/{owner}/{repo}/dispatches` を叩くには、対象リポジトリの Contents に write が要ります。Actions ではありません。`POST /repos/{owner}/{repo}/actions/workflows/{id}/dispatches` の方が Actions の write です。

つまり `repository_dispatch` を使う限り、このトークンは procon-judge に push もできます。Actions だけに絞りたければ、`judge.yml` が `workflow_dispatch` も受けているので、そちらに切り替えて Contents を read に落とせます。今は設計どおり `repository_dispatch` の側にしてあります。

### 期限は付けていません

期限を付けると、切れた日に合図が黙って止まります。03:00 の定期実行が記録の方は拾ってしまうので、止まったことに気づく手がかりが残りません。権限が 1 リポジトリぶんしかないので、期限なしで置いて、要らなくなったら消す形にしました。

### mylib/ だけを見ます

`paths` を `mylib/**` に絞ってあります。提出の include 閉包に入るのはそこだけなので、`test/` や docs を直してもこちらのキーは動きません。絞らないと、verify の結果をコミットするたびに 6 ジョブが空回りします。取りこぼしても定期実行が拾います。

## M10 の実装の記録

ジョブが `test` / `plan` / `run` / `collect` の 4 つになりました。`plan` は `results` と問題定義と `lib/` を読んで、環境ごとに何本立てるかを決め、マトリクスの JSON を出します。`run` は `fromJSON` でそれを受けます。

### マトリクスに束を入れませんでした

`plan` が渡すのは `env`、`runs_on`、`toolchain`、`job`、`jobs`、`budget` だけです。束は入れていません。GitHub はマトリクスの値からジョブ名を作るので、問題 id が何十個も並ぶと一覧が読めなくなります。問題が増えるとマトリクス自体も膨らみます。

代わりに `run` が同じ並びを組み直します。並びは記録と問題定義と `lib/` だけから決まるので、同じものを読めば同じ順になります。`pj.plan.for_env` は `run` からも呼びます。順番を決める規則が 2 か所に増えるのを避けるためです。ジョブ名は `name:` で `run (x64-gcc #0)` の形に上書きしました。

`plan` が `lib/` を取った時点と `run` が取った時点の間に Library が動くと、並びがずれてジョブの担当が重なったり抜けたりします。重なるぶんは同じキーの 2 本目の記録になるだけで、抜けたぶんは次の実行が拾います。踏み込みがもともと同じずれを前提にしているので、ここを揃えるための機構は足していません。

### 空のマトリクスは受け付けてもらえません

やることが 0 件だと `include` が空になりますが、空のマトリクスは GitHub がエラーにします。`plan` が `any` も出して、`run` の `if` で止めています。

`collect` は `run` が 1 本も立たなくても走ります。`lib/` が動けば参考かどうかの判定が変わるので、記録が増えなくてもサイトは作り直す必要があります。

### アーティファクトの名前に番号を足しました

同じ環境で複数のジョブが記録を上げるので、名前が `records-<env>` のままだと衝突します。`records-<env>-<job>` にしました。`collect` は名前で絞らずに全部落としているので、そちらは変わりません。

### キーの recipe を 1 か所に寄せました

`plan` がやりたいのは「このモデルで測ったらどのキーになるか」です。M8 の `Freshness` がやっている「測ってからソース側が変わったか」と、機械側の値をどこから持ってくるかが違うだけで、キーの作り方は同じです。`Freshness.key_for` を切り出して、`current` もそれを呼ぶ形にしました。`pj/site/freshness.py` は `pj/freshness.py` へ移しました。サイトだけの道具ではなくなったからです。

### 手書きのマトリクスが消えました

`judge.yml` に 4 環境を書いていたのをやめて、`environments.toml` から出すようにしました。ツールチェインの名前は `cxx` から決めます (`g++-15` なら `gcc`)。二重持ちが無くなったので、環境を足して `judge.yml` を忘れる事故は起きません。代わりに、その環境のコンパイラを入れる step が `judge.yml` にあるかどうかを pytest で見ています。

### budget は件数で、束の途中でも止めます

`--budget` は走らせる提出の件数です。6 時間で打ち切られるとその回に測ったぶんを丸ごと落とすので、大きい束に当たったときこそ効いてほしく、束の切れ目は待ちません。止めた先は `held` に入れて件数だけ報告します。残りは次の実行が同じ順で拾います。

最初の値は 1 環境あたり最大 5 本、1 ジョブ 100 件でした。5 本は同時実行 20 を 4 環境で割った数です。100 件は raw 移行の CI の実測から決めました。40 件で 60 分前後 (保管庫からの取得と mylib のコンパイルが大半) だったので、100 件なら 2.5 時間ほどで 6 時間の上限の半分に収まる見込みでした。

### 時間でも止めて、本数は全モデルの合計で数えます

2026-09-22 に見直しました。100 件のジョブの実測は 7 分から 27 分 (外れ値 49 分) で、6 時間に対して余りすぎていました。一方で未計測は x64 の 1 環境で 1 モデルあたり 770 件ほど、モデルが 6 つあるので 4600 件ほどあり、1 run で 5 本 × 100 件 = 500 件しか減らないので、埋まるまで 10 run かかる計算でした。

変えたのは 3 つです。件数の上限を 200 にし、それとは別に時間の上限 `--minutes` (既定 240 分) を run に渡して、過ぎたら次の提出に手を付けないようにしました。走っている 1 本は止めません。最後の 1 本の最悪 (ケース数 × `tle_sec`、十数分) と upload を足しても 6 時間に届きません。件数だけでは重い束に当たったときの時間を抑えられないので、件数を上げるには時間の上限が要りました。ジョブの本数は 1 モデルあたりの平均ではなく全モデルを合わせた未計測の件数で数え、1 環境 8 本を上限にしました。平均で数えると、モデルが 6 つある x64 では本当の仕事の 1/6 しか見ないことになります。schedule は 03:00 JST に 15:00 JST を足して 1 日 2 回にしました。やることが 0 件なら run は立たず、plan だけで 2 分ほどで終わります。

### 見積もりは順番にしか使いません

費用の見積もりは、記録のある提出は `time_total_ms` を、無い提出は `case_count × tle_sec` を使います。使い道は束を重い順に並べることだけで、何を測るかには影響しません。外れても順番が少し入れ替わるだけです。記録が 1 件も無い問題はケース数も分からないので、20 ケースと置いています。

## 正規化とファイル別ハッシュの実装の記録

`normalize` をトークン単位にして、記録に `harness_hash`、`problem_hash`、`file_hashes` を足しました。1 回の push にまとめたのは、正規化を変えると全記録が一度参考に落ちて測り直しになるので、そのとき作られる新しい記録に最初からファイル別ハッシュが載るようにするためです。

### 字句解析器は正規表現 1 本です

候補を名前付きグループで並べた正規表現を `finditer` で流して、空白とコメントのグループを捨てます。候補の順番が要点です。指令の行は行頭でしか始まらないので、行頭の空白を空白として食う前に見ます。リテラルは識別子より先に見て、`u8"..."` や `R"(...)"` の接頭辞を識別子と切り離しません。数は記号より先に見て、`.5` を `.` と `5` に分けません。数の形はプリプロセッサの pp-number の文法に揃えてあり、`1e+5` や `0x1p-3` や `1'000'000` が 1 トークンになります。

指令の行だけは正規表現の外で潰します。引用符の中の `//` や `/*` はコメントではないので、リテラルを丸ごと飛ばしながらコメントを外し、空白を 1 つにします。`#  define` と `#define` は同じなので、`#` の直後の空白だけは消します。

テストは「同じになるべき組」と「違うままであるべき組」を並べてあります。前者はコメントの追加、整形、行継続、CRLF で、後者は上に書いた 3 つの罠です。

### 参考の理由は Freshness.diff が出します

`Freshness.diff(record)` が `Diff` を返します。中身が変わったファイル、今の閉包に増えたファイル、閉包から消えたファイル、それとファイル以外の成分 (`problem` と `cxxflags`) です。機械側の値は記録から借りているので、違いうる成分はこの 4 つに限られます。`file_hashes` を持たない古い記録では名指しができないので、空の `Diff` になります。

サイトでは `describe_diff` が 1 行の日本語にします。「変更 mylib/algebra/ModInt.hpp / 追加 pj.hpp / problem.toml が変わった」の形で、空の `Diff` は「理由は記録に無い」です。提出ページでは表の下に「参考の理由」としてまとめて出し、順位表では印の title に出します。

### run.describe を切り出しました

記録のうち走らせる前から決まる項目 (キーとその成分) を `describe(job)` にまとめました。テストが走らせずに「今のソースをこの条件で測った記録」を作れるようにするためで、`execute_job` も同じものを使います。

## 提出ページと逆引き JSON の実装の記録

`pj site build` が提出ページ (`submissions/<id>/<name>.html`) とヘッダごとの逆引き (`data/headers/<ラベル>.json`) を出すようにしました。順位表の提出名からも飛べます。中身の判断は「サイト」の節に書いたので、ここには作りで気づいたことだけを置きます。

### 提出ページは Python で HTML まで埋めます

順位表は環境と CPU モデルの切り替えがあるので JSON を JavaScript が描きますが、提出ページには切り替えが無く、行も 10 本ほどです。テンプレートの目印を Python で埋めて終わりにしました。名前は提出のパスから `submissions/` と拡張子を落としたもので、`submissions/lib-segtree.hpp` なら `lib-segtree.html` です。1 つの問題に同じ名前で拡張子だけ違う提出は置かない前提です。

### テンプレートの照合は埋める前にします

`_render` は埋めたあとの文字列に `{{...}}` が残っていないかを見ていました。提出のソースを埋めるようになると、`x{{1}}` のような波括弧の並びが普通に出るので、それに引っかかります。埋める前にテンプレートの目印の集合と渡された値の集合を照らし合わせる形に変えました。置き換えも 1 回の走査で行い、埋めた値の中に目印の形があっても二度読みしません。

### CPU モデルの列に幅を与えます

順位表は提出のパスがいちばん長いので、そこに残りの幅を渡しています。提出ページでは CPU モデルの名前 (`AMD EPYC 7763 64-Core Processor`) がいちばん長いので、そちらに 210px を与えて、参考の理由が入る「現行」の列に残りを渡しました。理由は長くなりうるので省略記号にして、元の文は title に残します。

### include の欄は閉包を辿り直して作ります

記録の `includes` はラベルだけなので、リンク先を決めるにはファイルの位置が要ります。`include.closure` をもう一度呼んでファイルを取り、`include.direct` で提出が直接 include しているものを直接の側に置きます。解決できなかった include は「見つからない」として並べます。

### 逆引き JSON の形

```json
{
  "schema": 1,
  "header": "mylib/data_structure/SegmentTree.hpp",
  "library": "Library",
  "generated_at": "2026-09-21T04:36:22Z",
  "judge_sha": "c100b9c...",
  "library_sha": "8c7d4d1...",
  "site": "https://hashiryo.github.io/procon-judge/",
  "environments": ["x64-gcc", "x64-clang", "arm-gcc", "arm-clang"],
  "submissions": [
    {
      "problem": "yosupo-point-add-range-sum",
      "title": "Point Add Range Sum",
      "submission": "submissions/lib-segtree.hpp",
      "direct": true,
      "page": "submissions/yosupo-point-add-range-sum/lib-segtree.html",
      "problem_page": "problems/yosupo-point-add-range-sum.html",
      "envs": [
        {"env": "x64-gcc", "status": "AC", "current": true, "models": 3, "algo_ns": 38700000},
        {"env": "arm-clang", "status": null, "current": null, "models": 0, "algo_ns": null}
      ]
    }
  ]
}
```

`envs` は環境ごとに CPU モデルを畳んだものです。状態は全部 AC のときだけ AC で、そうでなければ最初の AC でない状態、現行は全部現行のときだけ `true`、1 つでも参考なら `false` です。記録の無い環境は `status` が `null` で並びます。ライブラリ側のサイトが表の 1 セルにする単位に合わせてあります。モデルごとの明細は提出ページにあります。

### libraries.toml

```toml
[[library]]
name = "Library"
prefix = "mylib/"
page = "https://hashiryo.github.io/Library/{stem}.html"
source = "https://github.com/hashiryo/Library/blob/{sha}/mylib/{path}"
```

`{path}` はラベルから接頭辞を除いたもの、`{stem}` はさらに拡張子を落としたもの、`{sha}` は `lib/` の HEAD (無ければ `HEAD`) です。ファイルが無ければライブラリは無いものとして動きます。接頭辞が重なるときは長い方が勝ちます。

### 順位表の側

`data/problems/<id>.json` に `pages` (提出 → 提出ページのパス)、`url` (元の問題のページ)、行ごとの `reason` を足しました。`url` は取得元が判定サイトのときだけ分かります。順位表の提出名はリンクになり、参考の印の title に理由が入ります。順位表の見出しの行には、問題のディレクトリ (`problems/<id>`) を GitHub で開くリンクも置いています。列の幅は状態の列を 180px から 110px に詰めて、提出の名前が読める幅を 200px から 336px に広げました。

## exit_code の実装の記録

`compare.kind = "exit_code"` を入れました。最初の `raw` の問題でもあります。題材は Library の `test/sample_test/` にある STANDALONE のテストで、提出が入力と期待出力を自分で持っていて `assert` で確かめます。ファイルはそのまま `submissions/<名前>.cpp` に置けます。`competitive-verifier` のコメント行だけ落としています。

### 1 回だけ走らせて、ケースの名前は self にします

テストケースが無いので、空の標準入力で 1 回走らせます。TLE と MLE と RE の判定は `judge_case` と同じで、そこを `judge_limits` に切り出して共有しました。走り切って終了コードが 0 なら AC です。記録の形は他の問題と同じにして、`case_count` を 1、ケースの名前を `self` にしています。`failed_cases` とサイトの「失敗」の節がそのまま使えます。計測区間が無いので `algo_time_ns` は残りません。空打ちは他の問題と同じにしています。

### 落ちたときは stderr の末尾を添えます

期待出力が無いので、落ちたときの手がかりは stderr しかありません。`assert` の文言がそこに出るので、`failed_case.detail` に `signal 6` などの状態に続けて stderr の末尾を入れます。長さの上限は差分と同じ `DIFF_HEAD_CHARS` です。

### 組み合わせは source = "none" に限ります

`exit_code` は `testdata.source = "none"` の問題でしか使えないようにしました。テストデータがあるのに終了コードだけ見る形に意味が無いからです。逆に `none` で使える比較は `compile_only` と `exit_code` の 2 つです。

### サイトの表示

取得元のラベルは `無し (自己検証)` で、注意書きは「提出が自分で持っている入出力で検証して、終了コード 0 で走り切れば AC」です。`compile_only` の「コンパイルのみ」と区別しないと、走らせていないように読めます。`pj repro` も 1 回走らせて、落ちたときは stderr の場所を出します。

## raw 移行の実装の記録

Library の `test/**/*.test.cpp` のうち、ハーネスを書いて移していないものを `kind = "raw"` で取り込みました。`pj problems import` が competitive-verifier の注釈 (`PROBLEM` の URL、`TLE`、`MLE`、`ERROR`、`STANDALONE`、`IGNORE`) から id と取得元と制限と比較を出し、注釈の行だけを落としたファイルを `submissions/lib.cpp` に置きます (同じ問題に実装が複数あれば `lib-<実装>.cpp`)。内訳は yosupo 102 問、AOJ 142 問、yukicoder 104 問、AtCoder 133 問 (compile_only、うち 28 問は実装が複数)、自己検証 48 問 (alone 16 + sample_test 32、exit_code)、手で取り込む判定サイト 40 問 (manual) です。題名は Library Checker が `info.toml`、AOJ と yukicoder が API、AtCoder がページの `<title>` で、manual の問題は id のままです。

`ERROR` の注釈は `compare.kind = "float"` になり、絶対と相対の両方に同じ値を置きます。`float` の許容誤差は問題定義のハッシュに入りますが、float の問題にだけ足すので、ほかの問題の鍵は動きません。AOJ の制限は一覧の API (`problemTimeLimit` は秒、`problemMemoryLimit` は KB) から取り、5 秒と 512 MB より下には締めません。

### 制限は判定サイトの値にしました

Library の注釈には `TLE 0.5` や `MLE 64` のように締めた値が書いてありますが、採りません。ここは劣化を pass/fail で見る場所ではなく、時間とメモリは数字として記録に残ります。締めた値で TLE にすると、1 つのキーは 1 回しか測られないので、たまたま遅かった回の TLE がキーが変わるまで残ります。`tle_sec` は `info.toml` の `timelimit`、`mle_mb` は Library Checker の 1024 MB です。ただし注釈の方が大きいときは注釈に合わせます。Library が通ると知っている値だからです (`enumerate_primes` は 1.1 GB 使うので 2048)。

### convolution_mod_large は入れていません

テストデータが 12 GB 出ます (54 ケース、N と M が 2^24)。Release のアセットは 2 GiB までなので保管できず、ランナーの disk (14 GB ほど) にも収まりません。Library の verify に残したままです。

### pin と保管庫と actions/cache

この取り込みで Library Checker の問題が 26 問から 128 問になり、テストデータが 3 GB から 12 GB を超えます。actions/cache の塊は 10 GB に載らず、載っても 1 問しか要らない回に全部復元することになります。保管庫に 1 問 1 アセットで置いて要る問題だけ落とし、測り終えたら捨てる形にしました。保管庫に入れると中身が凍るので、`library-checker-problems` のコミットを `testdata.toml` に固定して、manifest に生成に使ったコミットを書き、pin と違えば作り直して置き換えます。手元の `.cache/testcases/` は消しません。

### 生成したケースは clone から移します

`generate.py` は問題のディレクトリの `in/` と `out/` に書きます。コピーすると同じものを 2 度持つので、キャッシュへ移して clone の側は空にします。`generate.py` は in と out が揃っていなければ作り直すので、次に要るときも困りません。

## local と gf2-64 のベンチの実装の記録

`source = "local"` を実装して (`pj/fetch/local.py`)、最初の問題として旧 judge の `gf2-64` の 10 問を移しました。`gen.py` を seed 1 つずつ `uv run --script` で走らせて入力を作り、参照実装をハーネスと一緒に組んで期待出力を作ります。ここまで local の問題が 1 つも無かったので、設計にはあって実装が無い状態でした。

### 問題をまたぐヘッダは problems/_shared/ に置きます

旧 judge の `gf2-64/_shared/` (共通の型と定数、`sq` や `frob` の部品、塔の基底変換の表) を `problems/_shared/gf2-64/` に移しました。`-I` を足すとキーが動くので足さず、提出は `../../_shared/gf2-64/...` の相対パスで include します。閉包の解決は include した側のディレクトリから探すので、この形でも辿れます。ラベルは `problems/_shared/gf2-64/sq.hpp` のようにリポジトリからの相対になり、提出ページでは GitHub へ飛びます。`_common.hpp` の `<bits/stdc++.h>` は Apple clang に無いので、名指しの include に置き換えました。

### 代表だけを移しました

旧 judge の 190 本を全部は移しません。問題ごとに `reference` と手法の族ごとの最良 (旧 judge の記録 17705 件から x64-g++ の最小値で選びました) を 3 本から 8 本、合わせて 47 本です。`log` には reference が無く、素直な BSGS の `pclmul.hpp` を参照実装にしました。インターフェースは旧 judge の `GF2_64Op::run` のままにして、提出の書き換えを include のパスだけにしています。Library に GF(2^64) の型ができたら `lib-*.hpp` を並べるのが、型を登録する門になります (roadmap)。

### 制限

`tle_sec` は mul / sq / frob が 5 秒、div / pow / sqrt / log が 10 秒です。旧 judge は全部 5 秒でしたが、div の参照実装が x64 で 4.4 秒かかっていて、遅いマシンに当たると参照実装そのものが TLE になります。

## 旧 judge の通常の問題の移植の記録

旧 judge (hashiryo/judge) の `algos/` を持つ 35 問 (yosupo 33、自作 2) を base の問題として移しました。うち 23 問は raw で取り込んだ Library の verify と同じ問題だったので、その問題を base に作り替え、`lib.cpp` を廃止して Library を使う提出を `lib.hpp` に書き直しました。書き直しは 15 問で、旧 judge に Library を使う `mine.hpp` があった問題はそれを `lib.hpp` に改名しただけです。`yosupo-subset-convolution` は既にハーネスがあったので、旧 judge の 2 本にハーネスの形に合わせる薄い層を足しました。

### 機械的に直したもの

`base.cpp` は `algos/_common.hpp` を `pj.hpp` と `common.hpp` に、`ALGO_HPP` を `SUBMISSION_HPP` に、`ALGO_TIME_NS` の `fprintf` を末尾の `report_metrics` に置き換えました。`algos/_common.hpp` は問題ごとの `common.hpp` になり、`<bits/stdc++.h>` は名指しの include に変わりました。提出は `algos/*.hpp` をそのまま `submissions/` に移し、include のパスだけ直しています (`_common.hpp` → `../common.hpp`、gf2-64 の共有ヘッダ → `../../_shared/gf2-64/`)。インターフェース (`Det::run(n, a)` のような static 関数) は旧 judge のままです。`yosupo-unionfind` だけは提出の型名が Library の `UnionFind` とぶつかるので `Solver` に変えました。

### 自作の 2 問は local にしました

`mod-inv-prime` と `warshall-floyd` は判定サイトの問題ではないので `source = "local"` です。`warshall-floyd` の入力は `V W seed` の 3 つで距離行列はハーネスが LCG で作るので、`gen.py` は旧 judge の 15 ケースの組をそのまま返します。参照実装は旧 judge の素朴な実装 (`naive_fermat` と `naive`) です。

### 制限

`tle_sec` と `mle_mb` は、raw で入っていた問題は既存の値と旧 judge の値の大きい方、新しく入った問題は Library Checker の `timelimit` と旧 judge の値の大きい方にしました。

## modulo-test の族の移植の記録

旧 judge の `modulo-test` (14 問)、`1word-mod` (4 問)、`gcd-test` (1 問)、`modpow-test` (2 問) を `source = "local"` の base の問題として移しました。gf2-64 と同じ「ハーネス + 代表」の形です。旧 judge の 202 本のうち 100 本を写し、Library の実装をそのまま使う `lib-*.hpp` を 41 本足して、21 問 141 本になりました。id は族の名前と旧 judge の小問題の名前をつなげて、`modulo-test-runtime-30` のようにしています。`^` と `+` は URL とアセット名に使いたくないので、`2^31-1` は `2pow31-1` と、`1e9+7` は `1000000007` と書きました。4 族の `algos/_common.hpp` は typedef だけで中身が同じだったので、`problems/_shared/modulo-test/_common.hpp` の 1 枚にまとめました。`base.cpp` の書き換えは通常の 35 問と同じ機械的なものです。インターフェース (`MP` の `set` / `get` / `mul` / `plus`、`DIV::mod`、`G::gcd`、`MP::pow`) は旧 judge のままです。

### 代表の選び方

旧 judge の記録 24200 件から、小問題と提出ごとに x64-g++ で AC した `algo_time_max_ns` の最小値を取りました。提出は名前の接頭辞で手法の族に分けます。族は naive / barrett / div2by1 / long_double / montgomery / montgomery_even / plantard / anton / lemire などです。残すのは参照実装 (素の naive) と、族ごとの最速 1 本です。AC の記録が無い族は落としています (runtime-32 の lemire は WA でした)。static-2^40-1 の barrett_reduction も WA でしたが、これは次に書くハーネスの間違いが原因で、しかも `MP_Br` の写しそのものなので `lib-br.hpp` で足りています。`_O3` の変種は pragma だけの差で時間も同じなので除きました。static-998244353 では `asm volatile` で定数の畳み込みを止めた `_volatile` の変種が barrett / montgomery / plantard の 3 族で最速でした。ここは素の版と両方を残しています。静的な mod では畳み込みが逆に遅くしていた、という旧 judge の観察を消さないためです。`long_double` は旧 judge の 5 秒で TLE になっていただけなので、制限を 10 秒にして残しました。arm では long double が 128 bit のソフトウェア実装なので TLE のままになります。

### 旧 judge の static の 2 問は法が間違っていました

static-2^64-1 は 4 本が全部 WA でした。原因は期待出力を作る `gen/ref.cpp` が MOD を 2^61-1 と書き間違えていたことで、提出側は正しく動いていました。新しい形では `naive64.hpp` 自身が期待出力を作るので、そのまま移しています。

static-2^40-1 は逆に `base.cpp` の側が `MOD= (1ull << 32) - 1` のままで (2^32-1 の写しの直し忘れ)、実際には 2^32-1 で測っていました。入力は 2^40-1 未満で作っているので法より大きい値が `set` に入り、`set` が値を縮めない Barrett だけが壊れて WA、`%` や Montgomery の `set` で縮める他の 3 本は「正しく」通っていたことになります。移した `base.cpp` は 2^40-1 に直しました。旧 judge のこの問題の記録は 2^32-1 のものなので、代表の選び方には使えません (3 本しか無いので全部残しています)。

この直しで、手元のキャッシュの置き場が `base.cpp` を見ていないことに気づきました。`local` の期待出力は参照実装をハーネスと一緒に組んで作るのに、置き場は `gen.py` と参照実装と `count` だけで決めていたので、法を直しても古い期待出力が使われました。`kind = "base"` のときは `base.cpp` の内容も混ぜるようにしました (`pj/fetch/__init__.py`)。CI は毎回生成するので影響は手元だけです。

### Library の実装は写しではなく include します

「mylib が採用したもの」は `lib-na.hpp` / `lib-mo32.hpp` / `lib-mo64.hpp` / `lib-br.hpp` / `lib-d2b1-1.hpp` / `lib-d2b1-2.hpp` です。`mylib/internal/Remainder.hpp` の `MP_*` を `using MP=` で名指しします。その問題の mod の範囲に合う型だけを置いています。Montgomery は奇数だけ、`MP_Mo32` は `ModInt` が使う 2^30 未満だけ、`MP_Br` は 2^20 < mod <= 2^41 です。modpow-test は `MP_Mo32` / `MP_Mo64` と `math_internal::pow` を包む薄い `struct MP`、gcd-test は `binary_gcd` を包む `struct G` です。旧 judge の `binary_lib.hpp` (mylib の写し) はこれに置き換えました。閉包に `mylib/internal/Remainder.hpp` が入るので、Library 側でここを直すと M9 で測り直され、Library のヘッダページには「このヘッダを使う提出」として並びます。

### ジェネレータは旧 judge の入力をそのまま出します

旧 judge の `gen/make_inputs.py` は手書きのケースと `random.Random(32)` (static は種が mod) の乱数ケースを一度に書き出していました。新しい `gen.py` は同じ順番で全ケースを組み立ててから seed 番目だけを出すので、中身は旧 judge の `testcases/*.in` と一致します (移すときに全問題で照合しました)。1word-mod、gcd-test、modpow-test の入力は LCG の種と個数だけです。実際の列はハーネスが計測の前に作ります。

### 制限

`tle_sec` は 32 bit の modulo-test が 10 秒、1word-mod が 5 秒、gcd-test と modpow-test が 10 秒です。64 bit の modulo-test (runtime-40 / 62 / 64、static-2^40-1 / 2^61-1 / 2^64-1) は 15 秒にしました。旧 judge は書いてある問題が 5 秒か 10 秒で、書いていない問題は 10 秒でした。64 bit を 15 秒にしたのは、参照実装の naive64 が arm で 6 秒から 8.5 秒かかるからです。遅いモデルに当たると参照実装が TLE になります。`mle_mb` は旧 judge の値のままです (1word-mod は計測の前に作る a の配列ぶんで 1024 と 2048)。

## AppleDouble の掃除の記録

保管庫のアーカイブに `._<ケース名>.in` / `.out` が入っていました。macOS の tar が拡張属性を AppleDouble の別エントリとして書いたものです。macOS の `tar -tf` では見えません (BSD tar が畳んで見せます)。Python の tarfile で数えると、yosupo-convolution-f2-64 は 212 エントリのうち 106 が `._` でした。CI の Linux で展開すると実ファイルになり、`collect_cases` が `*.in` として拾います。51 ケースの問題が 102 ケースになって `cases_hash` が変わり、提出は偽の入力を読んで WA / RE / TLE / MLE になります。results ブランチには 352 問 3180 件の記録がこの形で入っていました (WA 1892 / RE 1149 / TLE 127 / MLE 12)。arm-gcc のジョブが 3 run 続けて「runner が shutdown signal を受けた」で死んだのも同じ原因です。yosupo-convolution-f2-64 の提出が偽の入力を読んで暴走し、ランナーの VM ごと落ちています。

直したのは 2 か所です。ケースの列挙 (`collect_cases`) で `.` 始まりを飛ばすことと、アーカイブの作成と展開を tar コマンドから Python の tarfile に替えて `.` 始まりのエントリを入れも出しもしないことです。macOS の tar は `._` を拡張属性だと解釈して当てに行き、展開に失敗することがあります。Linux の tar は実ファイルにします。どちらにも任せません。既に上げてあるアーカイブはそのままで動きます。該当の問題は `cases_hash` が本来の値に戻るのでキーが変わり、次の run から自動で測り直されます。直した run (6030ffc) は 20 ジョブ全部が通り、yosupo-convolution-f2-64 は 51 ケースの本来の `cases_hash` で AC が並び始めました。

偽のケースで測った記録は `results` から消しました (355 問 3571 件、205 問は記録が 0 になって未計測に戻りました)。基準は「`failed_cases` に `._` 始まりの名前を含む記録と同じ (問題, `cases_hash`) を持つ記録」で、その `cases_hash` は偽のケースを数えた値なので同じ値の記録は全部偽のケースで測っています。消す前に、対象に AC が 1 件も無いことを確かめました。参考として残すと、キーに CPU モデルが入っているぶん、同じモデルに当たるまで嘘の WA が灰色で残り続けるからです。`results` への手作業の push は collect の push と重なると片方が弾かれるので、run の合間にやりました。

## LOJ の取得元の実装の記録

LOJ (LibreOJ) の 23 問は `source = "manual"` で入れていましたが、API から取れることが分かったので `source = "loj"` の取得元を足し、23 問を切り替えました。`name` は問題の番号です。

### ログイン無しで取れます

`api.loj.ac/api/problem/getProblem` に `displayId` と `testData: true`、`judgeInfo: true` を渡すと、ファイルの一覧 (名前と大きさ) と、制限とサブタスクごとの in/out の組が返ります。`downloadProblemFiles` に内部 id とファイル名の一覧を渡すと、ファイルごとの署名付き URL (Cloudflare R2) が返ります。どちらも認証は要りません。`CPtools/loj_download.py` が提出の結果からファイル名を集めていたので、当初は「自分が提出していないと取れない」と読み違えていました。

`downloadProblemFiles` に渡すのは表示の番号ではなく `meta.id` です。多くの問題は同じ値ですが、`loj-6787` は表示が 6787 で内部が 40988 でした。Cloudflare が Python の既定の User-Agent を 403 で弾くので、UA を付けています。接続が途中で切れる (SSL の EOF) ことが一度あったので、HTTP のエラー以外は 3 回までやり直します。

### ケースの組はサブタスクから作ります

ファイル名の付け方が問題ごとに違います。`1.in` / `1.out` のほかに、`gcd1.in` / `gcd1.ans`、`input0.txt` / `output0.txt`、`1.000.in` / `1.000.out` があります。サブタスクがある問題は `judgeInfo.subtasks[].testcases[]` の `inputFile` / `outputFile` の組をそのまま使い、同じケースが複数のサブタスクに出るので入力ファイルで畳みます。サブタスクの無い問題は、拡張子を除いた部分が同じ `.in` と `.out` (無ければ `.ans`) を組にします。ケース名は入力ファイルの拡張子を除いたもので、`input0.txt` は `input0` になります。`cases_hash` に入るので、取り込んだあとは変えません。

判定器は `lines` か `integers` で、どちらも `compare.kind = "tokens"` で読めます。

### 制限は API の値を採ります

`judgeInfo.timeLimit` (ミリ秒) と `memoryLimit` (MB) を、AOJ の一覧と同じ扱いで `pj problems import` に足しました (`Titles.loj_limits`)。既定の 5 秒 / 512 MB より下には締めず、上なら判定サイトの値にします。切り替えた 23 問では `loj-154` (7 秒 / 1024 MB)、`loj-155` `loj-2340` `loj-6729` `loj-6730` (10 秒 / 1024 MB)、`loj-6719` (7.5 秒 / 1024 MB)、`loj-6673` (6 秒)、`loj-3165` (1024 MB) が広がりました。

### 題名は zh_CN です

`localizedContentsOfLocale` で `en_US` を頼んでも、無い問題は既定の言語で返ります。23 問は `loj-6686` (Stupid GCD) と `loj-6714` (Stupid Product) を除いて中国語の題名です。`pj problems titles --fix` で判定サイトの名前に揃えました。手で書いてあった `loj-2419` の Landscaping は `「USACO 2016 US Open, Platinum」Landscaping` になりました。

### 切り替えの影響

`url` は `source` と `name` から組めるので消しました。サイトの表示は「LOJ」で、判定サイトのデータ (`official`) として扱います。23 問には記録が 1 件も無かったので、`problem.toml` が変わっても測り直しは起きません。テストデータは原本から取ったあとに保管庫へ上げてあるので、CI は保管庫から取ります。手元 (Apple clang) では 23 問 25 提出がすべて AC でした。テストデータは合わせて 350 MB ほどで、最大は `loj-6699` の 164 MB です。

## Library の tc.zip からの取り込みの記録

Library の Release `v0.0.0` に付いている `tc.zip` (574 MB) は、verify が oj で取れないサイトのために以前に手で落としたテストデータの束です。中は `tc/<URL の md5>/test/` に `.in` と `.out` が並ぶ形で、48 の URL ぶんが入っています (URL の一覧と md5 は CPtools の `url.csv` と `url_hash.md`)。LOJ 27 問、JOI 春合宿 5 問、CSES 1 問、Codeforces 2 問、HackerRank 10 問、UTPC 2 問、AtCoder の JOISC 2016 の 1 問です。

`manual` で人待ちだった問題のうち、CSES 1 問 (8 ケース)、Codeforces 2 問 (`cf-622-f` 21 ケース、`cf-gym103373-i` 163 ケース)、HackerRank 10 問 (6 から 51 ケース) をここから `pj testdata import` で取り込み、保管庫へ上げました。HackerRank の 13 問のうち `bonnie-and-clyde`、`cube-loving-numbers`、`cutting-the-string` の 3 問は入っていなかったので人待ちのままです。LOJ と JOI 春合宿は API と配布 zip から取ってあるので、こちらは使っていません。

UTPC 2012 の L (`atcoder-utpc2012-12`、82 ケース) と UTPC 2013 の K (`atcoder-utpc2013-11`、74 ケース) は AtCoder の問題なので `source = "none"` の compile_only でしたが、公開データが入っていたので `manual` の tokens に切り替えました。AtCoder の問題で本物の判定になるのはこの 2 問だけです。`problem.toml` が変わるので compile_only の記録は参考に落ち、次の run で測り直されます。

手元 (Apple clang) では 15 問 17 提出がすべて AC でした。`cses-2132` と `hackerrank-grid-xor-query` のハーネスは問題文のサンプルでしか確かめていなかったので、ここで初めて判定サイトのデータを通っています。

## 宣言による割り当ての実装の記録

`plan` が束を配ってジョブに渡す形をやめ、ジョブが起動してから問題を 1 つずつ宣言して取る形にしました。設計の経緯は my-docs の「procon-judge の割り当てを宣言に変える設計」にあります。

### なぜ変えたか

49ef163 の run は 18 本のジョブが 16 分から 41 分に散り、全体が 43 分でした。ランナーが引く CPU モデルごとに残りの量が違い、問題ごとの重さも違い、件数 200 で切っていたので重い束を引いた 1 本が全体を決めていました。同じログを分解すると、ジョブの固定費は 0.6 分から 0.8 分、保管庫からの取得は 1 問 1.5 秒から 3 秒で全体の 4 パーセントから 7 パーセント、コンパイルが 17 パーセントから 28 パーセント、実行が 65 パーセントでした。取得は軽いので、宣言の単位は問題のまま粗くしていません。

### 宣言は ref の作成です

宣言は `refs/claims/<run id>/<環境>/<CPU モデル>/<問題 id>` の ref を `git push --force-with-lease=<ref>:` で作ることです。期待値を空にした lease は「その ref がまだ無いこと」を条件にするので、既にあれば server が `stale info` で弾きます。`refs/heads` の外なので `on: push` は起きません。CPU モデルは空白を含むので、ref に使えない文字を `-` にまとめます (`claims.slug`)。

押す commit は宣言ごとに一意の、親の無いものです。同じ sha を押すと client が「Everything up-to-date」と判定して server に届かず、二重の宣言が両方通ります (手元の bare リポジトリで確かめました)。run id と環境とモデルと問題とジョブ番号を message に入れ、作者と日時を固定して、sha が宣言の内容だけで決まるようにしています。裏返しで、同じジョブが同じ宣言をやり直すと同じ sha になり、up-to-date で成功扱いになります。push がタイムアウトしたあと実は通っていた場合でも二重には数えません。

REST の API は使いません。GITHUB_TOKEN は 1 リポジトリ 1 時間 1000 リクエストで、全記録が参考に落ちる回 (700 問 × 6 モデルで 4000 を超える宣言) に足りないからです。git の push はこの枠に入りません。宣言の一覧は `git ls-remote --refs origin 'refs/claims/<run id>/<環境>/<モデル>/*'` で 1 往復で取れます。

### ジョブの動き

ジョブは `plan` と同じ重い順の並びを組み直し、ジョブ番号 j と本数 N から j/N の位置を開始点にして回ります。開始点が散っていれば同じ問題を同じ瞬間に宣言しようとすることがほぼ無く、時間切れで持ち越される問題も並びの末尾に偏りません。既知のモデルで全部計測済みの問題は `plan` の並びに無いので、うしろに付けています。引いたモデルが新しければそこにも仕事があります。

問題ごとに、まず記録から借りた `cases_hash` で判定して、走らせるものが無ければ宣言せずに次へ行きます。宣言できたらテストデータを取って本物の `cases_hash` で判定を組み直し、そのモデルで未計測の提出を全部測ります。時間の上限は宣言する前に見て、宣言した問題は必ず測り切ります。軽い問題と重い問題で分岐はしません。宣言 1 回は 1 秒から 2 秒で、問題 1 つの仕事は平均 40 秒から 80 秒なので誤差の範囲です。

宣言したまま死んだジョブの問題は、その run では誰も拾いません。名前が run 単位なので、次の run の `plan` が未計測として数え直して拾います。run の中で救う「期限切れの宣言を奪う」は複雑になるので入れていません。宣言の push が GitHub の都合で通らないときも、その問題を飛ばして次の run に回します (「-march を外してソースで宣言する記録」)。

### 消したもの

件数の上限 (`--budget`)、`plan.assignment` (j 番目の束から本数ぶん飛ばす配り方)、`run` が事前の担当を並べ直す `_assigned`、「暇なら次の束へ踏み込む」と「同じキーの 2 本目の記録は標本」の話です。束という言葉も消し、`plan` に残る 1 問題ぶんの見積もりは `Work` と呼びます。本数は全モデルを合わせた未計測の件数を 1 本あたりの見込み (`ITEMS_PER_JOB` = 50) で割った切り上げです。

### 最初の run で分かったこと

cbceff4 の run (35695428755) で宣言は動きました。GITHUB_TOKEN の `contents: write` で ref が作られ、5 分で 30 本の宣言が並びました。一方で x64 の 16 本のうち 10 本が「このモデルでは仕事なし 719 問」で 1 分ほどで終わりました。未計測がモデルに偏っていたからです。results を数えると x64-gcc の未計測は Xeon 8370C に 935 件、6973P-C に 831 件、EPYC 9V45 に 813 件、Xeon 8573C に 481 件で、よく当たる EPYC 7763 と 9V74 は 4 件ずつでした。稀なモデルに当たった 6 本が 1 本ずつ抱える形になり、宣言では解決しない「モデルの当たり」の天井そのものです。

対策は本数です。当たったモデルに仕事が無いジョブは 1 分ほどで終わって次のジョブに枠を渡すので、1 環境の上限を 8 本から 20 本に上げて当たりを引き直す回数を増やしました。同時実行の 20 を超えるぶんは GitHub が待たせるだけです。

もう 1 つ、保管庫に無い manual の問題 (HackerRank の 3 問) は記録が無いので費用が最悪値になって並びの先頭に来て、各モデルの最初のジョブが毎回宣言して取得に失敗していました。保管庫の一覧を 1 回取って、無いものは宣言せずに飛ばすようにしました。arm の `gf2-64-log` のように参照実装が組めない local の問題も同じく宣言して失敗しますが、これは宣言の前からある話なので触っていません。

### 掃除と手元での試し方

`collect` の最後に `pj claims clean --run-id <run id>` が、この run と、それより古い run (run id は増える整数) の宣言を消します。消し損ねても宣言は run 単位の名前なので次の run の邪魔にはならず、次の `collect` が消します。

手元では `git remote add claims-test <bare リポジトリ>` してから `pj run --env local --claim-run t1 --claim-remote claims-test --jobs 2 --job 1 --problem <id> --store <空の store>` で宣言の流れを試せます。2 本目のジョブが同じ問題を宣言済みとして飛ばすこと、`pj claims clean --run-id t1 --remote claims-test` で消えることを確かめました。

## ヘッダの要約とゲートの実装の記録

Library の verify を畳む順番の 1 番です (my-docs の「Library の verify を畳む設計」)。`pj site build` が `data/headers/index.json` を出し、summary に「全環境に現行 AC が揃ったヘッダ N / M」を出すようにしました。

要約は逆引き JSON と同じ材料 (ヘッダごとの提出の一覧と、提出ごとに環境で畳んだ状態) から `header_index` が作ります。環境ごとに提出を数え、現行の AC が 1 本以上あれば `verified` です。参考に落ちた AC は数えません。ソースを変えた直後に緑のままにしないためです。判定できない (現行かどうかが None の) AC は現行と同じ扱いにしています。これは順位表と同じ規則です。compile_only の提出は AC がコンパイル可の意味なので数えず、そのヘッダを使う提出が compile_only しか無いときだけそれで読み替えます。逆引き JSON の各行にも比較の種別 (`compare`) を足したので、読み替えの判断は向こうでもできます。

ゲートは全環境で `verified` なヘッダの数と総数です。総数は閉包に出てくるライブラリのヘッダの数で、Library の `hpp_map` の 144 と一致します。畳む判断は人がこの数字を見てやるので、CI を止めたりはしません。

## Library の pin と JSON の版の記録

Library と judge の結合を見直したとき (2026-09-22)、結合の形そのものは分けたまま作る下限に近いと判断して、弱かった 2 点だけ直しました。

1 つは Library の commit を run の中で揃えることです。`plan`、`run` の各ジョブ、`collect` が別々に `git clone` していたので、run の途中で Library に push があると、同じ run の中で違う commit を測ったり、`collect` が別の commit で現行を判定したりしていました。個々の記録はファイルごとのハッシュを持つので嘘はつきませんが、run の単位では不整合でした。`plan` が clone した HEAD を job の出力 `library_sha` に載せ、`run` と `collect` はその commit を `git fetch --depth=1 origin <sha>` で取ります。GitHub は到達可能な commit を名指しで fetch できるので、履歴は 1 段しか取りません。`plan` が取れていなければ (出力が空) 今までどおり最新を取ります。

もう 1 つは、リポジトリをまたぐ契約である `data/headers/` の JSON に版 (`schema`、今は 1) を載せることです。形を変えるときに上げ、Library 側のサイトは知らない版を読まずに節を隠します。版の無い JSON は 1 として読みます。

## ジョブの単位を組にした記録

宣言を入れた直後 (2026-09-22) に、run のジョブの単位を環境 (`x64-gcc` など 4 つ) から組 (`x64` と `arm`) に変えました。組は同じマシン (runs_on) に載る環境の束で、`pj.environment.groups` が環境名の最初の区切りまで (`x64-gcc` なら `x64`) で束ね、runs_on が揃っていることを確かめます。

理由は稀な CPU モデルです。未計測はよく当たるモデルでは埋まり、稀なモデルに残ります。環境ごとのジョブだと、稀なモデルに当たった 1 本は gcc か clang の片方しか測れず、もう片方はまた当たりを待つことになります。1 本が組の全環境を測れば、当たったその場で両方が埋まり、同じ問題の gcc と clang が同じモデルで同じ回に揃います。テストデータの取得と捨てる回数も問題につき 1 回になります。取得は全体の 4 パーセントから 7 パーセントなので、こちらは小さい利点です。

変わったのは 4 か所です。`plan` は環境ごとの計画を組にまとめ (`plan.group`)、本数は組の全環境と全モデルの合計から決めます。上限は 1 組 40 本で、環境ごとに 20 本だったときと当たりの引き直しの回数を揃えています。組の問題の並びは環境ごとの費用を足した重い順です (`plan.combined_order`)。matrix の 1 行は組で、`envs` (コンマ区切りの環境名) と `toolchains` を持ちます。`pj run --env x64-gcc,x64-clang --claim-run ...` は組の全環境のコンパイラを検出し、宣言は `refs/claims/<run id>/<組>/<モデル>/<問題>` に作り、宣言した問題を環境ごとに判定して測ります。judge.yml は組の全コンパイラを入れ、片方が入らなくても止めません。`pj run` が使えないコンパイラの環境を飛ばして残りで測ります。別のコンパイラで代用しない原則はそのままです。

## 順位表を束で測る実装の記録

2026-09-22 に、順位を出す問題 (`harness.kind = "base"`) の測り方を束に変えました。設計は my-docs の「procon-judge の順位表を束で測る設計」です。

きっかけは gf2-64-pow の順位表です。11 時台に測った 14 本と 13 から 14 時台に測った 5 本を同じ CPU モデルの表に並べたところ、EPYC 9V74 では後から測った 5 本が一様に 15 から 20 パーセント遅く、EPYC 7763 では逆に速い側に寄りました。同じ CPU モデルでも VM ごとに速さが 2 割ほど違い、variant 同士の差 (5 から 10 パーセント) より大きいということです。「キーとスキップ」は変動係数の中央値 0.69 パーセントを根拠に「同じジョブへ同居させる必要は無い」としていましたが、「採らなかった選択肢」が予告していた尾 (pclmul 系の周波数の振れ) がここに出ました。

束は 1 つの (問題, 環境, CPU モデル) を 1 本のジョブで一度に測った記録の集合です (`pj/batch.py`)。同じ VM で続けて測った記録どうしは VM の差を同じだけ受けているので比べられます。記録は `batch` に束の id を持ち、CI では `<run id>/<組>/<ジョブ番号>`、手元では `local/<12 桁>` です。同じジョブは組の全環境を測るので id は環境をまたいで同じ文字列ですが、束の同一性は (問題, 環境, モデル, id) で見ます。

### 測り直しの条件は 1 つです

その (問題, 環境, モデル) のいちばん新しい束が、今の全提出 (include を解決できるもの) の現行のキーの記録を全部含んでいれば何もしません。1 本でも欠けていれば全提出を同じジョブで測り直します。提出を足したとき、ヘッダを直して `lib-*.hpp` の閉包が動いたとき、テストデータが入れ替わったとき、書き換えを元に戻して古いキーが復活したとき、のどれもこの 1 つの条件で覆えます。「キーが記録にあるか」ではなく「最新の束にあるか」を見るのが要点で、元に戻したときに表が参考のまま止まりません。

`run` の `_decide` と `plan` の `_for_env` が同じ条件を見ます。`run` は `Store.batches` (問題, 環境, モデル ごとの最新の束) を受け取り、`plan` は問題の記録から `batch.newest` で組み直します。CE と分かっている提出を候補から外すのは raw だけにしました。束は全提出で作るためで、CE のコンパイルは安いので損はありません。

束の無い古い記録は「束が無い」なので、最初の 1 回は base の全問題が測り直しになります。170 問 × 6 モデルで、宣言の 40 本に散って 1 時間ほどの見込みです。raw の 597 本は動きません。

### 手元で 1 本だけ測るときは束を作りません

`pj run --problem X --submission Y` は `batch` を付けずに測ります。1 本だけの束が最新になると、表の他の行が全部「束の外」になるからです。`--problem X` だけなら全提出が対象なので束を作ります。raw の問題には常に付けません。

### 重複排除は (キー, 束) です

束は同じキーの記録を束ごとに 1 件ずつ作るので、`Store.absorb` の重複排除をキーだけから (キー, 束) に変えました (`Store.identities`)。束の無い記録は空文字で、今までどおりキーで排除します。

### 順位表はいちばん新しい束だけを並べます

base の問題の順位表は (環境, モデル) ごとにいちばん新しい束の記録だけを並べます (`collapse(..., batched=True)`)。順位はその束の中の値で、キーの記録の最小値は採りません。束をまたいで最小値を採ると、行ごとに違う VM の値が混ざって元の問題に戻るからです。

束に無い提出は従来の畳み方の値を「束の外」の印付きで出し、参考と同じく順位に混ぜず下にまとめます。出るのはジョブが途中で死んで束が欠けたときで、次の run が束ごと測り直して消えます。束がまだ無い (環境, モデル) は従来どおりに並べ、「別々のランナーで測った記録が混ざっていて、行どうしを比べられません」と注記します。束があれば表の上に「この表は同じランナーで一度に測った記録です (時刻)」と出し、title に束の id を置きます。

正しさの判定 (現行の AC、Library 側のゲートと印) は記録ごとのままで、束は順位表だけの話です。提出ページと逆引き JSON は変えていません。

### 入れなかったもの

ジョブ内の計測の繰り返しは入れていません。「採らなかった選択肢」のとおり、周波数の振れは同じジョブの中では全部同じように落ち込むので、繰り返しても束の中の順位は変わりません。較正 (参照実装との比) と、アンカーだけ測り直してずれが大きければ束ごと測り直す節約案も採りませんでした。前者は VM の差が一様な倍率でないため、後者はしきい値が新しい調整点になるためです。

## 問題ディレクトリの階層化とキーの -I の正規化の記録

2026-09-23 に `problems/` の置き方を平らな 722 ディレクトリから階層に変えました。エディタで問題を探すのが大変になったのがきっかけです。

`problems/` の下は何段でも掘れます。`problem.toml` を持つディレクトリが問題で、その下は探しません。持たないディレクトリはただの入れ物です。`_` か `.` で始まる名前 (`_shared`、`__pycache__`) は飛ばします。id は `problems/` からの各段を `-` で繋いだものです。`problems/atcoder/abc172-d/` が `atcoder-abc172-d`、`problems/gf2-64/pow/` が `gf2-64-pow` で、1 段で置いた問題は名前がそのまま id です。id を変えずに置き場所だけを変えたので、results の jsonl、保管庫のアセット名、サイトの URL、宣言の ref はどれも動いていません。同じ id が 2 か所から出たら `all_problem_dirs` が止めます (`pj/problem.py`)。

判定サイトの問題は接頭辞のディレクトリに入れました。`aoj/` 178、`atcoder/` 154、`yuki/` 142、`yosupo/` 139、`loj/` 23、`hackerrank/` 13、`cf/` 8、`joisc/` 5、`codechef/` 4、`luogu/` 3、`ojuz/` 2、`cses/` 1、`kattis/` 1 です。自作は族で分けました。`modulo-test/` 14、`gf2-64/` 10、`constexpr/` 9、`1word-mod/` 4、`nimber/` 3、`modpow-test/` 2、`gcd-test/` 1 です。族を持たない 6 問 (mod-inv-prime、oeis-mod-tetration、plc-pool-test、remainder、warshall-floyd、wbt-pool-test) は 1 段のままです。`atcoder/` を `abc/` などでさらに分けることはできません。段を足すとそのぶん id に `-` が入るからです。importer (`pj problems import`) は判定サイトの接頭辞を持つ id を接頭辞のディレクトリの下に書き出します (`problem.dir_for_new`)。

### キーの材料では問題のディレクトリの -I を印にします

問題のディレクトリはコンパイルフラグに `-Iproblems/<id>` として入り、フラグはキーの一部です。そのままだとディレクトリを動かした問題はそれだけで測り直しになります。キーの材料を作るときだけ、問題自身の `-I` を `-I@problem` に置き換えることにしました (`build.key_cxxflags`)。実際のコンパイルと記録の `cxxflags` の欄は実際のパスのままです。`run` の `_decide` と `Freshness.key_for` の両方がこの関数を使い、`Freshness.diff` が「フラグが変わった」と言うときの比較は実際のフラグどうしで行います。

この置き換えを入れた瞬間に全問題のキーの材料が変わるので、既存の記録は一度全部参考に落ちて測り直しになります。同じ日に入れた束の移行の測り直しと同じ規模です。以後はディレクトリを動かしても測り直しになりません。

### 問題をまたぐヘッダは -Iproblems で引きます

`problems/_shared/` のヘッダは `#include "../../_shared/gf2-64/_common.hpp"` のように相対パスで引いていました。問題が 1 段深くなると `../` が 1 つ増えるので、`-Iproblems` を足して `#include "_shared/gf2-64/_common.hpp"` の形にし、273 ファイルを書き換えました。探索の順は `lib`、問題のディレクトリ、`harness`、`problems`、`third_party/simde` です。`_shared` のヘッダのラベル (記録の `includes` と `file_hashes`) は `problems/_shared/...` から `_shared/...` に変わりました。

### サイトのリンクは実際のパスを使います

GitHub へ飛ぶリンク (提出のソース、ジェネレータ、参照実装、問題のディレクトリ) は id から `problems/<id>/...` を組んでいましたが、問題のディレクトリのリポジトリからの相対パスを問題の JSON の `dir` に載せてそれを使うようにしました。記録の `judge_sha` は測ったときのコミットなので、移す前に測った記録 (参考) のリンクは新しいパスと組み合わさって 404 になります。測り直しで入れ替わるまでの話です。

## 網羅モードの実装の記録

2026-09-23 に run にモードを 2 つ入れました。push と Library の dispatch は網羅モード (`cover`)、schedule と手動は全モデルモード (`all`) です。設計は my-docs の「procon-judge の push の run を網羅モードにする設計」にあります。きっかけは同日の階層化の push で、キーが変わって全問題が測り直しになり、run ジョブが 80 本立って 45 本が待ち行列に並び、次の push の run が 1 時間近く pending のままだったことです。本人が push の直後に見るのは x64-gcc の順位表 1 つで、6 モデル全部が新しくなるのを待つ必要はありませんでした。

### 判定は plan と run で同じ関数です

網羅モードの「揃っている」は `plan._for_env` の既存のモデルごとの判定をそのまま使い、todo が空のモデルが 1 つでもあれば飛ばす、と読み替えるだけにしました (`Work.cover`)。run のジョブは自分で plan を組み直して `needs_by_env` で「この環境は測る」の集合を持ち、宣言した問題のうちその環境だけを `build_worklist` に渡します。ジョブの側に別の判定を書かなかったのは、plan と run で判定がずれると「plan はジョブを立てたのに run は何も測らない」が静かに起きるからです。

### 仕事の量は 1 モデルぶんで数えます

全モデルモードの `Work.total` はモデル全部の合計でしたが、網羅モードはどのモデルに当たっても 1 回しか測らないので、欠けの平均の切り上げにしました。base は全提出、raw は 1 本の提出がどのモデルにも無ければ 1 です。1 問の push なら x64 と arm に 1 本ずつ立ちます。

### 全モデルモードの本数は 1 run 16 本です

`plan.group` が組ごとの本数を出したあと `cap_run` で合計を `MAX_JOBS_PER_RUN_ALL` (16) に縮めます。理由は同時実行の枠が concurrency group をまたいで共通なことで、group を分けても待ち行列の後ろに並ぶのは変わりません。網羅モードは組ごとの上限 40 本のままです。

### 宣言の掃除は走っている run を避けます

`claims.clean` は「この run 以前を全部消す」でしたが、group が 2 つになると古い run が走っている横で collect が動きます。古い run ごとに `gh api` で状態を 1 回見て、終わっていないものの宣言は残すようにしました (`run_finished`)。見られなければ残す側に倒し、次の collect が見直します。collect の permissions に `actions: read` と、掃除の step に `GH_TOKEN` を足しました。

### モードの式は judge.yml に 2 回書いてあります

concurrency の group は `env` を見られないので、引き金からモードを決める式を workflow の `env.MODE` と `concurrency.group` の 2 か所に書いています。直すときは両方です。plan は `--mode` で受けて matrix の各行に載せ、run は matrix の値をそのまま `--mode` に渡します。

### 別の group の run の記録は plan に見えません

run の記録は collect が最後に results へ push するまで results に無いので、同時に走っている別 group の run の plan はその分を未計測として数えます。移行の最初の push (93ba5cd) は、古い group の run (階層化でキーが変わった全問題の測り直し、80 本) が終わる前に走ったので、網羅モードでも全問題が「揃っているモデルが無い」になり、80 本が立って同じものを測り直しました。同じ group の run は順に走るので前の run の記録を見ますが、group をまたぐとき (schedule の run が走っている横で push したとき) は、その run が測っている途中の問題を重複して測ります。網羅モードの重複は 1 モデルぶんで、(キー, 束) の重複排除と最新の束の表示が守るので壊れませんが、キーが全部変わる回のように量が大きいこともあります。他の run の宣言 (`refs/claims/<別の run>/...`) が付いている問題を「測っている最中」として飛ばす案は、宣言が 1 往復で全部取れるので安く入れられますが、まだ入れていません。

### ヘッダの要約は現行の記録だけで畳みます

ヘッダの要約 (`data/headers/*.json` と `index.json`) は提出の記録を環境ごとに 1 行へ畳みますが、その畳み方は「モデルが全部現行のときだけ現行」でした。網羅モードの run は環境ごとに 1 モデルしか新しくしないので、この畳み方だと Library の push のたびに他のモデルの参考が残っている間ヘッダの印が落ち、ゲートは移行の直後に 143/144 から 59/144 に下がりました。畳み方を「現行の記録が 1 本でもあればそれだけを見る」に変えました (`env_summary`)。参考の記録は測ってからソースが変わったもので、今のソースの証拠ではありません。現行の記録どうしでは 1 モデルでも落ちていれば落ちている扱いのままで、現行が 1 本も無いときだけ参考で状態を出して current を False にします。

### サイトは揃っているモデルを既定にします

問題の JSON の組 (環境, モデル) に `complete` と `holes` (未計測と参考と束の外の合計) を足し、ページは揃っているモデルを既定に選びます。前回選んだモデル (localStorage) が揃っていればそれ、無ければ並びの最初の揃っているもの、揃っているものが無ければ欠けの少ないものです。選択肢の名前に欠けの数を添えています。push 直後は環境ごとに 1 モデルしか揃っていないので、既定が未計測の混ざったモデルだと測ったのに見えないからです。

## -march を外してソースで宣言する記録

2026-09-24 に、コンパイラへ `-march` を渡さず、使う命令はソースで宣言する方針に決めました。`-march=x86-64-v3` は AVX2 や BMI2 を黙って使えるようにするので、宣言を書き忘れたコードでも procon-judge では通ります。そういうコードを `-march` を付けない判定サイトに出すと CE になります。Codeforces の G++ のコマンドには `-march` がありません。手元の clang で x86_64 向けに比べると、v3 では組めて `-march` なしでは組めない提出が 66 本ありました。自動ベクトル化や 1 命令の popcount も `-march` で決まるので、宣言したぶんだけ速く出る形で測る方が、宣言を書く習慣に合います。

切り替えは 3 段に分けました。1 段目と 2 段目はソースの書き換えで、`-march=x86-64-v3` のままでも同じ経路を通ります。3 段目で、2026-09-25 に x64 の 2 環境から `-march` を外しました。arm の `-march=armv8.2-a` は別に決めます。SIMDe が PMULL を使うかどうかは全体のフラグで決まり、関数ごとの宣言では変えられないからです。

同じ 2026-09-25 のうちに、土台の命令はオプションに戻しました。ソースの宣言には置き場所の制約があり、ハーネスの形では判定サイトより遅く測られると分かったためです。経緯は、この節の最後の「土台の命令をオプションに戻しました」にあります。

### 機能のマクロで分けた経路を先に直しました

`__BMI2__` や `__AVX2__` で分けた経路は、`-march` を外すと CE にならずに遅い側へ落ちます。gf2-64-sq の pdep 系 9 本は、分岐を x86 かどうかの判定に替えて bmi2 を宣言しました。Library Checker の最速解の写し 12 本は、`#pragma GCC target` の隣に clang 向けの `#pragma clang attribute push` を置き、ファイルの末尾で pop します。

clang の push は GCC の pragma と違って `__AVX2__` などのマクロを立てません。写しの本体はマクロで分岐しているので、見ているマクロだけをその場で立てています。SIMDe のときに写しが自分で `__AVX2__` を立てているのと同じやり方です。

### gf2-64 の家族は共通ヘッダでまとめて宣言します

gf2-64 の 12 問と yosupo-convolution-f2-64 の提出は、どれも `_shared/gf2-64/_common.hpp` を読みます。その最後で、x86-64-v3 の命令と pclmul を宣言する領域を開きます。GitHub のランナーの 6 モデルは、どれもこれらを持っています。

clang の `#pragma clang attribute push` は、翻訳単位の中で pop しないとエラーになります。ヘッダで開いたままにはできないので、13 問の base.cpp の最後に `GF2_64_TARGET_END` を置いて閉じます。GCC の `#pragma GCC target` は翻訳単位の終わりまで効くので、GCC では何もしません。

関数ごとの `GNU_TARGET(x)` は、家族の一覧に x を足したものにしました。clang は関数に target を書くと、その関数には領域の宣言を付けないからです。一覧を含めておかないと、GCC では足し合わされ、clang では関数に書いた分だけになって、中身が変わります。`GNU_TARGET` を通さずに `[[gnu::target("pclmul")]]` を直接書いていた log と sqrt の 4 本も、`GNU_TARGET` に揃えました。

pclmul を家族の一覧に入れたので、今の v3 のもとで起きていた呼び出しが消えました。pclmul を関数の側にだけ宣言し、呼び出す側のループに宣言していないと、インライン展開されずに 1 回ずつの呼び出しになります。手元の clang で数えると、測る関数が小さい mul や sq を呼び出していた提出は 28 本で、書き換えたあとは表を引く tower 系の 3 本だけでした。

### 名前空間の 256 bit の定数はベクタのリテラルで初期化します

名前空間に置いた `const __m256i RED_TABLE= _mm256_setr_epi8(...)` の初期化は、コンパイラが作る関数の中で走ります。clang の attribute push はユーザーが書いた関数にしか付かないので、`-march` なしではその関数に AVX が無く、CE になります。x86 では `GF2_64_M256_SETR_EPI8` で、関数を呼ばないベクタのリテラルにしました。値の並びは `_mm256_setr_epi8` と同じで、SIMDe のときは `_mm256_setr_epi8` のままです。

### 3 段目で -march を外しました

外す前に、家族の外で `#pragma GCC target` だけを書いていた 9 本に、clang 向けの宣言を並べました。clang はこの pragma を無視するので、intrinsics を使っていれば CE で気づけますが、自動ベクトル化に頼るコードは黙って遅くなります。手元の clang で組むと、9 本とも前は 256 bit の命令が 0 でした。AVX-512 の 2 本は、前は clang で CE で、今は GCC と同じく AVX-512 のある CPU でだけ走ります。

キーのフラグが変わるので、x64 の記録は外した回に全部測り直しになりました。Library の PR の検査も `-march` を外しました。ただしその検査は構文だけを見るので、宣言の書き忘れは見つけられません。書き忘れの CE はインライン展開のときに出るので、見つけるのは procon-judge の記録です。

外した回の測り直し (af79ee7) で判定が変わったのは 7 件でした。4 件は AVX-512 や GFNI のない EPYC 7763 に当たって RE になったもので、同じ CPU では前から RE でした。modulo-test-runtime-30-even の long_double.hpp の TLE は、同じ 7763 で前は最長 7.4 秒の AC だったので、`-march` を外して遅くなったものです。残りの 2 件は convolution-f2-64 の schoenhage の 2 本で、x64-gcc で CE になりました。pclmul だけを宣言した関数で SSE4.1 の `_mm_extract_epi64` を使っていたので、宣言に sse4.1 を足しました。clang はこの intrinsic を SSE4.1 なしで組めるので AC のままでした。

判定は変わらないまま、x64-gcc でだけ 2 倍から 5 倍遅くなった提出が 9 本あります。どれも Library Checker の写しで、`#pragma GCC optimize("Ofast")` と `#pragma GCC target` を両方書いています。GCC は fast-math (中の finite-math-only) のもとで target を組み直すと、宣言より前に定義された関数と target のフラグが食い違い、その関数をインライン展開しません。base.cpp は提出より先に pj.hpp と common.hpp を読むので、`std::vector::operator[]` のような標準ライブラリの関数がすべて宣言より前に来て、測る関数に展開されなくなります。判定サイトでは pragma が `#include <bits/stdc++.h>` より前にあるので、この食い違いは起きません。`-march=x86-64-v3` を付けていたあいだは pragma が何も足さないので隠れていました。clang はインライン展開してよいかを命令セットの包含だけで決めるので、影響を受けません。characteristic-polynomial の写しを x86 の GCC 15 で組むと、展開を断った数は今の読み込み順で 882、提出を pj.hpp より先に読む順で 2 でした。直し方は未定です。

RED_TABLE の名前の衝突 (940963a で mul.hpp と mul2.hpp が同じ名前空間に同じ名前の表を持った) で CE だった 58 本の陰に、gf2-64-log の 15 本の宣言の漏れが隠れていました。構造体の static inline メンバの `__m256i` を、`_mm256_set_epi64x` と `_mm256_set1_epi64x` で初期化していました。2 段目の `_mm256_setr_epi8` と同じ理由で、`-march` なしの clang では CE になります。`_common.hpp` に `GF2_64_M256_SET_EPI64X` と `GF2_64_M256_SET1_EPI64X` を足して、リテラルで初期化するようにしました。

### gf2-64 の家族は関数ごとに宣言しない

家族の提出と共通ヘッダの関数に付けていた GNU_TARGET を外しました。家族の一覧 (x86-64-v3 の命令と pclmul) は `_common.hpp` の最後で開く領域がまとめて付けるので、pclmul や bmi2 の GNU_TARGET はもともと重なっていただけです。一覧に無い vpclmulqdq と gfni は、提出が共通ヘッダを初めて include するより前で `GF2_64_EXTRA_TARGETS` を定義します。`_common.hpp` は、その分を足した一覧で領域を 1 回だけ開きます。

include したあとで足す形にしなかったのは、clang に効かないからです。`#pragma clang attribute push` を入れ子にすると、clang は関数に両方の target を付けたうえで、一番外側のものだけを使います。外側を家族の一覧、内側を vpclmulqdq にすると、vpclmulqdq が無いという CE になりました。内外を入れ替えると、今度は avx2 が無いという CE になりました。

足す命令は、足さずに組んで CE になった提出にだけ入れました。vpclmulqdq が 86 本で、gfni が 1 本です。足した命令はその提出の翻訳単位全体に効きます。ただ、家族には実行時に CPU を見て経路を切り替える提出がないので、動く CPU の範囲は変わりません。

足す命令のない提出は、x86_64 と arm64 のどちらでも、assert に渡す行番号を除いて前と同じコードでした。足した提出のうち 23 本は、宣言の違う関数の境目が消えて、関数の呼び出しが減りました。x86 の GCC 15 でも、家族の 186 本が `-march` なしで組めます。

### 宣言の push は GitHub の 500 を受けてもジョブを止めません

2026-09-24 の run で、宣言の ref の push に GitHub が 500 を返し、2 秒おき 3 回のやり直しが全部外れて、2 本のジョブが止まりました。間隔を倍々に延ばして 5 回までやり直し、それでも通らなければその問題だけを飛ばして次へ進むようにしました。飛ばした問題は次の run が拾います。宣言の一覧が取れないときは、手元の一覧のまま進みます。

### 土台の命令をオプションに戻しました

2026-09-25 に、x64 の 2 環境へ土台の命令を `-march=x86-64-v3 -mpclmul -mvpclmulqdq` で渡す形に戻しました。x86-64-v3 の命令に pclmul と vpclmulqdq を足したもので、GitHub の x64 ランナーの 6 モデルはどれも持っています。設計は my-docs の「procon-judge の x64 の土台の命令をオプションで渡す設計」にあります。

ソースの宣言には置き場所の制約があります。GCC の `#pragma GCC target` は、pragma より後ろに定義された関数にだけ効きます。clang の `#pragma clang attribute push` は、push より後ろに書かれた関数にだけ宣言を付けます。コンパイラが作る関数 (暗黙のデストラクタなど) と、push より前に書かれたテンプレートの中身には付けません。base.cpp は提出より先に pj.hpp を読むので、提出が先頭に宣言を書いても、標準ライブラリは宣言の外に残ります。宣言の多い関数は宣言の少ない関数の中へ展開されないので、std::sort に渡したラムダがソートの中へ展開されません。試したソートでは、ラムダを関数として呼ぶ箇所が 2 から 18 に増えました。上に書いた fast-math の 9 本が x64-gcc で遅かったのも、同じ置き場所の制約から来ています。clang の push を標準ライブラリより前へ置くと、libstdc++ の暗黙のデストラクタで CE になるので、前へ置くこともできません。

毎回同じ一覧を全部宣言するのであれば、オプションで渡すのと変わりません。オプションなら、標準ライブラリもコンパイラが作る関数も同じ命令を持つので、ここまでの制約はどれも起きません。characteristic-polynomial の写しを x86 の GCC 15 で組むと、target の食い違いで展開を断った数は、`-march` なしで 882、土台を渡すと 0 でした。9 本の pragma が挙げる命令 (avx2、bmi、bmi2) はどれも土台にあるので、GCC は pragma で target を組み直しません。

土台に vpclmulqdq を入れたのは、コンパイラが自分の判断では使わず、intrinsics を書いた場所にしか出てこないからです。Codeforces の判定機は vpclmulqdq を持たないので、vpclmulqdq を使うコードは `__builtin_cpu_supports("vpclmulqdq")` で実行時に分けます。AVX-512 はコンパイラが自動ベクトル化で自分から使い、GFNI も clang がバイト単位の処理に使うことがあるので、土台に入れません。使う提出が今までどおりソースで宣言し、その命令を持つ CPU でだけ走ります。宣言した命令は土台に足されます。clang でも、関数の target は土台に足す形で効くので、gf2-64 の家族の領域の中の関数に `target("gfni")` を書いても組めます。push の入れ子は、土台があっても一番外側の target だけが使われます。

1 段目から 3 段目でソースに足した宣言は残しました。土台と重なる宣言は、組んだ中身も展開のされ方も変えません。判定サイトへ出すコードにとっては、直した意味が残ります。gf2-64 の家族の仕組み (`_common.hpp` の領域と `GF2_64_EXTRA_TARGETS`) を整理するかは、あとで決めます。

諦めたのは、土台の命令の宣言を書き忘れたコードを procon-judge で見つけることです。判定サイトへ出すときは、algo-workspace のバンドラが同じ一覧の GCC の pragma を提出の先頭に入れて補います。一覧は gf2-64 の家族の `GF2_64_TARGETS` に vpclmulqdq を足したものです。

```cpp
#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif
```

`__x86_64__` を条件に入れるのは、x86 以外の CPU では GCC が x86 の命令名を CE にするからです。clang 向けには何も入れません。AtCoder は `-march=native` で組むと理解しているので、宣言が要らないはずです。

キーのフラグが変わるので、戻した回で x64 の記録は全部測り直しになりました。Library の PR の検査も同じオプションで組むようにしました。

戻した回の測り直し (99d655f、run 36102107710) で、`-march` なしのときから判定が変わったのは 2 件でした。modulo-test-runtime-30 と modulo-test-runtime-30-even の long_double.hpp が、x64-gcc の EPYC 7763 で TLE から AC に戻りました。土台が原因の CE、WA、RE はありません。

速さは、同じ CPU モデルの記録どうしで、合計時間の比の中央値を取って比べました。`-march` なしで v3 のころより 1.5 倍以上遅くなっていたのは 34 件 (提出で 27 本) で、そのうち 33 件が v3 のころの 0.42 倍から 1.14 倍に戻りました。fast-math の 9 本は v3 のころの 0.79 倍から 1.02 倍で、`-march` なしのときと比べられる 7 本は、そのときの 0.20 倍から 0.65 倍の時間です。

残る 1 件は yuki-1303 の x64-clang で、6973P-C で v3 のころの 4 倍でした。同じ回に 6973P-C で測った raw の問題は、プロセスの時間が全体に伸びています (中央値 1.17 倍)。yuki-1303 の x64-gcc も 3.6 倍でしたが、GCC 15 で組むと v3 と今回のフラグで機械語が同じでした。ハーネスが測る計算の時間は同じ 6973P-C で中央値 1.02 倍なので、マシンの揺れと見ています。9V74 のジョブ 2 本 (x64/20 と x64/23) でも、束の全提出がそろって 1.25 倍から 1.5 倍遅く出ました。gf2-64-frob16 の sq_chain.hpp が v3 のころより遅いのは、そのあいだに sq.hpp が変わったためで、フラグの影響とは切り分けていません。

### arm は -mcpu=neoverse-n2+aes にしました

2026-09-25 に、arm の 2 環境を `-march=armv8.2-a` から `-mcpu=neoverse-n2+aes` にしました。arm は動けばよく、速いほど CI が早く終わるので、ランナーの CPU に合わせる方を選びました。run のジョブの最初で /proc/cpuinfo の命令の行を出すようにして、arm のランナー (Neoverse-N2) が aes、pmull、sha3、sve2、i8mm、bf16 などを持つことを確かめています。mte と rng は見えませんが、コンパイラが自分から使う命令ではありません。

`+aes` が効くのは SIMDe の `_mm_clmulepi64_si128` です。`__ARM_FEATURE_AES` があれば、PMULL の 1 命令になります。SIMDe は clang 22 より前ではこの経路を使わないので、速くなるのは arm-gcc で pclmul を使う提出です。SIMDe の x86 の経路は SVE を使わないので、SVE2 が増えても SIMDe の中身は変わりません。キーのフラグが変わるので、arm の記録は全部測り直しになりました。

## 既存リポジトリから移すもの

| 移すもの | どこから | 備考 |
| --- | --- | --- |
| テストデータ取得の実装 | `Library/scripts/lib/download.py` ほか | 判定サイトごとの癖が入っているので移植します |
| 問題 URL と制限の対応 798 件 | `Library/test/**/*.test.cpp` のアノテーション | 一番高い資産です。機械的に変換できます |
| algo 実装 551 個 | `judge/problems/*/algos/*.hpp` | うち SIMD か SIMDe を使うものが 173 ファイルです |
| 記録のスキーマ | `judge/.results/history.jsonl` | 必要な項目をほぼ持っています |
| ランダム入力の部品 | `algo-workspace/gen_util.py` | `local` のジェネレータで使います |
| LOJ 取得と URL ハッシュ | `CPtools/` | `loj_download.py` と `url_hash.md` です |

798 件の問題 URL の内訳です。AOJ 222、yukicoder 192、AtCoder 172、Library Checker 161、LOJ 25、HackerRank 15、IOI 7、CSES 2、Codeforces 2 です。6 年ぶんの蓄積で、実際に解いて通した実績なので再生産できません。問題定義への変換は機械的にできますが、提出の中身は書き直しになります。全部を一度に持ってこず、よく使うものから順に移してください。

## 採らなかった選択肢

劣化の自動判定は入れません。しきい値の調整が面倒なので後回しにします。記録は溜まっていくので、欲しくなったときに足せます。データモデルを変える必要はありません。

callgrind と llvm-mca は使いません。callgrind で命令数を数えるなら、計測区間を切るマクロと問題ごとの小さい固定入力が必要になります。機構が増えるので見合いません。llvm-mca はループ回数を見ないので変化検出になりません。基本ブロック単体を理想条件で見るだけなので、予測にもなりません。既存の judge にある `run-callgrind.sh`、`merge-callgrind.py`、`analyze-mca.py`、`count-simd.py` は移しません。

計測の繰り返しは入れません。変動係数の中央値が 0.69% なので 1 回で足ります。レコードに項目が無いものを 1 と見なせばよいので、あとから足すのも安く済みます。

ただし戻ってくる条件があります。x64 の変動係数は 90% 点が 11% から 14% あって、`gf2-64` の pclmul 系のような特定の問題に固まっていました。1 つのキーは一度しか測られないので、そういう問題で順位が一度おかしく出ると、キーが変わるまで直りません。困ったら同じジョブ内で繰り返して最小値を取る形を足してください。

ただしジョブ内の繰り返しでは直らない可能性が高いです。あの尾は AVX や GNU_TARGET("pclmul") を詰めたループでの周波数の振れ方なので、同じジョブの中では全部同じように落ち込みます。5 回繰り返しても 5 回とも低い値が出るだけです。

そのときに採るべきはジョブ内の繰り返しではなく、**実行をまたいだ標本の蓄積**です。別のマシン、別の日、別の混雑状況で測るので、独立した標本になります。実際に効いている変動が見えます。

やり方はスキップの判定を変えるだけです。判定を「記録の有無」から「記録が n 件あるか」に変えます。判定のときにそのキーの記録はどのみち読むので、数えるのは無料です。新しい仕組みが要りません。

確率的に再実行する案もありますが、n 件で打ち切る方が良いです。仕事も保管も上限が決まって収束します。確率だと止まらず、キーごとの標本数も揃いません。

この形には副産物があります。ソースが安定しているキーほど標本が溜まり、頻繁に書き換えているキーには溜まりません。キーの寿命がソースを触る間隔で決まるからです。順位を見たいのは安定している方なので、欲しい場所に標本が集まります。

この場合は 1 つのキーに複数の記録が付きます。順位を最小値で定義してあるので、表示側は変わりません。

実行時間を愚直解との比に正規化する方式は採りません。比の安定性は、比べる相手がどれだけ似ているかで決まります。SIMD 実装とスカラーの愚直解の比はマイクロアーキテクチャで変わります。基準にすると誤った順位が出ます。同じ手法の他人の実装との比なら安定します。

gist は使いません。提出ページにソースが出るので、そこが読む場所になります。

## 決めていないこと

- Library 側の API 設計です。変種を掛け算でファイルにするか、テンプレート引数に畳むかが決まっていません。このリポジトリの設計には影響しません。
- C++ の規格を 20 と 23 のどちらにするかです。`environments.toml` の 1 行なので後から変えられます。judge では既に `gnu++23` が動いています。
- 798 問を全部移すか、一部だけ移すかです。M4 で数問を足してみてから、残りをどうするか決めてください。

## 実装するときの注意

`uv` を使ってください。`pip install` とシステム Python への直接インストールは避けてください。依存は `pyproject.toml` に書いて `uv.lock` をコミットします。使い捨てのスクリプトは PEP 723 のインラインメタデータで書いて `uv run` で動かします。

`pj` 自身のテストを pytest で書いてください。特に include 閉包の解決、キーの計算、出力比較の 3 つは固めておく価値があります。ここが壊れると記録の意味が変わります。

リポジトリは public にしてください。public だと Actions の分数が無料になり、arm ランナーも無料枠の対象に入ります。4 環境ぶんのマトリクスは private の枠に収まりません。

他サイトのテストデータを public なリリースに置かないでください。ミラーが必要なら private なリポジトリに置いて PAT で取ります。AtCoder が公開を止めた理由がジャッジシステムの複製だったので、同じことをしないようにします。
