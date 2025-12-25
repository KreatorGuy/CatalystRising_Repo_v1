#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SGDialogueTypes.h"
#include "SGCinematicsSubsystem.generated.h"

/**
 * Simple helper subsystem to query the cinematics shotlist DataTable.
 * This does NOT attempt to drive Level Sequences automatically (that becomes project-specific fast).
 */
UCLASS()
class SGNARRATIVE_API USGCinematicsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Cinematics")
	void Reload();

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Cinematics")
	bool GetShotsForScene(const FString& Questline, const FString& SceneId, TArray<FSGCinematicShotRow>& OutShots) const;

	UFUNCTION(BlueprintPure, Category="Shattered Gods|Cinematics")
	UDataTable* GetShotlistTable() const { return ShotlistTable; }

private:
	UPROPERTY()
	UDataTable* ShotlistTable = nullptr;

	/** Cache key: Questline|SceneId */
	TMap<FString, TArray<FSGCinematicShotRow>> ShotsByScene;

	void BuildIndex();
};
