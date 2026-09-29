"""コンパイラを子として走らせ、終わるまでの時間とメモリの山を測る小さな親。

build.py が 1 段はさんで起動する。pj が直接コンパイラを spawn すると、Linux では子の
ru_maxrss に pj 自身のピーク RSS が乗る (カーネルが exec のときに古い mm の high-water を
引き継ぐため。execute.py の peak_rss_kb)。この親は exec し直した小さなプロセスなので、
ここから起こしたコンパイラの ru_maxrss には、この親の小さな値しか乗らない。wait4 の
ru_maxrss は、コンパイラとその子 (cc1plus、lto1、リンカ) のうち、いちばん大きいものの値。

打ち切りもここでする。コンパイラを別のプロセスグループで起こし、時間を過ぎたらグループ
ごと止める。ドライバだけを止めると cc1plus が残って走り続ける。

pj のパッケージは import しない。単体のスクリプトとして走らせる。

使い方: python compile_meter.py <結果を書く JSON> <打ち切りの秒数> <コマンド...>
コンパイラの標準出力と標準エラーはそのまま受け継ぐ。終了コードはコンパイラのもので、
打ち切ったときは 124。
"""

from __future__ import annotations

import json
import os
import platform
import signal
import subprocess
import sys
import time

TIMED_OUT_EXIT = 124


def _write(path: str, report: dict) -> None:
    with open(path, "w", encoding="utf-8") as f:
        json.dump(report, f)


def _rss_to_kb(ru_maxrss: int) -> int:
    # 単位は Linux が KB、macOS がバイト (execute.py の _rss_to_kb と同じ)。
    return ru_maxrss // 1024 if platform.system() == "Darwin" else ru_maxrss


def main(argv: list[str]) -> int:
    out_path, timeout, *cmd = argv
    t0 = time.monotonic()
    try:
        proc = subprocess.Popen(cmd, start_new_session=True)
    except OSError as e:
        _write(out_path, {"error": str(e)})
        return 127

    state = {"timed_out": False, "interrupted": 0}

    def stop_group() -> bool:
        try:
            os.killpg(proc.pid, signal.SIGKILL)
        except ProcessLookupError:
            # ちょうど終わったところ。回収は下の wait4 がする。
            return False
        return True

    def on_alarm(signum, frame) -> None:
        if stop_group():
            state["timed_out"] = True

    def on_interrupt(signum, frame) -> None:
        # コンパイラは別のグループにいるので、端末の Ctrl-C は届かない。代わりに止める。
        state["interrupted"] = signum
        stop_group()

    signal.signal(signal.SIGALRM, on_alarm)
    signal.signal(signal.SIGINT, on_interrupt)
    signal.signal(signal.SIGTERM, on_interrupt)
    signal.setitimer(signal.ITIMER_REAL, float(timeout))
    # Popen.wait は rusage を返さないので wait4 で回収する。
    _, status, rusage = os.wait4(proc.pid, 0)
    signal.setitimer(signal.ITIMER_REAL, 0)
    seconds = time.monotonic() - t0

    code = os.waitstatus_to_exitcode(status)
    _write(
        out_path,
        {
            "seconds": seconds,
            "rss_kb": _rss_to_kb(rusage.ru_maxrss),
            "timed_out": state["timed_out"],
            "returncode": code,
        },
    )
    if state["interrupted"]:
        return 128 + state["interrupted"]
    if state["timed_out"]:
        return TIMED_OUT_EXIT
    # シグナルで落ちたときはシェルと同じく 128 + シグナル番号にする。
    return code if code >= 0 else 128 - code


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
