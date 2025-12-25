# Shattered Gods: Catalyst Rising — Codex Repo Starter (v1)

This repository is a **UE 5.6.x-friendly starter scaffold** for *Shattered Gods: Catalyst Rising*.

It’s designed for two things:
1) **You** to open in Unreal, import narrative CSVs, and start wiring gameplay.
2) **ChatGPT Codex** to read the repo, understand your narrative data + runtime code, and help you build features via PRs.

## What’s inside

### Narrative + data (ready for Unreal DataTables)
- `Narrative/DataTables/` — UE-friendly CSVs (`DT_*_UE.csv`) + original generator CSVs
- `Narrative/Notes/` — extracted state keys, suggested gameplay tags, import notes
- `Narrative/Docs_Source/` — source PDFs/DOCX/TXT for reference
- `Narrative/Generated/DecisionPoints/` — **parsed Decision Points v2** (JSON + UE-friendly CSV)

### UE project scaffold
- `CatalystRising.uproject` — minimal project stub that enables the plugin
- `Source/CatalystRising/` — a minimal game module
- `Plugins/SGNarrative/` — the core **narrative runtime plugin** (C++), including:
  - Dialogue/Decision evaluation (conditions, checks, grants)
  - Branch quest summary + JSON quest detail loader
  - SaveGame payload types
  - Cinematics shotlist lookup helpers

### Tools
- `Tools/` — small Python helpers to validate/transform narrative files

## Unreal quick start (local machine)

1. Install / build **Unreal Engine 5.6.x**.
2. Clone / unzip this repo.
3. Right-click `CatalystRising.uproject` → **Generate project files**.
4. Open the `.uproject` in Unreal.
5. **Import the CSVs** as DataTables:
   - Create DataTable assets in Unreal (Add → Misc → Data Table)
   - Use the matching row structs from the plugin:
     - `FSGDialogueDecisionRow` ← `Narrative/DataTables/DT_DialogueDecisionSuite_UE.csv`
     - `FSGCinematicShotRow`    ← `Narrative/DataTables/DT_CinematicsShotlist_UE.csv`
     - `FSGMainQuestLineRow`    ← `Narrative/DataTables/DT_MainQuestDialogue_UE.csv`
     - `FSGMainQuestLineRow`    ← `Narrative/DataTables/DT_OptionalPrompts_UE.csv` (same struct)
     - `FSGBranchQuestSummaryRow` ← `Narrative/DataTables/DT_BranchQuestOutlines_Summary_UE.csv`
     - `FSGDecisionPointRow` ← `Narrative/Generated/DecisionPoints/DT_DecisionPoints_v2_UE.csv`
6. Configure **Project Settings → Shattered Gods Narrative** (Developer Settings) and point it at your imported DataTable assets.
7. Wire UI / triggers:
   - Use `USGDialogueSubsystem` to fetch nodes by ID, present decisions, and apply results into `FSGStoryState`.

## Codex workflow (cloud + local)

- **Codex Web (chatgpt.com/codex):** great for reading the repo, refactors, adding systems, drafting PRs.
- **Codex CLI (local):** best when you want the agent to run tools that only exist on your machine (Unreal build, cooking, editor automation).

Recommended pattern:
1. Push this repo to GitHub.
2. Connect GitHub inside ChatGPT/Codex.
3. Ask Codex for PRs that add/modify C++ systems and tools.
4. Pull those PRs locally and compile/run Unreal.

## Repo philosophy

Unreal binary assets (`.uasset`, `.umap`) are intentionally **not** included here.
Codex is strongest on text/code. You do the art/blueprints/polish in-engine.

