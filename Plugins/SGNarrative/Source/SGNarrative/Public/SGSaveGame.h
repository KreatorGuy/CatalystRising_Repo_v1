#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SGStoryState.h"
#include "SGQuestSubsystem.h"
#include "SGSaveGame.generated.h"

/**
 * Minimal SaveGame payload for narrative state + quest progress.
 * Extend freely (inventory, world state, checkpoints, etc).
 */
UCLASS()
class SGNARRATIVE_API USGCatalystSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shattered Gods|Save")
	FSGStoryState StoryState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shattered Gods|Save")
	TMap<FName, FSGQuestProgress> QuestProgress;

	/** The current narrative node id (if you want to resume mid-conversation). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shattered Gods|Save")
	FName CurrentDialogueId;
};
