# Next Steps (Suggested Roadmap)

## Phase 0 — Sanity + imports
- [ ] Run `python Tools/validate_narrative_data.py`
- [ ] Import DataTables into Unreal and assign them in **Project Settings → Shattered Gods Narrative**
- [ ] Create a tiny test map and print a line from `GA_001` on BeginPlay

## Phase 1 — Core narrative loop
- [ ] Create a UMG dialogue widget
- [ ] Implement a Blueprint-driven conversation controller:
  - fetch node by id
  - display dialogue
  - when node is decision: query options, show buttons
  - on selection: apply effects + go to `next`
- [ ] Add one actor component `BP_DialogueTrigger` that starts a conversation when interacted

## Phase 2 — Quest progression
- [ ] Use `USGQuestSubsystem` to start/advance branch quests
- [ ] Hook quest completion into StoryState flags (carryover, reputation, etc.)
- [ ] Add UI quest tracker (current objective text)

## Phase 3 — Cinematics
- [ ] Build a Level Sequence pipeline for scenes
- [ ] Use `USGCinematicsSubsystem` shotlist to block camera framing + durations

## Phase 4 — Save/Load
- [ ] Save story + quest progress to slots
- [ ] Add a debug menu: show flags/ints, current quest objective
