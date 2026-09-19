"""出力比較。ここが壊れると記録の意味が変わるので固めておく。"""

from pj.compare import compare_tokens


def write(tmp_path, name, text):
    path = tmp_path / name
    path.write_text(text)
    return path


def test_identical(tmp_path):
    a = write(tmp_path, "a", "1 2 3\n")
    b = write(tmp_path, "b", "1 2 3\n")
    assert compare_tokens(a, b).ok


def test_whitespace_and_newlines_are_ignored(tmp_path):
    a = write(tmp_path, "a", "1\n2\n3\n")
    b = write(tmp_path, "b", "  1 2   3  \n\n")
    assert compare_tokens(a, b).ok


def test_missing_trailing_newline(tmp_path):
    a = write(tmp_path, "a", "1 2 3")
    b = write(tmp_path, "b", "1 2 3\n")
    assert compare_tokens(a, b).ok


def test_differing_token(tmp_path):
    a = write(tmp_path, "a", "1 9 3\n")
    b = write(tmp_path, "b", "1 2 3\n")
    result = compare_tokens(a, b)
    assert not result.ok
    assert "token 1" in result.detail


def test_extra_token(tmp_path):
    a = write(tmp_path, "a", "1 2 3 4\n")
    b = write(tmp_path, "b", "1 2 3\n")
    assert not compare_tokens(a, b).ok


def test_missing_token(tmp_path):
    a = write(tmp_path, "a", "1 2\n")
    b = write(tmp_path, "b", "1 2 3\n")
    assert not compare_tokens(a, b).ok


def test_both_empty(tmp_path):
    a = write(tmp_path, "a", "")
    b = write(tmp_path, "b", "\n")
    assert compare_tokens(a, b).ok


def test_numbers_compare_as_text(tmp_path):
    # トークン比較なので 1 と 1.0 は違う。誤差を許したいなら compare.kind = "float"。
    a = write(tmp_path, "a", "1.0\n")
    b = write(tmp_path, "b", "1\n")
    assert not compare_tokens(a, b).ok
