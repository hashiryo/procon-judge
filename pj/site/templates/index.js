// 問題一覧。data/index.json だけを読む。
//
// 表は 1 枚のまま分けない。id で並べれば出どころごとに固まって見えるし、800 行
// あってもブラウザは平気。探すのは絞り込み、状況を見るのは並べ替えでやる。
"use strict";

function el(tag, text, className) {
  const node = document.createElement(tag);
  if (text !== undefined && text !== null) node.textContent = String(text);
  if (className) node.className = className;
  return node;
}

function stamp(iso) {
  return iso ? iso.slice(0, 16).replace("T", " ") : "";
}

function link(href, text) {
  const a = el("a", text);
  a.href = href;
  return a;
}

// 現行 / 全枠。分子は現行だけで、参考は測り直し待ちなので埋まっている数に数えない。
function covered(p) {
  return p.measured - (p.stale || 0);
}

function frames(p) {
  return p.measured + p.pending;
}

const COLUMNS = [
  {
    id: "id",
    label: "問題",
    text: true,
    value: (p) => p.id,
    cell: (p) => {
      const td = el("td");
      td.append(link("problems/" + encodeURIComponent(p.id) + ".html", p.id));
      if (p.title) td.append(el("div", p.title, "dim"));
      return td;
    },
  },
  {
    id: "source",
    label: "取得元",
    text: true,
    value: (p) => p.source,
    cell: (p) => el("td", p.source, "dim"),
  },
  {
    id: "submissions",
    label: "提出",
    value: (p) => p.submissions,
    cell: (p) => el("td", p.submissions, "n"),
  },
  {
    id: "covered",
    label: "計測済み",
    // 埋まっていない枠が多い順に並べたいことが多いので、値は残りの枠の数にする。
    value: (p) => frames(p) - covered(p),
    cell: (p) => {
      const td = el("td", covered(p) + " / " + frames(p), "n");
      if (frames(p) - covered(p) > 0) td.classList.add("dim");
      return td;
    },
  },
  {
    id: "stale",
    label: "参考",
    value: (p) => p.stale || 0,
    cell: (p) => el("td", p.stale || "-", p.stale ? "n" : "n dim"),
  },
  {
    id: "records",
    label: "記録",
    value: (p) => p.records,
    cell: (p) => el("td", p.records, "n"),
  },
  {
    id: "updated",
    label: "最終更新 (UTC)",
    text: true,
    value: (p) => p.updated || "",
    cell: (p) => {
      const td = el("td", stamp(p.updated), "dim");
      td.title = p.updated || "";
      return td;
    },
  },
];

let DATA = null;
let sortBy = "id";
let ascending = true;
let query = "";

function compare(a, b, column) {
  const x = column.value(a);
  const y = column.value(b);
  return column.text ? String(x).localeCompare(String(y)) : x - y;
}

function matches(p) {
  if (!query) return true;
  const q = query.toLowerCase();
  return p.id.toLowerCase().includes(q) || (p.title || "").toLowerCase().includes(q);
}

function renderHead() {
  const tr = document.getElementById("head");
  tr.replaceChildren();
  for (const column of COLUMNS) {
    const th = el("th", column.label, column.text ? "sortable" : "n sortable");
    th.append(el("span", column.id === sortBy ? (ascending ? "▲" : "▼") : "", "mark"));
    th.addEventListener("click", () => {
      if (sortBy === column.id) ascending = !ascending;
      else {
        sortBy = column.id;
        ascending = true;
      }
      renderHead();
      render();
    });
    tr.append(th);
  }
}

function render() {
  const column = COLUMNS.find((c) => c.id === sortBy);
  const sign = ascending ? 1 : -1;
  const rows = DATA.problems
    .filter(matches)
    .sort((a, b) => sign * compare(a, b, column) || a.id.localeCompare(b.id));

  const body = document.getElementById("rows");
  body.replaceChildren();
  for (const p of rows) {
    const tr = el("tr");
    for (const c of COLUMNS) tr.append(c.cell(p));
    body.append(tr);
  }
  document.getElementById("shown").textContent =
    query ? rows.length + " / " + DATA.problems.length + " 件" : "";
}

async function main() {
  DATA = await (await fetch(document.body.dataset.src)).json();
  const origins = Object.entries(DATA.origins || {})
    .map(([name, count]) => name + " " + count)
    .join(" / ");
  const bits = ["問題 " + DATA.problems.length + (origins ? " (" + origins + ")" : "")];
  bits.push("記録 " + DATA.record_count);
  if (DATA.stale_count) bits.push("参考 " + DATA.stale_count);
  bits.push(stamp(DATA.generated_at) + " 生成 (UTC)");
  document.getElementById("sub").textContent = bits.join(" / ");

  if (DATA.problems.length === 0) {
    document.getElementById("wrap").replaceChildren(
      el("p", "まだ問題がありません。", "empty")
    );
    document.querySelector(".controls").style.display = "none";
    return;
  }

  const filter = document.getElementById("filter");
  filter.addEventListener("input", () => {
    query = filter.value.trim();
    render();
  });
  renderHead();
  render();
}

main().catch((e) => {
  document.getElementById("sub").textContent = "読み込みに失敗しました: " + e;
});
