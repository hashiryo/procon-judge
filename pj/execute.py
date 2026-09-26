"""実行と計測。

プロセスは posix_spawn で起こして wait4 で回収する。ピーク RSS は、Linux では
差し込んだ共有ライブラリ (rss_preload.c) が報告する VmHWM を採る。
"""

from __future__ import annotations

import functools
import hashlib
import json
import os
import platform
import resource
import signal
import subprocess
import sys
import tempfile
import threading
import time
from dataclasses import dataclass, field
from pathlib import Path

from .paths import BUILD_CACHE_DIR

METRICS_PREFIX = "PJ_METRICS "

# 空打ちに与える時間。exec さえ済めばよいので短くてよい。
WARMUP_TIMEOUT_SEC = 2.0

# 子に LD_PRELOAD で差し込む共有ライブラリのソース。終了時に VmHWM を報告する。
RSS_PRELOAD_SOURCE = Path(__file__).with_name("rss_preload.c")


@dataclass(frozen=True)
class RunResult:
    exit_code: int | None
    term_signal: int | None
    timed_out: bool
    wall_ns: int
    max_rss_kb: int
    metrics: dict = field(default_factory=dict)

    @property
    def wall_ms(self) -> int:
        return self.wall_ns // 1_000_000

    @property
    def crashed(self) -> bool:
        return self.term_signal is not None or (self.exit_code or 0) != 0

    @property
    def algo_time_ns(self) -> int | None:
        value = self.metrics.get("algo_time_ns")
        return int(value) if value is not None else None


def _rss_to_kb(ru_maxrss: int) -> int:
    """ru_maxrss の単位はプラットフォームで違う。Linux は KB、macOS はバイト。"""
    if platform.system() == "Darwin":
        return ru_maxrss // 1024
    return ru_maxrss


def peak_rss_kb(metrics: dict, ru_maxrss: int, parent_peak_kb: int | None = None) -> int:
    """ピーク RSS。子が報告していればそちらを採る。分からなければ 0。

    posix_spawn した子の ru_maxrss は当てにならない。カーネルが exec のときに
    古い mm の high-water を引き継ぐので、Linux では max(pj 自身のピーク, 子のピーク)
    になる。ハーネスを持たない raw の提出も含めて、Linux では差し込んだ共有ライブラリ
    (rss_preload.c) が /proc/self/status の VmHWM を報告する。ハーネスも同じ値を報告する。

    異常終了した実行では報告が出ないので ru_maxrss に落ちる。parent_peak_kb (spawn する
    直前の pj 自身のピーク) 以下なら子の値ではないので、分からないとして 0 を返す。
    0 は MLE の判定に引っかからず、サイトでは - になる。macOS の posix_spawn は
    アドレス空間を共有しないので ru_maxrss がそのまま使え、parent_peak_kb は渡さない。
    """
    reported = metrics.get("max_rss_kb")
    if isinstance(reported, int) and not isinstance(reported, bool) and reported > 0:
        return reported
    kb = _rss_to_kb(ru_maxrss)
    if parent_peak_kb is not None and kb <= parent_peak_kb:
        return 0
    return kb


@functools.cache
def rss_preload() -> str | None:
    """子に LD_PRELOAD で差し込む共有ライブラリのパス。Linux 以外と、組めないときは None。

    初回にソースを cc で組んで、ビルドのキャッシュに置く。名前にソースのハッシュと
    CPU の種類を入れるので、ソースを書き換えれば組み直す。並んで走る pj どうしが
    同じファイルを書かないよう、一時ファイルに組んでから置き換える。
    """
    if platform.system() != "Linux":
        return None
    digest = hashlib.sha256(RSS_PRELOAD_SOURCE.read_bytes()).hexdigest()[:16]
    out = BUILD_CACHE_DIR / f"rss_preload-{digest}-{platform.machine()}.so"
    if out.is_file():
        return str(out)
    out.parent.mkdir(parents=True, exist_ok=True)
    tmp = out.with_name(f"{out.name}.{os.getpid()}.tmp")
    cmd = ["cc", "-shared", "-fPIC", "-O2", "-o", str(tmp), str(RSS_PRELOAD_SOURCE)]
    try:
        subprocess.run(cmd, check=True, capture_output=True, text=True)
    except (OSError, subprocess.CalledProcessError) as e:
        detail = getattr(e, "stderr", "") or str(e)
        print(
            f"warning: {RSS_PRELOAD_SOURCE.name} を組めませんでした。"
            f"ハーネスを持たない提出のメモリは分からなくなります: {detail.strip()}",
            file=sys.stderr,
        )
        return None
    os.replace(tmp, out)
    return str(out)


def _child_env() -> dict[str, str] | os._Environ:
    """子の環境変数。Linux では LD_PRELOAD の先頭に rss_preload を足す。"""
    preload = rss_preload()
    if preload is None:
        return os.environ
    env = dict(os.environ)
    rest = env.get("LD_PRELOAD")
    env["LD_PRELOAD"] = f"{preload}:{rest}" if rest else preload
    return env


def parse_metrics(stderr_path: Path) -> dict:
    """stderr から PJ_METRICS の行を拾う。他の出力が混ざっていても平気。"""
    metrics: dict = {}
    try:
        with stderr_path.open("r", errors="replace") as f:
            for line in f:
                if not line.startswith(METRICS_PREFIX):
                    continue
                try:
                    payload = json.loads(line[len(METRICS_PREFIX):])
                except json.JSONDecodeError:
                    continue
                if isinstance(payload, dict):
                    metrics.update(payload)
    except OSError:
        pass
    return metrics


def raise_stack_limit() -> None:
    """スタックの上限を硬い方まで上げる。

    既定の 8 MB だと、再帰で木を辿る実装が深さ数十万で落ちる。判定サイトは
    どこもスタックを縛らないので、そこに合わせる。縛ったままだと、問題とは
    関係のない理由で再帰の実装だけが RE になり、比較にならない。

    posix_spawn には exec 前に差し込む口が無いので、自分の上限を上げて子へ
    継承させる。親のスタックは既に張られているので、上げても影響しない。
    Linux では無制限、macOS では 64 MB 前後が硬い上限になる。

    使うメモリが増えるわけではないので、mle_mb の後判定は変わらない。
    """
    soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
    if soft == hard:
        return
    try:
        resource.setrlimit(resource.RLIMIT_STACK, (hard, hard))
    except (ValueError, OSError):
        # 上げられない環境では既定のままで走らせる。
        pass


def run(
    binary: Path,
    *,
    stdin_path: Path | None,
    stdout_path: Path,
    stderr_path: Path,
    tle_sec: float,
) -> RunResult:
    """1 ケース走らせる。tle_sec を超えたら kill する。

    メモリは後判定にできるが時間はできない。無限ループの提出があると
    ジョブが埋まるので、その場で打ち切る。
    """
    raise_stack_limit()
    stdout_path.parent.mkdir(parents=True, exist_ok=True)
    create = os.O_WRONLY | os.O_CREAT | os.O_TRUNC
    file_actions = [
        (os.POSIX_SPAWN_OPEN, 0, str(stdin_path or os.devnull), os.O_RDONLY, 0o644),
        (os.POSIX_SPAWN_OPEN, 1, str(stdout_path), create, 0o644),
        (os.POSIX_SPAWN_OPEN, 2, str(stderr_path), create, 0o644),
    ]

    # posix_spawn は PATH を見ないので絶対パスで渡す。
    exe = str(binary.resolve())
    env = _child_env()
    # 子の ru_maxrss に乗る pj 自身のピーク。報告が無いときの見分けに使う (peak_rss_kb)。
    parent_peak_kb = (
        _rss_to_kb(resource.getrusage(resource.RUSAGE_SELF).ru_maxrss)
        if platform.system() == "Linux"
        else None
    )
    t0 = time.monotonic_ns()
    pid = os.posix_spawn(exe, [exe], env, file_actions=file_actions)

    lock = threading.Lock()
    state = {"reaped": False, "timed_out": False}

    def on_timeout() -> None:
        with lock:
            if state["reaped"]:
                return
            state["timed_out"] = True
            try:
                os.kill(pid, signal.SIGKILL)
            except ProcessLookupError:
                # ちょうど終わったところ。回収は待っている側がやる。
                pass

    timer = threading.Timer(tle_sec, on_timeout)
    timer.daemon = True
    timer.start()
    try:
        _, status, rusage = os.wait4(pid, 0)
    finally:
        with lock:
            state["reaped"] = True
        timer.cancel()
    wall_ns = time.monotonic_ns() - t0

    if os.WIFSIGNALED(status):
        term_signal: int | None = os.WTERMSIG(status)
        exit_code: int | None = None
    else:
        term_signal = None
        exit_code = os.WEXITSTATUS(status)

    metrics = parse_metrics(stderr_path)
    return RunResult(
        exit_code=exit_code,
        term_signal=term_signal,
        timed_out=state["timed_out"],
        wall_ns=wall_ns,
        max_rss_kb=peak_rss_kb(metrics, rusage.ru_maxrss, parent_peak_kb),
        metrics=metrics,
    )


def warmup(binary: Path, *, tle_sec: float) -> None:
    """計測の前にバイナリを 1 回空打ちする。

    macOS では新しくコンパイルしたバイナリの初回 exec に 0.3 秒ほどかかる
    (署名の検証)。そのぶんが最初のケースの実時間に乗ると time_max_ms が狂う。
    入力を与えないので提出はすぐ終わる。終わらなくても exec は済んでいるので
    打ち切ってよい。
    """
    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        try:
            run(
                binary,
                stdin_path=None,
                stdout_path=tmp_path / "stdout",
                stderr_path=tmp_path / "stderr",
                tle_sec=min(tle_sec, WARMUP_TIMEOUT_SEC),
            )
        except OSError:
            pass
