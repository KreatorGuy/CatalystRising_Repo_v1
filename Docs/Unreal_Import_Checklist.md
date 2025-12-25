# Unreal Import Checklist (DataTables + Settings)

## 1) Create/import DataTables
In Unreal Editor:
1. **Add → Miscellaneous → Data Table**
2. Pick the matching row struct (from plugin `SGNarrative`)
3. Import CSV

### Recommended mapping
| CSV | Struct |
|---|---|
| `Narrative/DataTables/DT_DialogueDecisionSuite_UE.csv` | `FSGDialogueDecisionRow` |
| `Narrative/DataTables/DT_CinematicsShotlist_UE.csv` | `FSGCinematicShotRow` |
| `Narrative/DataTables/DT_MainQuestDialogue_UE.csv` | `FSGMainQuestLineRow` |
| `Narrative/DataTables/DT_OptionalPrompts_UE.csv` | `FSGMainQuestLineRow` |
| `Narrative/DataTables/DT_BranchQuestOutlines_Summary_UE.csv` | `FSGBranchQuestSummaryRow` |
| `Narrative/Generated/DecisionPoints/DT_DecisionPoints_v2_UE.csv` | `FSGDecisionPointRow` |

## 2) Assign tables in settings
- Project Settings → **Shattered Gods Narrative**
  - Set each DataTable reference.
  - `BranchQuestDetailsJson` and `DecisionPointsJson` are already set in `Config/DefaultGame.ini` for early prototyping.

## 3) Quick smoke test
- Create a throwaway Blueprint Actor.
- On BeginPlay:
  - Get GameInstanceSubsystem → `SGDialogueSubsystem`
  - Call `GetFirstRowByNarrativeId` with `GA_001` (Glow Audit) and print the `line_or_prompt`.
