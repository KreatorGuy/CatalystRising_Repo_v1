#include "SGSaveGameLibrary.h"
#include "Kismet/GameplayStatics.h"

USGCatalystSaveGame* USGSaveGameLibrary::CreateNewSave()
{
	return Cast<USGCatalystSaveGame>(UGameplayStatics::CreateSaveGameObject(USGCatalystSaveGame::StaticClass()));
}

bool USGSaveGameLibrary::SaveToSlot(USGCatalystSaveGame* SaveObj, const FString& SlotName, int32 UserIndex)
{
	if (!SaveObj)
	{
		return false;
	}
	return UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, UserIndex);
}

USGCatalystSaveGame* USGSaveGameLibrary::LoadFromSlot(const FString& SlotName, int32 UserIndex)
{
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
	return Cast<USGCatalystSaveGame>(Loaded);
}
