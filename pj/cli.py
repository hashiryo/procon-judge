"""コマンドの入口。

手元と CI が同じコマンドを使う。CI 前提の分岐はここに入れない。
"""

from __future__ import annotations

import argparse
import json
import sys
import time
from collections.abc import Sequence
from pathlib import Path

from . import batch as batch_mod
from . import claims as claims_mod
from . import environment as env_mod
from . import fetch
from . import migrate as migrate_mod
from . import plan as plan_mod
from . import problem as problem_mod
from . import repro as repro_mod
from . import run as run_mod
from . import titles as titles_mod
from .fetch import mirror
from .paths import LIB_DIR, RESULTS_DIR, SITE_DIR
from .site import build as site_build
from .store import Store


def _die(message: str) -> int:
    print(f"error: {message}", file=sys.stderr)
    return 1


def cmd_problems_list(args: argparse.Namespace) -> int:
    rows = []
    for directory in problem_mod.all_problem_dirs():
        try:
            p = problem_mod.load(directory)
        except problem_mod.ProblemError as e:
            if args.json:
                rows.append({"id": directory.name, "error": str(e)})
            else:
                print(f"{directory.name}\tERROR: {e}")
            continue
        entry = {
            "id": p.id,
            "title": p.title,
            "harness": p.harness_kind,
            "source": p.testdata.source,
            "compare": p.compare.kind,
            "submissions": [s.as_posix() for s in p.submissions()],
        }
        if args.json:
            rows.append(entry)
        else:
            print(
                f"{p.id}\t{p.title}\t{p.harness_kind}/{p.testdata.source}"
                f"/{p.compare.kind}\t{len(entry['submissions'])} submissions"
            )
    if args.json:
        print(json.dumps(rows, ensure_ascii=False, indent=1))
    return 0


def cmd_problems_check(args: argparse.Namespace) -> int:
    directories = problem_mod.all_problem_dirs()
    if args.problem:
        directories = [d for d in directories if d.name == args.problem]
        if not directories:
            return _die(f"問題 {args.problem!r} がありません")
    failed = 0
    warned = 0
    for directory in directories:
        try:
            p = problem_mod.load(directory)
        except problem_mod.ProblemError as e:
            print(f"NG  {directory.name}: {e}")
            failed += 1
            continue
        for message in problem_mod.warnings(p):
            print(f"WARN {p.id}: {message}")
            warned += 1
        if not p.submissions():
            print(f"NG  {p.id}: submissions/ に提出がありません")
            failed += 1
            continue
        print(f"OK  {p.id}  ({len(p.submissions())} submissions)")
    if warned:
        print(f"\n{warned} 件に警告があります", file=sys.stderr)
    if failed:
        print(f"\n{failed} 件が通りませんでした", file=sys.stderr)
    return 1 if failed else 0


def cmd_problems_titles(args: argparse.Namespace) -> int:
    """題名を判定サイトの名前と突き合わせる。--fix で problem.toml を書き換える。"""
    directories = problem_mod.all_problem_dirs()
    if args.problem:
        directories = [d for d in directories if d.name == args.problem]
        if not directories:
            return _die(f"問題 {args.problem!r} がありません")
    titles = titles_mod.Titles()
    mismatched = failed = 0
    for directory in directories:
        try:
            p = problem_mod.load(directory)
        except problem_mod.ProblemError as e:
            print(f"NG  {directory.name}: {e}")
            failed += 1
            continue
        try:
            official = titles.official(p)
        except titles_mod.TitleError as e:
            print(f"??  {p.id}: 取れませんでした: {e}")
            failed += 1
            continue
        if official is None:
            print(f"--  {p.id}  ({p.testdata.source} には判定サイトの名前が無い)")
            continue
        if official == p.title:
            print(f"OK  {p.id}  {p.title}")
            continue
        if args.fix:
            try:
                titles_mod.rewrite_title(directory / "problem.toml", official)
            except titles_mod.TitleError as e:
                print(f"NG  {p.id}: {e}")
                failed += 1
                continue
            print(f"FIX {p.id}  {p.title!r} -> {official!r}")
        else:
            print(f"NG  {p.id}  {p.title!r} (判定サイトは {official!r})")
            mismatched += 1
    if mismatched:
        print(f"\n{mismatched} 件が判定サイトと違います。--fix で書き換えます", file=sys.stderr)
    if failed:
        print(f"\n{failed} 件を確かめられませんでした", file=sys.stderr)
    return 1 if (mismatched or failed) else 0


def cmd_problems_import(args: argparse.Namespace) -> int:
    """competitive-verifier のテストを raw の問題として取り込む。"""
    try:
        files = migrate_mod.collect_files([Path(p) for p in args.paths])
    except migrate_mod.MigrateError as e:
        return _die(str(e))
    existing = {d.name for d in problem_mod.all_problem_dirs()}
    items = migrate_mod.plan(
        files,
        existing=existing,
        single_only=args.single_only,
        titles=titles_mod.Titles(),
    )
    migrate_mod.report(items, dry_run=args.dry_run)
    if args.dry_run:
        return 0
    for item in items:
        if not item.skipped:
            migrate_mod.write(item)
    return 0


def cmd_submissions_list(args: argparse.Namespace) -> int:
    directories = problem_mod.all_problem_dirs()
    if args.problem:
        directories = [d for d in directories if d.name == args.problem]
        if not directories:
            return _die(f"問題 {args.problem!r} がありません")
    for directory in directories:
        p = problem_mod.load(directory)
        for submission in p.submissions():
            print(f"{p.id}\t{submission.as_posix()}")
    return 0


def _problems(args: argparse.Namespace) -> list[problem_mod.Problem]:
    if getattr(args, "problem", None):
        return [problem_mod.load_by_id(args.problem)]
    return [problem_mod.load(d) for d in problem_mod.all_problem_dirs()]


def cmd_fetch(args: argparse.Namespace) -> int:
    if not args.problem and not args.all:
        return _die("--problem か --all のどちらかを指定してください")
    for p in _problems(args):
        if not fetch.needs_testdata(p):
            print(f"{p.id}: テストデータを使いません")
            continue
        # --from-origin は保管庫を疑うとき用。判定サイトを叩くので既定では通らない。
        testcases = fetch.ensure(
            p,
            refresh=args.refresh or args.from_origin,
            allow_mirror=not args.from_origin,
            env=env_mod.load(args.env),
        )
        print(
            f"{p.id}: {testcases.count} cases  cases_hash={testcases.cases_hash}  "
            f"{testcases.dir}"
        )
    return 0


def cmd_testdata_import(args: argparse.Namespace) -> int:
    p = problem_mod.load_by_id(args.problem)
    testcases = fetch.import_dir(p, Path(args.dir))
    print(
        f"{p.id}: {testcases.count} cases 取り込みました  "
        f"cases_hash={testcases.cases_hash}  {testcases.dir}"
    )
    return 0


def cmd_mirror_push(args: argparse.Namespace) -> int:
    p = problem_mod.load_by_id(args.problem)
    if not mirror.should_mirror(p):
        return _die(
            f"{p.id}: source = {p.testdata.source!r} は生成し直せるので保管しません"
        )
    directory = fetch.cache_dir_for(p)
    if not (directory / fetch.MANIFEST_NAME).is_file():
        return _die(f"{p.id}: 先に pj fetch か pj testdata import をしてください")
    mirror.push(p, directory, force=args.force)
    return 0


def cmd_mirror_pull(args: argparse.Namespace) -> int:
    p = problem_mod.load_by_id(args.problem)
    if not mirror.pull(p, fetch.cache_dir_for(p)):
        return _die(f"{p.id}: 保管庫から取れませんでした")
    return 0


def cmd_mirror_status(args: argparse.Namespace) -> int:
    stored = {a["name"]: a for a in mirror.assets()}
    for p in _problems(args):
        name = mirror.asset_name(p)
        if not mirror.should_mirror(p):
            state = "-\t生成し直せるので保管しません"
        elif name in stored:
            state = f"あり\t{stored[name].get('size', '?')} bytes"
        else:
            state = "なし"
        print(f"{p.id}\t{state}")
    return 0


def _targets(args: argparse.Namespace) -> list[run_mod.Target]:
    """走らせる候補を (問題, 提出) の組で列挙する。"""
    if args.submission and not args.problem:
        raise problem_mod.ProblemError("--submission には --problem も要ります")

    if args.problem:
        problems = [problem_mod.load_by_id(args.problem)]
    else:
        problems = [problem_mod.load(d) for d in problem_mod.all_problem_dirs()]

    targets: list[run_mod.Target] = []
    for p in problems:
        if args.submission:
            targets.append((p, problem_mod.resolve_submission(p, args.submission)))
        else:
            targets.extend((p, s) for s in p.submissions())
    return targets


def cmd_plan(args: argparse.Namespace) -> int:
    envs = env_mod.load_all()
    problems = [problem_mod.load(d) for d in problem_mod.all_problem_dirs()]
    store = Store(Path(args.store) if args.store else RESULTS_DIR)
    # lib/ が無いとライブラリを使う提出の閉包が欠ける。欠けた閉包はキーを
    # 誤らせるので、plan はそれを未計測と数えずに警告だけ出す。
    if not LIB_DIR.is_dir():
        print(
            f"warning: {LIB_DIR.name}/ がありません。"
            "ライブラリを使う提出は立てません",
            file=sys.stderr,
        )

    plans = plan_mod.build(problems, envs, store)
    for one in plans:
        parts = [
            f"モデル {len(one.models)} 種",
            f"未計測 {one.expected:.1f} 件/モデル (全モデル {one.total} 件)",
            f"問題 {len(one.works)} 件",
        ]
        print(f"{one.env.name}\t" + " / ".join(parts), file=sys.stderr)
        for submission in one.unresolved:
            print(
                f"warning: {submission} の include を解決できません",
                file=sys.stderr,
            )
        for submission in one.compile_errors:
            print(
                f"{one.env.name}\t{submission} は CE と分かっているので立てません",
                file=sys.stderr,
            )
    groups = plan_mod.group(plans, minutes=args.minutes)
    for one in groups:
        print(
            f"{one.name}\tジョブ {one.jobs} 本 (環境 {', '.join(one.env_names)}、"
            f"未計測 {one.total} 件、問題 {len(one.order)} 件)",
            file=sys.stderr,
        )
    matrix = plan_mod.matrix(groups)
    print(json.dumps(matrix, ensure_ascii=False))
    if args.github_output:
        with Path(args.github_output).open("a") as f:
            f.write(f"matrix={json.dumps(matrix, ensure_ascii=False)}\n")
            f.write(f"any={'true' if matrix['include'] else 'false'}\n")
    return 0


def cmd_run(args: argparse.Namespace) -> int:
    names = [n for n in args.env.split(",") if n]
    envs = [env_mod.load(n) for n in names]
    store = Store(Path(args.store) if args.store else RESULTS_DIR)
    # CI では results ブランチから読んで、記録はアーティファクトに置く。
    # push するのは collect だけなので、run は store を書き換えない。
    out = Store(Path(args.out)) if args.out else store
    if args.claim_run:
        if args.dry_run:
            return _die("--dry-run は --claim-run と併用できません")
        return _run_claiming(args, envs, store, out)
    if len(envs) != 1:
        return _die("--env に複数の環境を渡せるのは --claim-run のときだけです")
    env = envs[0]
    machine = run_mod.Machine.detect(env)
    print(
        f"{env.name} / {machine.cpu_model} / {machine.compiler_version}",
        file=sys.stderr,
    )

    # 1 本だけ指定したときは束を作らない。1 本だけの束が最新になると、表の他の
    # 行が全部「束の外」になる。
    batch_id = None if args.submission else batch_mod.local_id()
    worklist = run_mod.build_worklist(
        _targets(args), env, machine, store.keys(),
        borrowed=store.cases_hashes(),
        allow_fetch=not args.dry_run, refresh=args.refresh,
        batches=store.batches(), batch_id=batch_id,
    )
    _report(worklist)

    timed_out = 0
    if args.dry_run:
        for job in worklist.jobs:
            print(f"run \t{job.label}\t{job.key}")
        for job in worklist.skipped:
            print(f"skip\t{job.label}\t{job.key}")
        for problem, submission in worklist.pending:
            print(f"fetch\t{problem.id}\t{submission.as_posix()}\t-")
        for problem, submission in worklist.blocked:
            print(f"block\t{problem.id}\t{submission.as_posix()}\t-")
    else:
        _, timed_out = _execute(
            worklist.jobs, out, minutes=args.minutes, evict=args.evict_testdata
        )

    _print_summary(worklist, args.dry_run, file=sys.stderr, timed_out=timed_out)
    # WA や TLE は判定であって失敗ではない。記録が出せたら 0 で返す。
    # ここを非ゼロにすると、CI の step が落ちて記録を取りこぼす。
    return 0


def _run_claiming(
    args: argparse.Namespace,
    envs: Sequence[env_mod.Environment],
    store: Store,
    out: Store,
) -> int:
    """CI のジョブ。問題を 1 つ宣言して、組の全環境で測る、を時間いっぱい繰り返す。

    並びは plan と同じ (組の全環境を合わせた重い順) で、ジョブ番号で開始点をずらす。
    宣言済みの問題と、記録から借りた cases_hash で「このモデルではどの環境も
    走らせるものが無い」と分かる問題は、宣言せずに飛ばす。宣言した問題は時間を
    過ぎても測り切る。コンパイラが入らなかった環境は飛ばして、残りで測る。
    """
    jobs = args.jobs if args.jobs is not None else 1
    if jobs < 1 or not 0 <= args.job < jobs:
        return _die(f"--job {args.job} が --jobs {jobs} に収まりません")
    groups = {env_mod.group_name(e) for e in envs}
    if len(groups) != 1:
        return _die(f"--env の環境が 1 つの組に収まりません: {sorted(groups)}")
    scope = groups.pop()

    # 入らなかったコンパイラの環境は飛ばす。別のコンパイラで代用すると、環境名が
    # 名乗るものと実際に測ったものがずれた記録が残る。
    machines: dict[str, run_mod.Machine] = {}
    for env in envs:
        try:
            machines[env.name] = run_mod.Machine.detect(env)
        except env_mod.EnvironmentError_ as e:
            print(f"warning: {env.name} のコンパイラが使えないので飛ばします: {e}", file=sys.stderr)
    usable = [e for e in envs if e.name in machines]
    if not usable:
        print("使えるコンパイラが無いので終わります", file=sys.stderr)
        return 0
    cpu_model = machines[usable[0].name].cpu_model
    for env in usable:
        print(f"{env.name} / {cpu_model} / {machines[env.name].compiler_version}", file=sys.stderr)

    all_envs = env_mod.load_all()
    problems = [problem_mod.load(d) for d in problem_mod.all_problem_dirs()]
    by_id = {p.id: p for p in problems}
    order = list(plan_mod.combined_order(plan_mod.for_envs(usable, problems, all_envs, store)))
    # 既知のモデルで全部計測済みの問題は plan の並びに無い。引いたモデルが新しければ
    # そこにも仕事があるので、うしろに付けておく。実際に走らせるかはこの先で決まる。
    seen = set(order)
    order += [p.id for p in problems if p.id not in seen]
    if args.problem:
        if args.problem not in by_id:
            return _die(f"問題 {args.problem!r} がありません")
        order = [args.problem]
    sequence = plan_mod.rotation(order, args.job, jobs)

    claims = claims_mod.Claims(
        args.claim_run, scope, cpu_model, job=args.job, remote=args.claim_remote
    )
    keys = store.keys()
    borrowed = store.cases_hashes()
    batches = store.batches()
    batch_id = batch_mod.ci_id(args.claim_run, scope, args.job)
    taken = claims.taken()
    unavailable = _without_testdata(problems)
    print(
        f"宣言済み {len(taken)} 問から始めます"
        + (f" (保管庫に無い manual の {len(unavailable)} 問は宣言しません)" if unavailable else ""),
        file=sys.stderr,
    )

    started = time.monotonic()
    claimed = executed = skipped = lost = nothing = already = 0
    timed_out = False
    for problem_id in sequence:
        if run_mod.out_of_time(started, args.minutes, time.monotonic()):
            timed_out = True
            break
        if problem_id in taken or problem_id in unavailable:
            already += problem_id in taken
            continue
        problem = by_id[problem_id]
        targets = [(problem, s) for s in problem.submissions()]
        # 借りた値で判定して、どの環境も走らせるものが無ければ宣言しない。
        # テストデータにも触らない。
        probes = [
            run_mod.build_worklist(
                targets, env, machines[env.name], keys, borrowed=borrowed, allow_fetch=False,
                batches=batches, batch_id=batch_id,
            )
            for env in usable
        ]
        if not any(probe.jobs or probe.pending for probe in probes):
            nothing += 1
            continue
        if not claims.claim(problem_id):
            lost += 1
            taken = claims.taken()
            continue
        claimed += 1
        print(f"宣言 {claimed}: {problem_id}", file=sys.stderr)
        for env in usable:
            worklist = run_mod.build_worklist(
                targets, env, machines[env.name], keys, borrowed=borrowed,
                allow_fetch=True, refresh=args.refresh,
                batches=batches, batch_id=batch_id,
            )
            _report(worklist)
            # テストデータは組の全環境で使い回すので、ここでは捨てない。
            done, _ = _execute(worklist.jobs, out, minutes=None, evict=False)
            executed += done
            skipped += len(worklist.skipped)
        if args.evict_testdata:
            fetch.evict(problem)

    parts = [
        f"宣言 {claimed} 問",
        f"実行 {executed} 件",
        f"スキップ {skipped} 件",
        f"宣言済みで飛ばした {already} 問",
        f"同時に取ろうとして負けた {lost} 問",
        f"このモデルでは仕事なし {nothing} 問",
    ]
    if timed_out:
        parts.append(f"時間の上限 {args.minutes:g} 分で終了")
    print(" / ".join(parts), file=sys.stderr)
    return 0


def _without_testdata(problems: Sequence[problem_mod.Problem]) -> set[str]:
    """保管庫に無い manual の問題。宣言しても取れないので、宣言せずに飛ばす。

    記録が無い問題は費用の見積もりが最悪値になって並びの先頭に来るので、放って
    おくと各モデルの最初のジョブが毎回宣言して取得に失敗する。保管庫を見られない
    (トークンが無い) ときは空で、今までどおり取りに行って失敗する。
    """
    if not mirror.available():
        return set()
    try:
        names = mirror.asset_names()
    except mirror.MirrorError as e:
        print(f"warning: 保管庫の一覧を取れません: {e}", file=sys.stderr)
        return set()
    return {
        p.id
        for p in problems
        if p.testdata.source == "manual" and mirror.asset_name(p) not in names
    }


def _report(worklist: run_mod.Worklist) -> None:
    for problem_id, before, after in worklist.moved:
        print(
            f"warning: {problem_id} のテストデータが変わりました "
            f"({before} -> {after})。この問題の記録は測り直しになります",
            file=sys.stderr,
        )
    for problem_id, reason in worklist.failed:
        print(
            f"warning: {problem_id} のテストデータを取れません: {reason}",
            file=sys.stderr,
        )
    for submission, target in worklist.unresolved:
        print(
            f"warning: {submission} の include {target!r} を解決できません",
            file=sys.stderr,
        )
    if worklist.blocked:
        print(
            f"warning: include を解決できない {len(worklist.blocked)} 件を飛ばしました。"
            "ライブラリを取れていない可能性があります",
            file=sys.stderr,
        )


def _execute(
    jobs: Sequence[run_mod.Job], out: Store, *, minutes: float | None, evict: bool
) -> tuple[int, int]:
    """並んだ順に走らせて記録を書く。(走らせた件数, 時間で見送った件数) を返す。"""
    started = time.monotonic()
    executed = 0
    for index, job in enumerate(jobs, start=1):
        if run_mod.out_of_time(started, minutes, time.monotonic()):
            # 時間の上限。残りは次の実行が同じ順で拾う。
            timed_out = len(jobs) - index + 1
            print(
                f"時間の上限 {minutes:g} 分を過ぎたので、残り {timed_out} 件は見送ります",
                file=sys.stderr,
            )
            return executed, timed_out
        print(
            f"[{index}/{len(jobs)}] {job.problem.id} / {job.submission.as_posix()}",
            file=sys.stderr,
        )
        try:
            record = run_mod.execute_job(job)
        except fetch.FetchError as e:
            # テストデータや判定器が用意できなかった。1 件のために残りを落とさず、
            # 飛ばして次へ。取りこぼしは次の実行が拾う。
            print(f"warning: {job.problem.id} / {job.submission.as_posix()} を飛ばします: {e}",
                  file=sys.stderr)
        else:
            out.append(record)
            print(record.to_json(), flush=True)
            executed += 1
        # 同じ問題の提出は続けて並ぶので、最後の 1 本を測ったらそのテストデータを
        # 捨てられる。ランナーの disk のためで、手元では渡さない。
        last_of_problem = index == len(jobs) or jobs[index].problem.id != job.problem.id
        if evict and last_of_problem:
            fetch.evict(job.problem)
    return executed, 0


def _print_summary(
    worklist: run_mod.Worklist, dry_run: bool, *, file, timed_out: int = 0
) -> None:
    verb = "実行予定" if dry_run else "実行"
    parts = [f"{verb} {len(worklist.jobs) - timed_out} 件", f"スキップ {len(worklist.skipped)} 件"]
    if timed_out:
        parts.append(f"時間で見送り {timed_out} 件")
    if worklist.pending:
        parts.append(f"テストデータ未取得 {len(worklist.pending)} 件")
    if worklist.blocked:
        parts.append(f"include 未解決 {len(worklist.blocked)} 件")
    print(" / ".join(parts), file=file)


def cmd_claims_clean(args: argparse.Namespace) -> int:
    """この run 以前の宣言の ref を消す。collect の最後に呼ぶ。"""
    removed = claims_mod.clean(args.run_id, remote=args.remote)
    print(f"宣言を {removed} 本消しました", file=sys.stderr)
    return 0


def cmd_claims_list(args: argparse.Namespace) -> int:
    for ref in claims_mod.list_refs(remote=args.remote):
        print(ref)
    return 0


def cmd_records_append(args: argparse.Namespace) -> int:
    store = Store(Path(args.store) if args.store else RESULTS_DIR)
    added, skipped = store.absorb(Path(d) for d in args.dirs)
    print(f"追加 {added} 件 / 既にあった {skipped} 件", file=sys.stderr)
    return 0


def cmd_site_build(args: argparse.Namespace) -> int:
    store = Store(Path(args.store) if args.store else RESULTS_DIR)
    out = Path(args.out) if args.out else SITE_DIR
    # 古い記録かどうかは include 閉包を作り直して見る。lib/ が無いと
    # ライブラリを使う提出の閉包が欠けて、判定できないまま現行として出る。
    if not LIB_DIR.is_dir():
        print(
            f"warning: {LIB_DIR.name}/ がありません。"
            "ライブラリを使う提出は古いかどうかを判定しません",
            file=sys.stderr,
        )
    summary = site_build.build(store, out)
    parts = [
        f"問題 {summary.problems} 件",
        f"記録 {summary.records} 件",
        f"ページ {summary.pages + 1} 枚",
        f"提出ページ {summary.submission_pages} 枚",
        f"ヘッダ {summary.headers} 件",
        # verify を畳むゲート。ヘッダの総数に揃った回で Library の verify を止められる。
        f"全環境に現行 AC が揃ったヘッダ {summary.verified_headers} / {summary.headers}",
    ]
    if summary.stale:
        parts.append(f"参考 {summary.stale} 件")
    print(f"{summary.out} に " + " / ".join(parts), file=sys.stderr)
    return 0


def cmd_repro(args: argparse.Namespace) -> int:
    """1 提出を手元で走らせて、落ちたケースの差分とファイルの場所を出す。"""
    try:
        problem = problem_mod.load_by_id(args.problem)
        env = env_mod.load(args.env)
    except (problem_mod.ProblemError, env_mod.EnvironmentError_) as e:
        return _die(str(e))
    submission = Path(args.submission)
    if not (problem.dir / submission).is_file():
        return _die(f"{problem.id} に提出 {submission} がありません")
    try:
        return repro_mod.repro(problem, submission, env, case=args.case)
    except (repro_mod.ReproError, fetch.FetchError) as e:
        return _die(str(e))


def cmd_records_list(args: argparse.Namespace) -> int:
    store = Store(Path(args.store) if args.store else RESULTS_DIR)
    for problem_id in store.problem_ids():
        records = list(store.read(problem_id))
        print(f"{problem_id}\t{len(records)} records")
        for record in records:
            print(
                f"  {record['status']:3} {record['submission']}\t"
                f"{record['env']}\t{record['cpu_model']}\t{record['key'][:16]}"
            )
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="pj", description="procon-judge")
    sub = parser.add_subparsers(dest="command", required=True)

    problems = sub.add_parser("problems", help="問題").add_subparsers(
        dest="subcommand", required=True
    )
    p_list = problems.add_parser("list", help="問題の一覧")
    p_list.add_argument("--json", action="store_true")
    p_list.set_defaults(func=cmd_problems_list)
    p_check = problems.add_parser("check", help="problem.toml の検証")
    p_check.add_argument("--problem")
    p_check.set_defaults(func=cmd_problems_check)
    p_titles = problems.add_parser("titles", help="題名を判定サイトの名前と突き合わせる")
    p_titles.add_argument("--problem")
    p_titles.add_argument("--fix", action="store_true", help="problem.toml を書き換える")
    p_titles.set_defaults(func=cmd_problems_titles)
    p_import = problems.add_parser(
        "import", help="competitive-verifier のテストを raw の問題として取り込む"
    )
    p_import.add_argument("paths", nargs="+", metavar="PATH", help="*.test.cpp かそのディレクトリ")
    p_import.add_argument(
        "--single-only", action="store_true", help="実装が 1 本の問題だけを取り込む"
    )
    p_import.add_argument("--dry-run", action="store_true", help="計画を出すだけ")
    p_import.set_defaults(func=cmd_problems_import)

    submissions = sub.add_parser("submissions", help="提出").add_subparsers(
        dest="subcommand", required=True
    )
    s_list = submissions.add_parser("list", help="提出の一覧")
    s_list.add_argument("--problem")
    s_list.set_defaults(func=cmd_submissions_list)

    p_fetch = sub.add_parser("fetch", help="テストデータ取得")
    p_fetch.add_argument("--problem")
    p_fetch.add_argument("--all", action="store_true", help="全問題")
    p_fetch.add_argument("--refresh", action="store_true", help="キャッシュを無視する")
    p_fetch.add_argument(
        "--from-origin",
        action="store_true",
        help="保管庫も飛ばして原本から取る。保管庫の中身を疑うとき用",
    )
    p_fetch.add_argument(
        "--env", default="local", help="local の参照実装を組む環境 (既定 local)"
    )
    p_fetch.set_defaults(func=cmd_fetch)

    testdata = sub.add_parser("testdata", help="テストデータ").add_subparsers(
        dest="subcommand", required=True
    )
    t_import = testdata.add_parser("import", help="手元で落としたものを取り込む")
    t_import.add_argument("--problem", required=True)
    t_import.add_argument("--dir", required=True, metavar="PATH")
    t_import.set_defaults(func=cmd_testdata_import)

    mirror_cmd = sub.add_parser("mirror", help="テストデータの保管庫").add_subparsers(
        dest="subcommand", required=True
    )
    m_push = mirror_cmd.add_parser("push", help="保管庫へ上げる")
    m_push.add_argument("--problem", required=True)
    m_push.add_argument(
        "--force",
        action="store_true",
        help="既にあっても置き換える。取り直したデータに入れ替えるとき用",
    )
    m_push.set_defaults(func=cmd_mirror_push)
    m_pull = mirror_cmd.add_parser("pull", help="保管庫から落とす")
    m_pull.add_argument("--problem", required=True)
    m_pull.set_defaults(func=cmd_mirror_pull)
    m_status = mirror_cmd.add_parser("status", help="問題ごとの保管状況")
    m_status.add_argument("--problem")
    m_status.set_defaults(func=cmd_mirror_status)

    p_plan = sub.add_parser("plan", help="環境ごとのジョブの本数を決める")
    p_plan.add_argument("--store", help=f"記録を読む場所 (既定 {RESULTS_DIR.name}/)")
    p_plan.add_argument(
        "--minutes",
        type=int,
        default=plan_mod.DEFAULT_MINUTES,
        help=f"1 ジョブの実行時間の上限 (分、既定 {plan_mod.DEFAULT_MINUTES})",
    )
    p_plan.add_argument(
        "--github-output", help="matrix と any を書き足すファイル ($GITHUB_OUTPUT)"
    )
    p_plan.set_defaults(func=cmd_plan)

    p_run = sub.add_parser("run", help="実行して記録を出す")
    p_run.add_argument(
        "--env", required=True,
        help="環境名。--claim-run のときは組の全環境をコンマ区切りで (x64-gcc,x64-clang)",
    )
    p_run.add_argument("--problem", help="省略すると全問題")
    p_run.add_argument("--submission", help="省略すると問題の全提出")
    p_run.add_argument(
        "--dry-run", action="store_true", help="走らせる対象を出すだけ"
    )
    p_run.add_argument("--store", help=f"記録を読む場所 (既定 {RESULTS_DIR.name}/)")
    p_run.add_argument("--out", help="記録の書き先 (既定は --store と同じ)")
    p_run.add_argument("--refresh", action="store_true", help="テストデータを取り直す")
    p_run.add_argument(
        "--claim-run",
        metavar="RUN_ID",
        help="CI のジョブとして、問題を 1 つずつ宣言して測る。GitHub の run id を渡す",
    )
    p_run.add_argument(
        "--claim-remote", default="origin", help="宣言を置く remote (既定 origin)。手元で試すとき用"
    )
    p_run.add_argument(
        "--job", type=int, default=0, help="何番目のジョブか (0 始まり)。宣言の開始点をずらす"
    )
    p_run.add_argument(
        "--jobs", type=int, help="その環境で立っているジョブの本数 (--claim-run のとき)"
    )
    p_run.add_argument(
        "--minutes",
        type=float,
        help="実行時間の上限 (分)。過ぎたら次の提出に手を付けない。省略すると打ち切らない",
    )
    p_run.add_argument(
        "--evict-testdata",
        action="store_true",
        help="測り終えた問題のテストデータを手元のキャッシュから消す (CI のランナー用)",
    )
    p_run.set_defaults(func=cmd_run)

    p_repro = sub.add_parser(
        "repro", help="1 提出を手元で走らせて、落ちたケースの差分を見る"
    )
    p_repro.add_argument("--problem", required=True)
    p_repro.add_argument("--submission", required=True, help="submissions/xxx.hpp の形")
    p_repro.add_argument("--case", help="このケースだけ走らせる。省略すると全部")
    p_repro.add_argument("--env", default="local")
    p_repro.set_defaults(func=cmd_repro)

    claims_cmd = sub.add_parser("claims", help="CI のジョブが仕事を取るための宣言").add_subparsers(
        dest="subcommand", required=True
    )
    c_clean = claims_cmd.add_parser("clean", help="この run 以前の宣言を消す")
    c_clean.add_argument("--run-id", required=True)
    c_clean.add_argument("--remote", default="origin")
    c_clean.set_defaults(func=cmd_claims_clean)
    c_list = claims_cmd.add_parser("list", help="今ある宣言の ref を出す")
    c_list.add_argument("--remote", default="origin")
    c_list.set_defaults(func=cmd_claims_list)

    records = sub.add_parser("records", help="記録").add_subparsers(
        dest="subcommand", required=True
    )
    r_append = records.add_parser("append", help="ほかの jsonl を取り込む")
    r_append.add_argument("dirs", nargs="+", metavar="DIR")
    r_append.add_argument("--store")
    r_append.set_defaults(func=cmd_records_append)

    r_list = records.add_parser("list", help="記録の一覧")
    r_list.add_argument("--store")
    r_list.set_defaults(func=cmd_records_list)

    site = sub.add_parser("site", help="サイト").add_subparsers(
        dest="subcommand", required=True
    )
    site_b = site.add_parser("build", help="記録から静的なサイトを作る")
    site_b.add_argument("--out", help=f"書き先 (既定 {SITE_DIR.name}/)")
    site_b.add_argument("--store", help=f"記録を読む場所 (既定 {RESULTS_DIR.name}/)")
    site_b.set_defaults(func=cmd_site_build)

    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        return args.func(args)
    except (
        problem_mod.ProblemError,
        env_mod.EnvironmentError_,
        fetch.FetchError,
        mirror.MirrorError,
        migrate_mod.MigrateError,
        site_build.SiteError,
        claims_mod.ClaimError,
    ) as e:
        return _die(str(e))


if __name__ == "__main__":
    sys.exit(main())
