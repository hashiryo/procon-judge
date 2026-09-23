"""記録が今のソースで測ったものかどうかを見る。

キーには cases_hash と compiler_version と CPU モデルが要る。サイトを作る
ジョブはテストデータを持たないし、測ったマシンでもないので、キーを一から
作り直すことはできない。ただ知りたいのは「測ってからソース側が変わったか」
だけなので、機械側の値はその記録から借りる。借りた値と今のソースでキーを
組み直して、記録のキーと突き合わせる。

キーの計算は pj.key をそのまま呼ぶ。同じ判定を二度書くと、片方を直し忘れた
ときに記録の意味が静かにずれる。

cases_hash も記録から借りているので、テストデータだけが上流で作り直された
場合は見逃す。それは次に run が走ったときに測り直されて入れ替わるので、
表示が遅れるだけで済む。

参考に落ちた理由も出せる。記録は閉包のファイルごとのハッシュを持っているので、
今の閉包と突き合わせれば、どのファイルが動いたかが分かる。ファイル以外の成分
(problem.toml とコンパイルフラグ) は、それぞれのハッシュと文字列を比べる。
"""

from __future__ import annotations

from collections.abc import Sequence
from dataclasses import dataclass

from . import build as build_mod
from . import key as key_mod
from .environment import Environment
from .problem import Problem


@dataclass(frozen=True)
class Diff:
    """記録と今のソースの、キーに効く違い。参考に落ちた理由。

    どれも空なら、記録に材料が無くて理由を言えない (古い記録)。
    """

    # 中身が変わったファイル。ラベルは記録の includes と同じ形。
    changed: tuple[str, ...] = ()
    # 今の閉包にあって、記録の閉包に無かったファイル。
    added: tuple[str, ...] = ()
    # 記録の閉包にあって、今の閉包に無いファイル。
    removed: tuple[str, ...] = ()
    # ファイル以外の成分。"problem" (problem.toml か local のジェネレータ) と
    # "cxxflags" (environments.toml) のどちらか、または両方。
    settings: tuple[str, ...] = ()

    @property
    def known(self) -> bool:
        return bool(self.changed or self.added or self.removed or self.settings)


class Freshness:
    """1 問題ぶんの判定。ソース側のハッシュは問題につき一度だけ作る。"""

    def __init__(self, problem: Problem, envs: Sequence[Environment]):
        self.problem = problem
        self._search = build_mod.include_dirs(problem)
        self._harness = key_mod.harness_key(problem, self._search)
        self._problem_hash = key_mod.problem_hash(problem)
        # 実際のフラグ (記録の cxxflags と比べて「変わったか」を言うのに使う) と、
        # キーの材料 (問題のディレクトリの -I を印に置き換えたもの) を分けて持つ。
        self._cxxflags = {
            env.name: build_mod.effective_cxxflags(env, problem) for env in envs
        }
        self._key_cxxflags = {
            env.name: build_mod.key_cxxflags(env, problem) for env in envs
        }
        self._subs: dict[str, key_mod.SubmissionKey | None] = {}

    def _submission(self, path: str) -> key_mod.SubmissionKey | None:
        if path not in self._subs:
            source = self.problem.dir / path
            self._subs[path] = (
                key_mod.submission_hash(source, self._search)
                if path and source.is_file()
                else None
            )
        return self._subs[path]

    def key_for(
        self,
        submission: str,
        *,
        cases_hash: str,
        env: str,
        compiler_version: str,
        cpu_model: str,
    ) -> str | None:
        """今のソースでこの条件を測ったら付くはずのキー。

        作れないときは None を返す。提出のファイルが消えている、閉包が欠けて
        いる (lib/ を取っていない)、環境の定義が消えている、のどれか。

        機械側の 4 つは呼ぶ側が持ってくる。記録から借りれば「測ってからソース
        側が変わったか」が見られるし、既知のモデルの値を入れれば「そのモデル
        で測ったらどのキーになるか」が分かる。plan が後者を使う。
        """
        sub = self._submission(submission)
        if sub is None or sub.unresolved:
            return None
        cxxflags = self._key_cxxflags.get(env)
        if cxxflags is None:
            return None
        return key_mod.compute(
            submission=submission,
            submission_hash=sub.submission_hash,
            harness_hash=self._harness.harness_hash,
            problem_hash=self._problem_hash,
            cases_hash=cases_hash,
            env=env,
            compiler_version=compiler_version,
            cxxflags=cxxflags,
            cpu_model=cpu_model,
        )

    def current(self, record: dict) -> bool | None:
        """今のソースで測った記録なら True、ソースが変わっていれば False。

        判定できないときは None を返す。分からないものを古い側に倒すと、
        ライブラリを取れなかった回に表が丸ごと参考になってしまう。
        """
        expected = self.key_for(
            record.get("submission", ""),
            cases_hash=record.get("cases_hash", ""),
            env=record.get("env", ""),
            compiler_version=record.get("compiler_version", ""),
            cpu_model=record.get("cpu_model", ""),
        )
        return None if expected is None else record.get("key") == expected

    def diff(self, record: dict) -> Diff | None:
        """参考に落ちた理由。現行の記録と、判定できない記録には None。

        機械側の値は記録から借りているので、違いうる成分は提出とハーネスの
        閉包、problem.toml、コンパイルフラグの 4 つに限られる。閉包はファイル
        ごとに突き合わせて名指しする。file_hashes を持たない古い記録では
        ファイルを名指しできないので、known が False の Diff になる。
        """
        if self.current(record) is not False:
            return None
        sub = self._submission(record.get("submission", ""))
        assert sub is not None  # current() が False を返した以上、判定できている

        settings = []
        if record.get("cxxflags") != self._cxxflags.get(record.get("env", "")):
            settings.append("cxxflags")
        recorded_problem = record.get("problem_hash")
        if recorded_problem and recorded_problem != self._problem_hash:
            settings.append("problem")

        recorded = record.get("file_hashes")
        if not isinstance(recorded, dict):
            return Diff(settings=tuple(settings))
        now = dict(sub.file_hashes + self._harness.file_hashes)
        return Diff(
            changed=tuple(
                sorted(l for l, h in recorded.items() if l in now and now[l] != h)
            ),
            added=tuple(sorted(l for l in now if l not in recorded)),
            removed=tuple(sorted(l for l in recorded if l not in now)),
            settings=tuple(settings),
        )
