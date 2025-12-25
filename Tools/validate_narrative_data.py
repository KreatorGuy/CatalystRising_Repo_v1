"""Basic validation for the narrative CSVs.

This is intentionally *not* an Unreal build step. It's a quick sanity check
before importing DataTables or after editing CSV/JSON.

Checks:
- DT_DialogueDecisionSuite_UE.csv
  - next IDs exist
  - decision nodes have options
  - JSON-like fields parse as JSON (best-effort)

Usage:
  python Tools/validate_narrative_data.py
"""

from __future__ import annotations

import json
from pathlib import Path

import pandas as pd

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "Narrative" / "DataTables"


def _try_json(value: str):
    value = (value or "").strip()
    if not value:
        return None
    try:
        return json.loads(value)
    except Exception:
        return None


def validate_dialogue_suite() -> list[str]:
    errors: list[str] = []

    path = DATA / "DT_DialogueDecisionSuite_UE.csv"
    if not path.exists():
        return [f"Missing: {path}"]

    df = pd.read_csv(path)

    required_cols = [
        "id",
        "type",
        "line_or_prompt",
        "option_key",
        "option_text",
        "checks",
        "conditions",
        "set_flags",
        "grants",
        "next",
    ]
    for c in required_cols:
        if c not in df.columns:
            errors.append(f"DT_DialogueDecisionSuite_UE.csv missing column: {c}")

    if errors:
        return errors

    ids = set(df["id"].astype(str))

    # next id exists
    for i, row in df.iterrows():
        nxt = str(row.get("next") or "").strip()
        if nxt and nxt != "nan" and nxt not in ids:
            errors.append(f"Row {i} id={row['id']} points to missing next={nxt}")

    # decision has options
    for decision_id, group in df.groupby("id"):
        types = set(group["type"].astype(str).str.upper())
        if "DECISION" in types and "DECISION_OPTION" not in types:
            errors.append(f"Decision id={decision_id} has no DECISION_OPTION rows")

    # option rows have key/text
    opt_rows = df[df["type"].astype(str).str.upper() == "DECISION_OPTION"]
    for i, row in opt_rows.iterrows():
        k = str(row.get("option_key") or "").strip()
        t = str(row.get("option_text") or "").strip()
        if not k or k == "nan":
            errors.append(f"Option row {i} id={row['id']} missing option_key")
        if not t or t == "nan":
            errors.append(f"Option row {i} id={row['id']} missing option_text")

    # JSON-like fields parse
    for field in ["checks", "conditions", "set_flags"]:
        for i, row in df.iterrows():
            v = str(row.get(field) or "").strip()
            if not v or v == "nan":
                continue
            if _try_json(v) is None:
                errors.append(f"Row {i} id={row['id']} field {field} is not valid JSON: {v}")

    # grants is either empty, {}, or valid JSON
    for i, row in df.iterrows():
        v = str(row.get("grants") or "").strip()
        if not v or v == "nan" or v == "{}":
            continue
        if _try_json(v) is None:
            errors.append(f"Row {i} id={row['id']} grants is not valid JSON: {v}")

    return errors


def main() -> None:
    errs = []
    errs.extend(validate_dialogue_suite())

    if errs:
        print("\n".join(["ERROR: " + e for e in errs]))
        raise SystemExit(1)

    print("OK: narrative CSVs look structurally sane.")


if __name__ == "__main__":
    main()
