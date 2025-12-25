"""Parse Catalyst Rising Decision Points PDF into JSON + UE-friendly CSV.

Inputs:
  Narrative/Docs_Source/Catalyst_Rising_Branching_Narrative_v2_Added_Decision_Points.pdf

Outputs:
  Narrative/Generated/DecisionPoints/DecisionPoints_v2.parsed.json
  Narrative/Generated/DecisionPoints/DT_DecisionPoints_v2_UE.csv

Notes:
  - Uses PyPDF2 text extraction (no OCR).
  - Parser is intentionally tolerant (the source uses bullets, unicode minus, etc.).
"""

from __future__ import annotations

import json
import re
from pathlib import Path

import pandas as pd
from PyPDF2 import PdfReader

ROOT = Path(__file__).resolve().parents[1]
PDF_PATH = ROOT / "Narrative" / "Docs_Source" / "Catalyst_Rising_Branching_Narrative_v2_Added_Decision_Points.pdf"
OUT_DIR = ROOT / "Narrative" / "Generated" / "DecisionPoints"


def extract_lines(pdf_path: Path) -> list[str]:
    reader = PdfReader(str(pdf_path))
    text = "\n".join([(p.extract_text() or "") for p in reader.pages])
    # normalize
    text = text.replace("\uf0b7", "•").replace("■", "-").replace("\x7f", "").replace("• •", "••")
    lines = [ln.strip() for ln in text.splitlines()]
    return [ln for ln in lines if ln]


def parse(lines: list[str]) -> list[dict]:
    dp_re = re.compile(r"^(DP-\d+[A-Z]?)\s+—\s+(.*)$")
    scene_re = re.compile(r"^S\d+[A-Z]?\s+—\s+(.+)$")
    act_re = re.compile(r"^Act\s+[IVX]+")
    opt_re = re.compile(r"^Option\s+([A-Z])\s*:\s*(.*)$")

    dps: list[dict] = []
    current_act: str | None = None
    current_scene_line: str | None = None
    current_dp: dict | None = None
    current_opt: dict | None = None

    def finalize():
        nonlocal current_dp, current_opt
        if current_dp:
            dps.append(current_dp)
        current_dp = None
        current_opt = None

    for line in lines:
        if act_re.match(line):
            current_act = line
            continue
        if scene_re.match(line):
            current_scene_line = line
            continue
        m_dp = dp_re.match(line)
        if m_dp:
            finalize()
            current_dp = {
                "dp_id": m_dp.group(1),
                "title": m_dp.group(2),
                "act": current_act or "",
                "scene": current_scene_line or "",
                "options": [],
            }
            continue
        m_opt = opt_re.match(line)
        if m_opt and current_dp is not None:
            current_opt = {
                "key": m_opt.group(1),
                "text": m_opt.group(2),
                "immediate": "",
                "long_term": "",
            }
            current_dp["options"].append(current_opt)
            continue
        if current_dp is not None and current_opt is not None:
            if line.startswith("Immediate:"):
                val = line[len("Immediate:") :].strip()
                current_opt["immediate"] = (current_opt["immediate"] + " " + val).strip()
                continue
            if line.startswith("Long-term:"):
                val = line[len("Long-term:") :].strip()
                current_opt["long_term"] = (current_opt["long_term"] + " " + val).strip()
                continue

    finalize()
    return dps


def to_ue_csv(decision_points: list[dict]) -> pd.DataFrame:
    rows: list[dict] = []
    for dp in decision_points:
        dp_id = dp["dp_id"]
        norm = re.sub(r"[^A-Za-z0-9_]", "_", dp_id.replace("-", "_"))

        rows.append(
            {
                "Name": f"{norm}_PROMPT",
                "act": dp.get("act", ""),
                "scene": dp.get("scene", ""),
                "dp_id": dp_id,
                "row_type": "PROMPT",
                "prompt_text": dp.get("title", ""),
                "option_key": "",
                "option_text": "",
                "immediate": "",
                "long_term": "",
            }
        )

        for opt in dp.get("options", []):
            rows.append(
                {
                    "Name": f"{norm}_OPT_{opt.get('key','')}",
                    "act": dp.get("act", ""),
                    "scene": dp.get("scene", ""),
                    "dp_id": dp_id,
                    "row_type": "OPTION",
                    "prompt_text": dp.get("title", ""),
                    "option_key": opt.get("key", ""),
                    "option_text": opt.get("text", ""),
                    "immediate": opt.get("immediate", ""),
                    "long_term": opt.get("long_term", ""),
                }
            )

    return pd.DataFrame(rows)


def main() -> None:
    if not PDF_PATH.exists():
        raise FileNotFoundError(PDF_PATH)

    OUT_DIR.mkdir(parents=True, exist_ok=True)

    lines = extract_lines(PDF_PATH)
    dps = parse(lines)

    json_path = OUT_DIR / "DecisionPoints_v2.parsed.json"
    json_path.write_text(
        json.dumps(
            {
                "source_pdf": PDF_PATH.name,
                "decision_points": dps,
            },
            ensure_ascii=False,
            indent=2,
        ),
        encoding="utf-8",
    )

    df = to_ue_csv(dps)
    csv_path = OUT_DIR / "DT_DecisionPoints_v2_UE.csv"
    df.to_csv(csv_path, index=False)

    print(f"Wrote: {json_path}")
    print(f"Wrote: {csv_path}")


if __name__ == "__main__":
    main()
