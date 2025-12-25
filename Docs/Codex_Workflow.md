# Codex Workflow Notes

This repo is structured so Codex can understand it without needing Unreal installed.

## Suggested prompts for Codex
- "Add a BlueprintCallable function to USGDialogueSubsystem that returns the node type enum for a given id"
- "Add validation: warn when a dialogue 'next' id doesn't exist"
- "Generate an example ConversationController C++ actor component that drives a UMG widget via delegates"
- "Add unit-test-like validation scripts under Tools/"

## Best practice
- Keep binary assets out of PRs (uasset/umap). Have Codex write code + text instructions instead.
