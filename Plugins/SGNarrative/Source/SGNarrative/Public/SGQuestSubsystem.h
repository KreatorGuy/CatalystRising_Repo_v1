#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SGDialogueTypes.h"
#include "SGQuestSubsystem.generated.h"

/** Parsed quest branch details (from BranchQuestOutlines_Expanded.parsed.json). */
USTRUCT(BlueprintType)
struct SGNARRATIVE_API FSGBranchQuestBranch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FString> objectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString encounters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString variables;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString fail_forward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString rejoin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString rewards;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString notes;
};

USTRUCT(BlueprintType)
struct SGNARRATIVE_API FSGBranchQuestDetails
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName code;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString overview;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString preconditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSGBranchQuestBranch> branches;
};

USTRUCT()
struct FSGBranchQuestDetailsFile
{
	GENERATED_BODY()

	UPROPERTY()
	FString generated_from;

	UPROPERTY()
	TArray<FSGBranchQuestDetails> quests;
};

USTRUCT(BlueprintType)
struct SGNARRATIVE_API FSGQuestProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName quest_code;

	/** 0-based index into details.branches. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 branch_index = 0;

	/** 0-based index into branches[branch_index].objectives. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 objective_index = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;
};

/**
 * Quest helper subsystem: loads branch quest summaries (DataTable) and details (JSON), and tracks simple progress.
 */
UCLASS()
class SGNARRATIVE_API USGQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Quests")
	void Reload();

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Quests")
	bool GetQuestSummary(FName Code, FSGBranchQuestSummaryRow& OutRow) const;

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Quests")
	bool GetQuestDetails(FName Code, FSGBranchQuestDetails& OutDetails) const;

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Quests")
	void StartQuest(FName Code, int32 BranchIndex);

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Quests")
	bool GetCurrentObjectiveText(FName Code, FString& OutObjective) const;

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Quests")
	void AdvanceObjective(FName Code, bool bCompleteWhenOutOfObjectives = true);

	UFUNCTION(BlueprintPure, Category="Shattered Gods|Quests")
	bool IsQuestActive(FName Code) const;

	UFUNCTION(BlueprintPure, Category="Shattered Gods|Quests")
	bool IsQuestCompleted(FName Code) const;

	UFUNCTION(BlueprintPure, Category="Shattered Gods|Quests")
	TMap<FName, FSGQuestProgress> GetAllProgress() const { return ProgressByCode; }

private:
	UPROPERTY()
	UDataTable* BranchQuestSummaryTable = nullptr;

	TMap<FName, FSGBranchQuestDetails> DetailsByCode;

	UPROPERTY(EditAnywhere)
	TMap<FName, FSGQuestProgress> ProgressByCode;

	void LoadDetailsJson();
};
