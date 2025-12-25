#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SGDialogueTypes.h"
#include "SGStoryState.h"
#include "SGDialogueSubsystem.generated.h"

/**
 * Minimal, Blueprint-friendly dialogue/decision runtime.
 *
 * Usage pattern:
 * - Set up DataTable assets in UE and assign them in Project Settings -> Shattered Gods Narrative.
 * - Call GetRowsByNarrativeId(...) to fetch a node group.
 * - Present DIALOGUE rows to the player; when you hit a DECISION, call GetDecisionOptions(...)
 * - ApplyRowEffects(...) when a line/option is taken.
 */
UCLASS()
class SGNARRATIVE_API USGDialogueSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Reload + rebuild indices (handy after hot-reload or if you swap DataTables). */
	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Dialogue")
	void Reload();

	// --- Lookup ---

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Dialogue")
	bool GetRowsByNarrativeId(FName Id, TArray<FSGDialogueDecisionRow>& OutRows) const;

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Dialogue")
	bool GetFirstRowByNarrativeId(FName Id, FSGDialogueDecisionRow& OutRow) const;

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Dialogue")
	bool GetDecisionOptions(FName DecisionId, const FSGStoryState& State, TArray<FSGDialogueDecisionRow>& OutOptions) const;

	// --- Evaluation / application ---

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Dialogue")
	bool AreConditionsMet(const FSGDialogueDecisionRow& Row, const FSGStoryState& State) const;

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Dialogue")
	bool AreChecksMet(const FSGDialogueDecisionRow& Row, const FSGStoryState& State) const;

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Dialogue")
	void ApplyRowEffects(const FSGDialogueDecisionRow& Row, UPARAM(ref) FSGStoryState& State) const;

	/** Convenience: apply effects and output the next narrative id. */
	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Dialogue")
	bool ApplyRowAndGetNext(const FSGDialogueDecisionRow& Row, UPARAM(ref) FSGStoryState& State, FName& OutNextId) const;

	/** Data access for UI debugging. */
	UFUNCTION(BlueprintPure, Category="Shattered Gods|Dialogue")
	UDataTable* GetDialogueDecisionTable() const { return DialogueDecisionTable; }

private:
	UPROPERTY()
	UDataTable* DialogueDecisionTable = nullptr;

	/** Cache: narrative id -> rows (including prompt + options). */
	TMap<FName, TArray<FSGDialogueDecisionRow>> RowsById;

	void BuildIndex();

	static void ParseStringArrayJson(const FString& JsonLikeArray, TArray<FString>& Out);
	static bool EvalSimpleCheck(const FString& Expr, const FSGStoryState& State);
	static int32 ParseDelta(const FString& DeltaStr);
};
