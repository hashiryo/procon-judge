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
    <id>/
      problem.toml
      base.cpp              ハーネス (kind = "base" のとき)
      common.hpp            提出が共通で使うもの (省略可)
      submissions/
        lib.hpp
        naive.hpp
        dirty_simd.hpp
      gen.py                ジェネレータ (source = "local" のとき)
      reference.hpp         期待出力を作る参照実装 (同上)
      checker.cpp           独自チェッカ (自分で書くときだけ)
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

`id` はディレクトリ名と一致させます。検証で一致しなければエラーにします。

### ハーネスの種別

`kind = "base"` が既定です。問題が `base.cpp` を持ち、提出は問題が決めたインターフェースを実装します。入出力はハーネスが担当し、計測区間を挟みます。

```cpp
// problems/yosupo-point-add-range-sum/base.cpp
#include "common.hpp"
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

  fprintf(stderr, "PJ_METRICS {\"algo_time_ns\":%lld}\n",
          (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
  fputs(out.c_str(), stdout);
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

特定の実装だけが有利になるインターフェースは避けてください。比較のための土台です。ある方針だけが余計なコピーや変換を強いられる形だと、その分まで測定に乗ります。

入力は全部読んでから計測区間に入ります。出力も文字列に溜めてから最後に書きます。こうすると入出力の時間が計測から外れます。

計測値は stderr に 1 行の JSON で出します。`PJ_METRICS ` という接頭辞を付けて、実行側はその接頭辞で始まる行だけを拾います。stdout に混ぜると出力比較が壊れるので、stdout は使いません。

接頭辞で絞る形にしておくと、他の出力が stderr に混ざっても平気です。サニタイザを入れた環境や、ライブラリが警告を吐く場合に効きます。JSON にしておくと、項目を足すときも JSON のキーが増えるだけで済みます。

専用のファイルに出す方式は採りません。パスを環境変数で渡す必要があり、渡されなかったときの退避も要ります。ケースごとにファイルを作って読んで消す手間も増えます。stderr は既定でバッファされないので、あとで異常終了しても行が残ります。

判定サイトに貼っても影響しません。AtCoder、Library Checker、AOJ、yukicoder のいずれも stderr を判定に使いません。

メモリは実行側が外から測ります。`base.cpp` の中で測ろうとしないでください。

内側からでも `getrusage(RUSAGE_SELF)` の `ru_maxrss` で取れますが、プロセスが死んだケースで値が失われます。外から `wait4(pid, &status, 0, &ru)` を呼べば、子が異常終了したあとでもピーク RSS が返ります。OOM による強制終了や segfault のあとでも取れます。MLE や RE のときにメモリが分かるのはこちらだけです。`raw` の提出でも扱いが揃うという利点もあります。

`ru_maxrss` の単位はプラットフォームで違います。**Linux は KB、macOS はバイト**です。手元が Mac で CI が Linux なので、変換をプラットフォームごとに分けてください。

RSS には実行ファイルと libc の共有ページが入るので、何もしないプログラムでも数 MB の下駄を履きます。既存の judge のレコードで素の naive が 3792 KB になっているのがそれです。`mle_mb` をきつく設定しないでください。

メモリ制限は強制しません。`wait4` で取ったピークが `mle_mb` を超えていたら MLE にする後判定で足ります。`setrlimit(RLIMIT_AS)` は触っていない予約まで引っかかるのでアロケータと相性が悪く、cgroup v2 はランナー上で扱う手間が大きいからです。暴走した確保が怖い場合は、`mle_mb` よりずっと大きい値で `RLIMIT_AS` を保険に入れてください。

既存の判定サイトのテストデータがそのまま使えます。ハーネスが判定サイトの入力形式で読んで出力形式で書くので、テストデータを加工する必要がありません。この形を選んだ一番の理由です。

`kind = "raw"` は逃げ道です。提出が `main()` ごと持ちます。対話式の問題、オンラインクエリの問題、それと入出力を持たない自己検証の提出で使います。計測区間が無いので `algo_time_ns` は記録されず、プロセス全体の時間だけ取ります。

入力を全部メモリに読むので、メモリ使用量は元の問題の想定と変わります。`mle_mb` は「このハーネスでの制限」として扱ってください。元の判定サイトの制限と一致させる意味はありません。

### テストデータの取得元

| source | 説明 | 追加の項目 |
| --- | --- | --- |
| `library_checker` | `yosupo06/library-checker-problems` を clone して生成します | `name` に問題のパス |
| `aoj` | judgedat の API から取得します | `name` に問題 ID |
| `yukicoder` | API から取得します。トークンが必要です | `name` に問題 ID |
| `manual` | 保管庫と手動の取り込みだけを使います。原本を叩きません | `name` に識別子 |
| `local` | リポジトリ内で生成します | `generator`、`count`、`reference` |
| `none` | テストデータを使いません | なし |

`local` の書き方です。

```toml
[testdata]
source = "local"
generator = "gen.py"      # 既定値
count = 200
reference = "reference.hpp"   # kind = "base" なら提出と同じ形
```

`gen.py` が seed を引数に取ってランダムな入力を stdout に出します。参照実装が期待出力を作ります。`count` 個の seed を 0 から順に使うので、生成は決定的になります。

`local` は `name` を持ちません。キャッシュのハッシュは問題 id から作ります。加えて `gen.py` の内容、参照実装の内容、`count` を混ぜてください。ジェネレータを直したときに、古い生成結果が使い回されるのを防ぐためです。

`kind = "base"` の問題では、参照実装も提出と同じ形にしてください。`reference` に `.hpp` を指定します。ハーネスと一緒にコンパイルして期待出力を作ります。こうすると入力の解析を二重に書かなくて済みます。`kind = "raw"` の問題なら、参照実装は単体で動く `.cpp` になります。

`none` はテストデータを持たない問題です。使い方が 2 通りあります。自己検証の提出なら `compare.kind = "exit_code"` にして、空の標準入力で走らせて終了コードを見ます。入力を読む提出なら `compare.kind = "compile_only"` にします。実行せず、コンパイルだけを見ます。

AtCoder は 2024 年 12 月からテストケースの公開が停止しています。告知は https://atcoder.jp/posts/1376 です。再開の見通しは無いので、AtCoder の問題は `source = "none"`、`harness.kind = "raw"`、`compare.kind = "compile_only"` にします。実行はしません。入力が無いので、走らせても落ちるだけです。

取得したテストデータは `.cache/testcases/<ハッシュ>/` に置きます。ハッシュは `source` と `name` から作ります。内容から作る `cases_hash` とは別物です。取得する前に引ける必要があるからです。CI では actions/cache で持ち越します。キャッシュは 7 日使われないと消えるので、消えても再取得できる形にしておきます。

取得の実装は既存のコードを移植してください。判定サイトごとの癖が入っているので、書き直すと同じ罠を踏みます。参照するファイルは次のとおりです。

```
hashiryo/Library     scripts/lib/download.py
hashiryo/judge       scripts/download-testcases.py
hashiryo/CPtools     loj_download.py
                     url_hash.md
                     yukicoder-spjudge-guide.md
```

LOJ はリアルタイムで取得できません。`CPtools/loj_download.py` の中身を確認しました。まず `getSubmissionDetail` にある提出 ID からケースのファイル名一覧が得られます。そのあと `downloadProblemFiles` で本体が落ちてきます。つまり自分がその問題に提出して結果を得ていないと、ファイル名は分かりません。IOI、CSES、Codeforces も実装がありません。これらは `source = "manual"` にします。

AOJ の judgedat は大きいケースを切り詰めて返すことがあります。取得したあとに header のケース数とサイズを突き合わせてください。合わなければ警告にします。切り詰められたデータで判定すると、結果の意味が変わります。

### テストデータの保管

リアルタイムで取得できない取得元があります。取得できるものでも、毎回原本を叩くのは遅くなります。レート制限と障害が経路に入り込みます。だから一度落としたテストデータは自分の置き場に保管します。

置き場は private リポジトリの Release アセットにします。リポジトリ名は `procon-judge-testdata` にして private で作ります。タグは 1 本だけ用意して、そこへアセットを足していきます。判定システム側からは PAT で読みます。secret 名は `TESTDATA_TOKEN` にします。

この形を選ぶ理由は 4 つです。

- 新しいサービスも新しい認証も増えません。`gh` の PAT だけで済みます。
- 無料で、egress の課金もありません。
- private なので公開ミラーになりません。
- アセットは git の外にあるので、リポジトリが太りません。

リリースは `testdata` の 1 本だけ作って、そこに 1 問 1 アセットで貼ります。名前は `<問題 id>.tar.zst` にします。問題 id がディレクトリ名なので、アセット名は対応表なしで計算できます。中身は入出力のペアと `manifest.json` です。manifest にはケース名、サイズ、内容のハッシュ、取得元、取得した日時を入れます。

アップロードとダウンロードはアセット単位です。1 問足すときは upload 1 回で済みます。他のアセットに触らず、コミットも発生しません。落とすときは `-p` でパターンを指定すると、その 1 個だけ取れます。

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
| 1 | `.cache/testcases/` | 手元のキャッシュ |
| 2 | actions/cache | CI のキャッシュ。7 日使われないと消えます |
| 3 | 保管庫 | private リポジトリの Release アセット |
| 4 | 原本 | 判定サイトの API またはジェネレータ |

4 まで到達したら、成功したあとに 3 へ上げます。一度保管すれば、CI が原本を叩かなくなります。

`cases_hash` はケースの内容から計算してください。取得経路に依存させてはいけません。保管庫と原本のどちらから取っても同じ値になる必要があります。違う値になると、キーが一致しません。キャッシュが無駄に消えます。

手動の取り込みの流れです。

1. 手元でテストデータを落とします。LOJ なら `loj_download.py` を使います。
2. `pj testdata import --problem ID --dir PATH` で取り込みます。
3. `pj mirror push --problem ID` で保管庫へ上げます。

以降は CI が保管庫から取るので、原本に触りません。

容量の見積もりです。Library Checker の 161 問はジェネレータで作るので保管が不要です。AtCoder の 172 問は入手できません。残るのは AOJ 222、yukicoder 192、その他 50 ほどで、合わせて 460 問前後です。圧縮して 1 問あたり数 MB なら、全体で数 GB に収まります。Release アセットは 1 ファイル 2 GB までなので余裕があります。

actions/cache だけで済ませてはいけません。7 日で消えるので durable ではありません。既存の Library が `tc.zip` を Release へ置いているのは、それに気づいた結果です。

Cloudflare R2 は 10 GB まで無料で egress も無料なので候補になります。ただし認証が 1 つ増えます。GitHub で足りるので使いません。

### 実行と打ち切り

`tle_sec` はケースごとの制限です。測るのはプロセス全体の実時間で、`algo_time_ns` とは別です。

超えたらそのケースでプロセスを kill して `TLE` にします。メモリと違って後判定にできません。無限ループの提出があると、GitHub の 6 時間の上限までジョブが埋まるからです。

最初の非 AC が出たら、残りのケースは走らせません。`failed_case` が単数なのはそのためです。打ち切った場合、`time_max_ms` と `time_total_ms` はそこまでの値になります。

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

実行対象はこれだけにします。過去に置いていたものを再実行する仕組みは作りません。

提出のパスがそのまま安定した識別子になります。ライブラリの解説から張るリンクはこのパスで、そのページが最新の記録を描きます。

先頭が `_` のファイルは提出として扱いません。共通ヘッダの置き場に使います。

慣習として、`lib` を「自分のライブラリを使った提出」の名前にします。ただし `pj` はこの名前に意味を持たせません。ライブラリとの紐付けは include の一覧から導きます。

## ライブラリの取得

ライブラリは submodule で pin しません。pin すると最新を指すという目的が達成できないので、実行のたびに取り直します。

CI は既定ブランチを `lib/` へ clone して、その SHA を記録の `library_sha` に入れます。全環境のフラグに `-I lib` を足すので、提出は `#include "mylib/data_structure/SegmentTree.hpp"` の形で書けます。

記録の `includes` は `lib/` を外した相対パスで書きます。ライブラリ側のサイトが自分のパスと突き合わせるためです。

ライブラリが取得できなくても問題は走ります。ライブラリを使わない提出があるからです。取得に失敗したら、ライブラリを include する提出だけを飛ばします。

ライブラリ側の push からこちらを起こすには `repository_dispatch` を使います。`GITHUB_TOKEN` では別リポジトリが起こせないので、PAT をライブラリ側の secret に置く必要があります。

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
  "cxxflags": "-std=gnu++23 -O2 -march=x86-64-v3 ...",
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
  "failed_case": null,
  "timestamp": "2026-09-19T10:00:00Z"
}
```

`status` は `AC`、`WA`、`TLE`、`MLE`、`RE`、`CE` のいずれかです。

`includes` はライブラリとの紐付けに使います。「このコンパイルはこのパスを含んだ」という中立な事実なので、ライブラリ固有の概念をこのリポジトリに持ち込みません。

`failed_case` は失敗したときだけ埋めます。中身はケース名、状態、時間、メモリ、出力の差分の先頭です。AC のときはケースごとの明細を捨てます。1 レコードの大きさが 10 分の 1 ほどになります。

記録の件数を抑えているのはスキップの方です。件数はキーの種類の数で決まり、実行の頻度では増えません。明細を捨てるのは 1 レコードの大きさの話で、別の軸です。

## キーとスキップ

CPU モデルが同じなら実行時間は再現します。judge の履歴 17705 件で測ったところ、変動係数の中央値は 0.69% でした。逆に CPU モデルを無視すると中央値比で 1.14 倍ずれます。だから CPU モデルをキーに入れると、過去の記録をそのまま比較に使えます。

キーの計算はこうします。

```
normalize(src) = 行末空白除去 + 空行除去

submission_hash = sha256( normalize(提出のソース)
                          + Σ sorted(include 閉包) の (パス + normalize(内容)) )
harness_hash    = sha256( normalize(base.cpp) + normalize(common.hpp) )
problem_hash    = sha256( problem.toml を正規化したもの )

key = sha256( 提出のパス, submission_hash, harness_hash, problem_hash, cases_hash,
              env, compiler_version, cxxflags, cpu_model )
```

提出のパスをキーに入れるのは、中身が同じ提出を別物として数えるためです。`submission_hash` は中身と include 閉包だけから作るので、バイト単位で同じ提出が 2 本あると同じ値になります。提出ページはパスごとに描くので、パスが違えば別の記録が要ります。リネームすると測り直しになりますが、新しいパスには記録が無いので、そちらの方が欲しい動きです。

`problem_hash` は実行に影響する項目だけから作ります。`limits`、`harness`、`testdata`、`compare` です。`title` のような表示用の項目は外します。題名を直しただけで全部が走り直すのを避けるためです。

`kind = "raw"` の問題には `base.cpp` がありません。その場合の `harness_hash` は空文字列のハッシュにします。

**このキーの記録が既にあれば実行をスキップします。**

整形を落としてからハッシュを取るのが要点です。ライブラリを磨いている時期は整形だけの変更が頻発するので、これを入れないと中身が同じなのに全部走り直します。

コメント除去は入れていません。正しく作るには文字列リテラルの中の `//`、生文字列リテラル、行継続を扱う必要があり、実質 C++ の字句解析器になります。ここでバグると記録の意味が変わります。空白の正規化だけなら数行で安全に書けます。コメントだけを変えたときに 1 回余分な実行が走りますが、壊れはしません。必要になったら任意の改良として足してください。

include 閉包は `#include "..."` を再帰的に辿って作ります。角括弧の include は辿りません。閉包の解決は `pj/include.py` に置いて、pytest で固めてください。

閉包を辿るときの include パスは、コンパイル時と同じにしてください。`lib/`、問題のディレクトリ、`third_party/simde` です。ずれているとライブラリのヘッダを解決できず、閉包が欠けます。欠けた閉包はキーを誤らせるので、ライブラリを直しても再実行されなくなります。

この設計から出る結果を 4 つ書きます。

順位を出すために同じジョブへ同居させる必要がありません。同じ CPU モデルと環境の記録を集めて並べるだけで順位が出ます。問題単位で全提出をまとめて走らせる仕組みは要りません。

広く依存されたヘッダを触っても、走るのはそれを含む提出だけです。他の提出の記録は既にあるので再実行しません。

CPU モデルはジョブが始まるまで分かりません。したがって「何を走らせるか」の判定はジョブの中で行います。事前に計画を立てる別ジョブは不要です。

**今の設計では、1 つのキーは 1 回しか測られません。** 同じ条件での再測定が起きないからです。記録が増えるのはキーが変わったときだけで、それは別の測定です。だから測定値の質はその 1 回で決まります。変えたくなったときの手は「採らなかった選択肢」に書いてあります。

## 環境

`environments.toml` に定義します。環境名はキーの一部なので、一度決めたら変えないでください。

```toml
[[env]]
name = "x64-gcc"
runs_on = "ubuntu-24.04"
cxx = "g++-15"
cxxflags = "-std=gnu++23 -Wall -Wextra -O2 -march=x86-64-v3 -flto=auto -pthread"
tier = 1

[[env]]
name = "x64-clang"
runs_on = "ubuntu-24.04"
cxx = "clang++-21"
cxxflags = "-std=gnu++23 -Wall -Wextra -O2 -march=x86-64-v3 -flto=auto -pthread -fuse-ld=lld"
tier = 2

[[env]]
name = "arm-gcc"
runs_on = "ubuntu-24.04-arm"
cxx = "g++-15"
cxxflags = "-std=gnu++23 -Wall -Wextra -O2 -march=armv8.2-a -flto=auto -pthread -DUSE_SIMDE -DSIMDE_ENABLE_NATIVE_ALIASES -Ithird_party/simde"
tier = 1

[[env]]
name = "arm-clang"
runs_on = "ubuntu-24.04-arm"
cxx = "clang++-21"
cxxflags = "..."
tier = 2

[[env]]
name = "local"
runs_on = "self"
cxx = "c++"
cxxflags = "-std=gnu++23 -Wall -Wextra -O2 -DUSE_SIMDE -DSIMDE_ENABLE_NATIVE_ALIASES -Ithird_party/simde"
tier = 0
```

`tier = 0` は手元専用で、CI では走らせません。Mac の Apple clang を使うので `-march` は付けません。

`tier = 1` の環境は変更ごとに走らせます。`tier = 2` は定期実行と手動のときだけ走らせます。これで変更ごとの費用が半分になります。

`pj` は `environments.toml` の `cxxflags` に `-I lib` と問題のディレクトリを足してからコンパイルします。記録の `cxxflags` には足したあとの文字列をそのまま入れてください。キーの一部なので、記録と実際のコンパイルがずれてはいけません。

arm では SIMDe を使います。手元の Mac が arm で提出先が x86 なので、x86 の intrinsics をそのまま書いて arm で動かすために必要です。SIMDe は `third_party/simde` に submodule で入れます。

## CLI

```
pj problems list [--json]
pj problems check                            problem.toml の検証
pj submissions list [--problem ID]
pj fetch [--problem ID] [--all]              テストデータ取得
pj testdata import --problem ID --dir PATH   手元で落としたものを取り込む
pj mirror push --problem ID                  保管庫へ上げる
pj mirror pull --problem ID                  保管庫から落とす
pj mirror status                             問題ごとの保管状況
pj run  --env NAME [--shard i/N] [--budget N]   実行して記録を出す
pj run  --dry-run --env NAME                 走らせる対象を出すだけ
pj run  --problem ID --submission PATH --env NAME   1 件だけ走らせる
pj records append <dir>...                   まとめて results の jsonl へ追記する
pj records squash                            results ブランチの履歴を畳む
pj bundle --problem ID --submission PATH     1 ファイルに展開する
pj site build --out DIR
```

手元と CI が同じコマンドを使います。CI 前提の分岐を CLI の中に入れないでください。手元では `--problem` と `--submission` で 1 件に絞り、CI では `--shard` と `--budget` で分割します。違うのは引数だけです。

`pj run` の流れはこうです。

1. CPU モデルを検出します。
2. 対象の提出を列挙します。
3. 各提出のキーを計算します。
4. 既存の記録にあるキーを除きます。
5. `sha256(key) % N == i` で自分の担当だけ残します。
6. `--budget` の件数まで実行します。
7. 記録を JSON Lines で出力します。

5 の分割は決定的なので、並列で走るジョブが調整なしに重複を避けられます。

## CI

ワークフローは 1 つです。`.github/workflows/judge.yml`。

引き金は 4 つです。

| 引き金 | 用途 |
| --- | --- |
| `push` (main) | 問題や提出を足したとき |
| `repository_dispatch` | ライブラリ側の更新から起こすとき |
| `schedule` | 取りこぼしを埋めるとき。1 日 1 回 |
| `workflow_dispatch` | 手動 |

```yaml
concurrency:
  group: judge
  cancel-in-progress: false
```

記録を失いたくないので、実行中のものを打ち切らずに待たせます。

ジョブは 2 段です。

`run` は matrix で環境 × シャードに広がります。各ジョブの手順は次のとおりです。

1. リポジトリを checkout します。
2. `results` ブランチを `fetch-depth: 1` で読みます。
3. テストデータのキャッシュを復元します。
4. ライブラリを HEAD で取得します。
5. `pj run` で担当分を実行します。
6. 記録をアーティファクトにアップロードします。

テストデータの取得は `pj run` の中で行います。対象は実際に走らせる問題だけです。キーの一致で全部スキップされた実行では、1 件も落としません。

`collect` は 1 つだけ走ります。手順は次のとおりです。

1. アーティファクトを集めて `pj records append` で問題ごとの jsonl へ追記します。
2. `results` ブランチを push します。
3. `pj site build` でサイトを作って Pages にデプロイします。

並列で `results` に push すると衝突するので、押すのは `collect` だけにします。`run` は記録をアーティファクトに置くだけです。

GitHub の同時実行ジョブ数はプランで決まります。Free で 20 前後、Pro で 40 前後です。マトリクスの上限は 1 実行あたり 256 ジョブです。分割を細かくしても並列度は上がらないので、シャード数は同時実行数に合わせてください。

`--budget` で 1 回の実行量に上限を付けます。残りは次の実行に回ります。記録の無いキーの集合が減っていく形にして、「すべてのコミットが検証済み」を目指すのはやめます。サイトには検証済みか検証待ちかを出します。

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

実行した結果はそのまま追記します。記録が作られるのは必ず新しいキーのときなので、重複は起きません。

読むときは必ず `fetch-depth: 1` にします。先端の状態しか要らないので、履歴が増えても CI の時間が変わりません。

太ってきたら `pj records squash` で履歴を捨てて 1 コミットに force push し直します。orphan branch なので履歴に意味がありません。既存の Library は 13 MB の結果ファイルを 63 回コミットして `.git` が 158 MB になっているので、同じことを起こさないようにします。

## サイト

Pages のアーティファクトに入れます。`actions/upload-pages-artifact` と `actions/deploy-pages` を使うので、ブランチを経由しません。サイトはビルド出力なので保存しません。記録から作り直せるからです。

npm を使いません。Python でテンプレートを埋めて HTML を出し、対話は素の JavaScript で書きます。図も inline SVG をその場で組みます。既存の judge の `docs/index.html` が単一の HTML で `fetch` するだけの作りなので、その延長です。

```
site/
  index.html                 問題一覧
  problems/<id>.html         順位表
  submissions/<問題 id>/<提出名>.html   1 提出の記録と時系列
  data/index.json
  data/problems/<id>.json
  data/submissions/<問題 id>/<提出名>.json
  data/headers/<path>.json   そのヘッダを含む提出の一覧
```

**ページごとに必要な形の JSON をビルド時に生成します。** ブラウザは自分に必要な 1 個か 2 個だけ取ります。結合や反転はすべて生成側で済ませます。

1 つの大きい JSON を読ませてはいけません。既存の judge の `docs/index.html` は 48 MB の `history.jsonl` を `fetch` しています。これを JSON にパースすると JS のオブジェクトで数百 MB になるので、スマートフォンでは落ちます。

大きさの目安です。問題 800 問で 1 行 150 バイトなら `index.json` が 120 KB です。問題のページは提出 10 本 × 環境 4 × CPU モデル 4 で 160 行なので 20 KB です。問題が増えても 1 ページの読み込み量は変わりません。

順位はそのキーの記録の最小値で決めます。最新の記録ではありません。今はキーごとに記録が 1 つなので同じ値になりますが、あとで標本を積み始めたときに表示側を書き直さずに済みます。

問題のページには CPU モデルの切り替えを置きます。4 モデルから 1 つ選ぶと、そのモデルの記録だけで順位が出ます。記録の無い提出は「未計測」として並べます。x86 は 3 モデルに散るので、穴を見せる方が正直です。

提出のページにはソースを出します。展開は `pj bundle` が行います。出すのは bundle 済みのソースで、貼れる 1 ファイルとしてもそれが欲しいものになります。gist は使いません。提出ページがソースを読む場所になるからです。

Pages のキャッシュヘッダは細かく制御できないので、データファイルの名前かクエリにビルドのハッシュを混ぜて、古いデータを見せないようにします。

Pages のデプロイはサイト全体の差し替えです。変わった問題の分だけ生成しても配信は全量になりますが、800 問ぶんの小さい JSON を作り直すだけなら一瞬なので気にしなくてよいです。

## 最小の第一版

先に全部作らないでください。次のものは第一版から外せます。どれもあとから足せます。

| 外すもの | 足す時期 |
| --- | --- |
| `tier` | 4 環境を毎回回して困ってから |
| `--shard` と `--budget` | 1 ジョブで捌けなくなってから |
| `state.json` | jsonl の走査が遅くなってから |
| 保管庫 | AOJ や yukicoder の問題を足すとき |
| `aoj`、`yukicoder`、`manual` の取得 | 同じとき |
| `compare.kind` の `float` と `exit_code` | 必要な問題が出てから |
| 提出ページとヘッダ別の JSON | ライブラリ側のサイトを作るとき |
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

M1 の題材は Library Checker の `point_add_range_sum` を勧めます。テストデータがジェネレータから作れるので判定サイトへの依存が無く、チェッカが同梱されていて、`base.cpp` のインターフェースが素直です。

M1 の時点で `problem.toml`、`base.cpp`、`submissions/naive.hpp` を手で書いて、それを通してください。インターフェースは仕様書ではなく、この最初の 1 問に決めさせます。

## 実装の記録

M2 まで実装しました。`pj run --env local` を 2 回叩くと 2 回目が 0 件になります。以下は実装しながら決めたことです。

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

`.cache/testcases/<ハッシュ>/` には in と out のペア、`manifest.json`、それにチェッカ一式 (`checker.cpp`、`testlib.h`、`params.h`) を置きます。`params.h` は生成のときに作られるので、clone しただけでは存在しません。チェッカのヘッダを一緒に置かないとコンパイルに失敗します。

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

`upload-artifact` は隠しファイルを既定で除きます。最初 `--out .artifacts` にしていたら 1 件も上がらず、`collect` が 0 件で終わりました。`if-no-files-found: ignore` にしていたので、警告も出ずに黙って落ちていました。`${{ runner.temp }}/records` に置き換えて、`if-no-files-found` は `warn` にしました。全部スキップされた実行では 1 件も出ないので、失敗にはできません。

### Linux の RSS は下駄が高いです

ubuntu-24.04 のランナーで測ると、入力が 45 バイトの `example_00` でも 22760 KB 出ます。小さいケースは全部この値で揃うので、これがこのランナーの下限です。手元の macOS は同じケースで 1408 KB でした。ケースの大きさで動くぶんはこの上に乗ります (`max_random` で 29820 KB)。設計が言うとおり `mle_mb` をきつく設定しないでください。

### マトリクスは今のところ手書きです

`.github/workflows/judge.yml` の matrix に `x64-gcc` と `ubuntu-24.04` を直に書いています。`environments.toml` と二重持ちですが、yml から toml を読むには matrix を作る前段のジョブが要ります。4 環境に広げる M5 でまとめて考えます。

### テストデータのキャッシュ

`actions/cache` で `.cache/testcases` を持ち越します。`point_add_range_sum` 1 問で 129 MB なので、問題を増やすと GitHub の 10 GB に当たります。M4 の保管庫を入れるときに、キャッシュへ何を残すかを決め直します。


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

ただしジョブ内の繰り返しでは直らない可能性が高いです。あの尾は AVX や PCLMUL を詰めたループでの周波数の振れ方なので、同じジョブの中では全部同じように落ち込みます。5 回繰り返しても 5 回とも低い値が出るだけです。

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
