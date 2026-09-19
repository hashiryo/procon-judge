"""コマンドの入口。

手元と CI が同じコマンドを使う。CI 前提の分岐はここに入れない。
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from . import environment as env_mod
from . import fetch
from . import problem as problem_mod
from . import run as run_mod
from .paths import RESULTS_DIR
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
    for directory in directories:
        try:
            p = problem_mod.load(directory)
        except problem_mod.ProblemError as e:
            print(f"NG  {directory.name}: {e}")
            failed += 1
            continue
        if not p.submissions():
            print(f"NG  {p.id}: submissions/ に提出がありません")
            failed += 1
            continue
        print(f"OK  {p.id}  ({len(p.submissions())} submissions)")
    if failed:
        print(f"\n{failed} 件が通りませんでした", file=sys.stderr)
    return 1 if failed else 0


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


def cmd_fetch(args: argparse.Namespace) -> int:
    p = problem_mod.load_by_id(args.problem)
    testcases = fetch.ensure(p, refresh=args.refresh)
    print(
        f"{p.id}: {testcases.count} cases  cases_hash={testcases.cases_hash}  "
        f"{testcases.dir}"
    )
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


def cmd_run(args: argparse.Namespace) -> int:
    env = env_mod.load(args.env)
    machine = run_mod.Machine.detect(env)
    store = Store(Path(args.store) if args.store else RESULTS_DIR)
    targets = _targets(args)

    print(
        f"{env.name} / {machine.cpu_model} / {machine.compiler_version}",
        file=sys.stderr,
    )
    plan = run_mod.build_plan(
        targets, env, machine, store.keys(),
        allow_fetch=not args.dry_run, refresh=args.refresh,
    )
    for submission, target in plan.unresolved:
        print(
            f"warning: {submission} の include {target!r} を解決できません",
            file=sys.stderr,
        )

    if args.dry_run:
        for job in plan.jobs:
            print(f"run \t{job.label}\t{job.key}")
        for job in plan.skipped:
            print(f"skip\t{job.label}\t{job.key}")
        for problem, submission in plan.pending:
            print(f"fetch\t{problem.id}\t{submission.as_posix()}\t-")
    else:
        for index, job in enumerate(plan.jobs, start=1):
            print(
                f"[{index}/{len(plan.jobs)}] {job.problem.id} / "
                f"{job.submission.as_posix()}",
                file=sys.stderr,
            )
            record = run_mod.execute_job(job)
            store.append(record)
            print(record.to_json(), flush=True)

    _print_summary(plan, args.dry_run, file=sys.stderr)
    # WA や TLE は判定であって失敗ではない。記録が出せたら 0 で返す。
    # ここを非ゼロにすると、CI の step が落ちて記録を取りこぼす。
    return 0


def _print_summary(plan: run_mod.Plan, dry_run: bool, *, file) -> None:
    verb = "実行予定" if dry_run else "実行"
    parts = [f"{verb} {len(plan.jobs)} 件", f"スキップ {len(plan.skipped)} 件"]
    if plan.pending:
        parts.append(f"テストデータ未取得 {len(plan.pending)} 件")
    print(" / ".join(parts), file=file)


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

    submissions = sub.add_parser("submissions", help="提出").add_subparsers(
        dest="subcommand", required=True
    )
    s_list = submissions.add_parser("list", help="提出の一覧")
    s_list.add_argument("--problem")
    s_list.set_defaults(func=cmd_submissions_list)

    p_fetch = sub.add_parser("fetch", help="テストデータ取得")
    p_fetch.add_argument("--problem", required=True)
    p_fetch.add_argument("--refresh", action="store_true", help="キャッシュを無視する")
    p_fetch.set_defaults(func=cmd_fetch)

    p_run = sub.add_parser("run", help="実行して記録を出す")
    p_run.add_argument("--env", required=True)
    p_run.add_argument("--problem", help="省略すると全問題")
    p_run.add_argument("--submission", help="省略すると問題の全提出")
    p_run.add_argument(
        "--dry-run", action="store_true", help="走らせる対象を出すだけ"
    )
    p_run.add_argument("--store", help=f"記録の置き場 (既定 {RESULTS_DIR.name}/)")
    p_run.add_argument("--refresh", action="store_true", help="テストデータを取り直す")
    p_run.set_defaults(func=cmd_run)

    records = sub.add_parser("records", help="記録").add_subparsers(
        dest="subcommand", required=True
    )
    r_list = records.add_parser("list", help="記録の一覧")
    r_list.add_argument("--store")
    r_list.set_defaults(func=cmd_records_list)

    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        return args.func(args)
    except (
        problem_mod.ProblemError,
        env_mod.EnvironmentError_,
        fetch.FetchError,
    ) as e:
        return _die(str(e))


if __name__ == "__main__":
    sys.exit(main())
