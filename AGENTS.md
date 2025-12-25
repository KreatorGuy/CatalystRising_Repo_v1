# Codex Instructions (Project: Catalyst Rising / UE 5.6.x)

You are working in a **Unreal Engine 5.6.x** game repo.

## Primary goals
- Build a clean, minimal, engine-practical narrative runtime for **dialogue + decisions + quest progress**.
- Keep the code **Blueprint-friendly** (BlueprintCallable, BlueprintType structs) so the project owner can do most orchestration in BP/UMG.

## Hard constraints
- Do NOT create or modify binary Unreal assets (`.uasset`, `.umap`). Provide instructions instead.
- Prefer **small, reviewable PRs**.
- Don’t assume Unreal is installed in your environment.

## Data sources
- Dialogue/decisions: `Narrative/DataTables/DT_DialogueDecisionSuite_UE.csv`
- Main quest lines: `Narrative/DataTables/DT_MainQuestDialogue_UE.csv`
- Optional prompts: `Narrative/DataTables/DT_OptionalPrompts_UE.csv`
- Cinematic shots: `Narrative/DataTables/DT_CinematicsShotlist_UE.csv`
- Branch quests: `Narrative/DataTables/DT_BranchQuestOutlines_Summary_UE.csv` + `Narrative/Notes/BranchQuestOutlines_Expanded.parsed.json`
- Decision points: `Narrative/Generated/DecisionPoints/DT_DecisionPoints_v2_UE.csv`

## Coding style
- Keep Unreal-style naming (PascalCase types, bPrefix booleans).
- Keep runtime allocations reasonable; cache indices where practical.
- Add comments that explain *why*, not *what*.

## Safety / narrative
- This is fictional narrative content. Don’t add real-person allegations or anything that would force content policy issues.
