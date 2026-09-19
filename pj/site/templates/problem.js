// 1 問題の順位表。data/problems/<id>.json だけを読む。
"use strict";

function el(tag, text, className) {
  const node = document.createElement(tag);
  if (text !== undefined && text !== null) node.textContent = String(text);
  if (className) node.className = className;
  return node;
}

function stamp(iso) {
  return iso ? iso.replace("T", " ").replace("Z", " UTC") : "";
}

function ms(ns) {
  return ns === null || ns === undefined ? "-" : (ns / 1e6).toFixed(2) + " ms";
}

function mb(kb) {
  return kb ? (kb / 1024).toFixed(1) + " MB" : "-";
}

function bytes(n) {
  if (n === null || n === undefined) return "-";
  return n < 1024 ? n + " B" : (n / 1024).toFixed(1) + " KB";
}

// 順位は algo の最大ケースで決める。計測区間の外の I/O と整形を含まないので、
// 実装どうしを比べるならこちら。algo が無い記録は実時間で代用する。
function speed(row) {
  return row.algo_ns === null || row.algo_ns === undefined
    ? row.wall_ms * 1e6
    : row.algo_ns;
}

let DATA = null;

function envs() {
  return [...new Set(DATA.combos.map((c) => c.env))];
}

function modelsOf(env) {
  return DATA.combos.filter((c) => c.env === env).map((c) => c.cpu_model);
}

function fill(select, values, keep) {
  select.replaceChildren();
  for (const v of values) {
    const option = el("option", v);
    option.value = v;
    select.append(option);
  }
  if (keep && values.includes(keep)) select.value = keep;
}

function render() {
  const env = document.getElementById("env").value;
  const model = document.getElementById("model").value;
  const combo = DATA.combos.find(
    (c) => c.env === env && c.cpu_model === model
  );
  document.getElementById("combo-meta").replaceChildren(
    el("span", combo ? combo.compiler_version : ""),
    el("span", DATA.cxxflags[env] || "", "flags mono")
  );

  const rows = DATA.rows.filter((r) => r.env === env && r.cpu_model === model);
  const byName = new Map(rows.map((r) => [r.submission, r]));
  const ac = rows.filter((r) => r.status === "AC").sort((a, b) => speed(a) - speed(b));
  const other = rows
    .filter((r) => r.status !== "AC")
    .sort((a, b) => a.submission.localeCompare(b.submission));
  const missing = DATA.submissions.filter((s) => !byName.has(s));

  const body = document.getElementById("rows");
  body.replaceChildren();
  ac.forEach((row, index) => body.append(line(row, index + 1)));
  other.forEach((row) => body.append(line(row, null)));
  missing.forEach((name) => body.append(missingLine(name)));
}

function line(row, rank) {
  const tr = el("tr");
  tr.append(el("td", rank === null ? "" : rank, "n"));
  tr.append(el("td", row.submission, "mono"));

  const status = el("td", row.status, "st st-" + row.status);
  if (row.failed && row.failed.name) {
    status.append(el("div", row.failed.name, "dim"));
  }
  if (row.failed && row.failed.detail) status.title = row.failed.detail;
  tr.append(status);

  // 打ち切られた実行の algo は通ったケースまでの値でしかない。数字は残すが
  // 順位の根拠には見えないように落とす。
  tr.append(el("td", ms(row.algo_ns), row.status === "AC" ? "n" : "n dim"));
  tr.append(el("td", row.wall_ms + " ms", "n"));
  tr.append(el("td", mb(row.rss_kb), "n"));
  tr.append(el("td", bytes(row.source_bytes), "n"));
  tr.append(el("td", bytes(row.binary_bytes), "n"));
  tr.append(el("td", row.samples, "n"));
  tr.append(el("td", stamp(row.timestamp), "dim"));
  return tr;
}

function missingLine(name) {
  const tr = el("tr");
  tr.append(el("td", "", "n"));
  tr.append(el("td", name, "mono dim"));
  const cell = el("td", "未計測", "dim");
  cell.colSpan = 7;
  tr.append(cell);
  return tr;
}

async function main() {
  DATA = await (await fetch(document.body.dataset.src)).json();
  document.getElementById("title").textContent = DATA.title || DATA.id;
  document.getElementById("sub").textContent = DATA.id;
  document.getElementById("meta").replaceChildren(
    el("span", "取得元 " + DATA.source),
    el("span", "比較 " + DATA.compare),
    el("span", "制限 " + DATA.tle_sec + " 秒 / " + DATA.mle_mb + " MB"),
    el("span", "ケース " + DATA.case_count),
    el("span", "提出 " + DATA.submissions.length)
  );

  if (DATA.combos.length === 0) {
    document.getElementById("wrap").replaceChildren(
      el("p", "まだ記録がありません。", "empty")
    );
    document.getElementById("controls").style.display = "none";
    return;
  }

  const envSelect = document.getElementById("env");
  const modelSelect = document.getElementById("model");
  fill(envSelect, envs());
  envSelect.value = DATA.combos[0].env;
  fill(modelSelect, modelsOf(envSelect.value));
  modelSelect.value = DATA.combos[0].cpu_model;

  envSelect.addEventListener("change", () => {
    // 環境を変えても、同じ CPU モデルがあれば選び直さない。
    fill(modelSelect, modelsOf(envSelect.value), modelSelect.value);
    render();
  });
  modelSelect.addEventListener("change", render);
  render();
}

main().catch((e) => {
  document.getElementById("sub").textContent = "読み込みに失敗しました: " + e;
});
