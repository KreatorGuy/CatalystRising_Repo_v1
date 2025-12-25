#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SGSaveGame.h"
#include "SGSaveGameLibrary.generated.h"

UCLASS()
class SGNARRATIVE_API USGSaveGameLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Save")
	static USGCatalystSaveGame* CreateNewSave();

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Save")
	static bool SaveToSlot(USGCatalystSaveGame* SaveObj, const FString& SlotName, int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Save")
	static USGCatalystSaveGame* LoadFromSlot(const FString& SlotName, int32 UserIndex = 0);
};
