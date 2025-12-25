# Catalyst Rising — Export Pack (v1)

This folder is a **hand-off bundle** for building *Shattered Gods: Catalyst Rising* in **Unreal Engine 5.6.1**.

It includes:
- Source narrative / writing docs (for reference)
- UE-ready **CSV DataTables** for dialogue, decisions, cinematics shots, and main-quest lines
- A parsed **Branch Quest Outlines JSON** (semi-structured) plus a quick summary CSV
- Code templates (optional) for a lightweight dialogue/decision runtime

> Note: Some documents are labeled “Books 1–2” because they were authored as a continuous corpus.
> Catalyst Rising (Game 1) is covered by the **Catalyst Rising** narrative + branch quest outlines, and by the shared dialogue/decision questlines.

---

## Folder layout

- `Docs_Source/`
  - Narrative & writing source files (PDF/DOCX/TXT)
- `UE/DataTables/`
  - Importable CSV tables (Dialogue/Decisions, Cinematic shotlist, Main Quest lines, Optional prompts)
- `UE/Notes/`
  - Parsed JSON + state/flag summaries + UE import notes
- `UE/CodeTemplates/`
  - Optional C++ templates (row structs + subsystem skeleton)
- `Art/`
  - Concept thumbnail sheets + location placement reference

---

## UE import quick-start (DataTables)

### 1) Create row structs
You can either:
- Use the provided templates in `UE/CodeTemplates/` (C++), **or**
- Create Blueprint Structs that match the CSV columns (fastest for prototypes)

### 2) Import CSV → DataTable assets
In UE:
1. **Add** → *Miscellaneous* → **Data Table**
2. Choose the matching row struct
3. Import the CSV from `UE/DataTables/`

Recommended DataTable names (use the *_UE.csv versions for stock UE import):
- `DT_DialogueDecisionSuite`       (from `DT_DialogueDecisionSuite_UE.csv`)
- `DT_MainQuestDialogue`           (from `DT_MainQuestDialogue_UE.csv`)
- `DT_OptionalPrompts`             (from `DT_OptionalPrompts_UE.csv`)
- `DT_CinematicsShotlist`          (from `DT_CinematicsShotlist_UE.csv`)
- `DT_BranchQuestOutlines_Summary` (from `DT_BranchQuestOutlines_Summary_UE.csv`)

The original generator CSVs are included as well (with their original filenames) in case you’re importing via custom code.


### 3) Runtime parsing conventions
In `SG_Dialogue_Decision_v1_0.csv` the fields `checks`, `conditions`, `set_flags`, and `grants`
are stored as JSON-like strings (e.g. `["flag_a"]`, `{ "rep": { "rep_palace": "+1" } }`).

Import them as **FString** (safe), then parse at runtime.

---

## Dialogue & Decision Suite rules (engine-side)

### Node types
- `DIALOGUE`: a spoken line or prompt
- `DECISION`: a choice prompt (no option key)
- `DECISION_OPTION`: one option for the most recent `DECISION` node (same `id`, has `option_key`)

### Branching
- `next` defines the next node ID (or END_* marker)
- `conditions` is a list of required flags
- `checks` is a list of numeric comparisons (ex: `academy_clearance>=1`)

### Save game minimum
Persist:
- Set of **flags** (strings / GameplayTags)
- Numeric state keys (reputation, ranks, trust, etc.)

See `UE/Notes/StateKeys_and_Flags.md` for the currently referenced keys.

---

## Cinematics integration
`SG_Cinematics_Shotlist_v1_0.csv` is designed to map cleanly to:
- Level Sequence shot tracks, or
- A lightweight “shot player” that picks camera rigs + lens presets based on `lens_mm` and `cam_move`

For narrative blocking and dialogue hooks, reference:
- `Docs_Source/Shattered Gods — Cinematic Scripts (books 1–2) V1.docx`

---

## Generated files in this export pack
- `UE/Notes/BranchQuestOutlines_Expanded.parsed.json` — semi-structured quest outline extraction
- `UE/Notes/StateKeys_and_Flags.md` — extracted keys/flags from the dialogue suite CSV
- `UE/Notes/GameplayTags_Suggested.ini` — optional tag list you can paste into DefaultGameplayTags.ini

---

## Source of truth
If anything disagrees, treat the **CSV tables + Catalyst Rising narrative** as authoritative for implementation,
and treat prose docs as reference/intent.

