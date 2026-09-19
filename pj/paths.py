"""リポジトリ内の固定パス。"""

from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

PROBLEMS_DIR = ROOT / "problems"
CACHE_DIR = ROOT / ".cache"
# 記録の置き場。M3 で results ブランチの作業ツリーになる。
RESULTS_DIR = ROOT / ".results"
# サイトの書き先。記録から作り直せるので git には置かない。
SITE_DIR = ROOT / "site"
TESTCASE_CACHE_DIR = CACHE_DIR / "testcases"
BUILD_CACHE_DIR = CACHE_DIR / "build"
LIBRARY_CHECKER_DIR = CACHE_DIR / "library-checker-problems"
LIB_DIR = ROOT / "lib"
SIMDE_DIR = ROOT / "third_party" / "simde"
ENVIRONMENTS_TOML = ROOT / "environments.toml"
