"""source = "local"。ジェネレータと参照実装からテストデータを作る。"""

import json

import pytest

from pj import environment as env_mod
from pj import fetch

TOML = """
id = "{id}"
title = "t"

[harness]
kind = "raw"

[testdata]
source = "local"
count = {count}
generator = "gen.py"
reference = "reference.cpp"

[compare]
kind = "tokens"
"""

GEN = """import sys
seed = int(sys.argv[1])
print(seed + 1)
print(*range(seed + 1))
"""

REFERENCE = """#include <cstdio>
int main() {
  int n; if (scanf("%d", &n) != 1) return 1;
  long long s = 0;
  for (int i = 0; i < n; i++) { int x; if (scanf("%d", &x) != 1) return 1; s += x; }
  printf("%lld\\n", s * 2);
}
"""


@pytest.fixture
def isolated_cache(tmp_path, monkeypatch):
    monkeypatch.setattr(fetch, "TESTCASE_CACHE_DIR", tmp_path / "cache")
    from pj.fetch import mirror

    monkeypatch.setattr(mirror, "available", lambda: False)
    return tmp_path / "cache"


def make(tmp_path, name="tmp-local", count=3, gen=GEN, reference=REFERENCE):
    from pj import problem as problem_mod

    directory = tmp_path / name
    directory.mkdir()
    (directory / "problem.toml").write_text(TOML.format(id=name, count=count))
    (directory / "gen.py").write_text(gen)
    (directory / "reference.cpp").write_text(reference)
    (directory / "submissions").mkdir()
    (directory / "submissions" / "sol.cpp").write_text(reference)
    return problem_mod.load(directory)


def test_local_generates_count_cases_with_the_reference(tmp_path, isolated_cache):
    problem = make(tmp_path)
    testcases = fetch.ensure(problem, env=env_mod.load("local"))
    assert testcases.count == 3
    assert [c.name for c in testcases.cases] == ["seed_000", "seed_001", "seed_002"]
    # seed 2 の入力は 0 1 2 で、参照実装は和の 2 倍を返す。
    assert testcases.cases[2].in_path.read_text() == "3\n0 1 2\n"
    assert testcases.cases[2].out_path.read_text() == "6\n"
    manifest = json.loads((testcases.dir / fetch.MANIFEST_NAME).read_text())
    assert manifest["source"] == "local"
    assert not (testcases.dir / "build").exists()


def test_local_is_deterministic_and_reused(tmp_path, isolated_cache, monkeypatch):
    problem = make(tmp_path)
    env = env_mod.load("local")
    first = fetch.ensure(problem, env=env)
    from pj.fetch import local

    monkeypatch.setattr(local, "fetch", lambda *a, **k: pytest.fail("作り直してはいけない"))
    second = fetch.ensure(problem, env=env)
    assert second.cases_hash == first.cases_hash


def test_a_failing_reference_is_an_error(tmp_path, isolated_cache):
    problem = make(tmp_path, reference="int main() { return 3; }\n")
    with pytest.raises(fetch.FetchError, match="参照実装"):
        fetch.ensure(problem, env=env_mod.load("local"))


def test_a_failing_generator_is_an_error(tmp_path, isolated_cache):
    problem = make(tmp_path, gen="import sys\nsys.exit(2)\n")
    with pytest.raises(fetch.FetchError, match="gen.py"):
        fetch.ensure(problem, env=env_mod.load("local"))
