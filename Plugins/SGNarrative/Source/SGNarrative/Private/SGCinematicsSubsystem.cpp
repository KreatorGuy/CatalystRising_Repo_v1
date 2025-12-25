#include "SGCinematicsSubsystem.h"
#include "SGNarrativeSettings.h"

void USGCinematicsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Reload();
}

void USGCinematicsSubsystem::Reload()
{
	ShotlistTable = nullptr;
	ShotsByScene.Reset();

	const USGNarrativeSettings* Settings = GetDefault<USGNarrativeSettings>();
	if (Settings)
	{
		ShotlistTable = Settings->CinematicsShotlistTable.LoadSynchronous();
	}

	BuildIndex();
}

void USGCinematicsSubsystem::BuildIndex()
{
	ShotsByScene.Reset();

	if (!ShotlistTable)
	{
		return;
	}

	static const FString Context = TEXT("USGCinematicsSubsystem::BuildIndex");
	TArray<FSGCinematicShotRow*> AllRows;
	ShotlistTable->GetAllRows(Context, AllRows);

	for (const FSGCinematicShotRow* Row : AllRows)
	{
		if (!Row) continue;

		const FString Key = Row->questline + TEXT("|") + Row->scene_id;
		TArray<FSGCinematicShotRow>& Bucket = ShotsByScene.FindOrAdd(Key);
		Bucket.Add(*Row);
	}

	// Sort shots by shot number per scene.
	for (auto& Pair : ShotsByScene)
	{
		Pair.Value.Sort([](const FSGCinematicShotRow& A, const FSGCinematicShotRow& B)
		{
			return A.shot_no < B.shot_no;
		});
	}
}

bool USGCinematicsSubsystem::GetShotsForScene(const FString& Questline, const FString& SceneId, TArray<FSGCinematicShotRow>& OutShots) const
{
	OutShots.Reset();

	const FString Key = Questline + TEXT("|") + SceneId;
	if (const TArray<FSGCinematicShotRow>* Found = ShotsByScene.Find(Key))
	{
		OutShots = *Found;
		return OutShots.Num() > 0;
	}

	return false;
}
