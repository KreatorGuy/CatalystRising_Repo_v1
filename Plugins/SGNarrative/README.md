# SGNarrative Plugin

This is a lightweight runtime plugin that turns your narrative CSV/JSON into **Blueprint-friendly gameplay data**.

## Key classes
- `USGDialogueSubsystem`
  - Loads the Dialogue/Decision DataTable
  - Groups rows by narrative id
  - Evaluates conditions + checks against `FSGStoryState`
  - Applies `set_flags` and `grants`

- `USGQuestSubsystem`
  - Loads quest summary DataTable
  - Loads expanded quest details JSON
  - Tracks a simple objective index per quest

- `USGCinematicsSubsystem`
  - Loads the shotlist DataTable
  - Returns ordered shots for a given scene

- `USGDecisionPointSubsystem`
  - Loads decision points DataTable (or falls back to JSON)

- `USGCatalystSaveGame` + `USGSaveGameLibrary`
  - Starter save payload for story state + quest progress

## Intended use
This plugin intentionally avoids dictating your UI, input flow, or Level Sequence pipeline.
It gives you clean data and predictable evaluation. You do the fun part.
