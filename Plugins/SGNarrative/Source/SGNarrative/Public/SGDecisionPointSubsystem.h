#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SGDialogueTypes.h"
#include "SGDecisionPointSubsystem.generated.h"

/**
 * Decision Point helper: loads DT_DecisionPoints_v2_UE.csv (imported as a DataTable) and allows lookup by dp_id.
 */
UCLASS()
class SGNARRATIVE_API USGDecisionPointSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|DecisionPoints")
	void Reload();

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|DecisionPoints")
	bool GetPrompt(const FString& DpId, FSGDecisionPointRow& OutPrompt) const;

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|DecisionPoints")
	bool GetOptions(const FString& DpId, TArray<FSGDecisionPointRow>& OutOptions) const;

	UFUNCTION(BlueprintPure, Category="Shattered Gods|DecisionPoints")
	UDataTable* GetDecisionPointsTable() const { return DecisionPointsTable; }

private:
	UPROPERTY()
	UDataTable* DecisionPointsTable = nullptr;

	TMap<FString, FSGDecisionPointRow> PromptById;
	TMap<FString, TArray<FSGDecisionPointRow>> OptionsById;

	void BuildIndex();
};
