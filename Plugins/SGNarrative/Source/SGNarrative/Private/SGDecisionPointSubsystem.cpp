#include "SGDecisionPointSubsystem.h"
#include "SGNarrativeSettings.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void USGDecisionPointSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Reload();
}

void USGDecisionPointSubsystem::Reload()
{
	DecisionPointsTable = nullptr;
	PromptById.Reset();
	OptionsById.Reset();

	const USGNarrativeSettings* Settings = GetDefault<USGNarrativeSettings>();
	if (Settings)
	{
		DecisionPointsTable = Settings->DecisionPointsTable.LoadSynchronous();
	}

	BuildIndex();

	// Fallback: if no DataTable is configured yet, we can still read the parsed JSON directly.
	if (!DecisionPointsTable && Settings)
	{
		const FString RelPath = Settings->DecisionPointsJson.TrimStartAndEnd();
		if (!RelPath.IsEmpty())
		{
			const FString AbsPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), RelPath));
			FString Json;
			if (FFileHelper::LoadFileToString(Json, *AbsPath))
			{
				TSharedPtr<FJsonObject> Root;
				const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
				if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
				{
					const TArray<TSharedPtr<FJsonValue>>* Dps = nullptr;
					if (Root->TryGetArrayField(TEXT("decision_points"), Dps) && Dps)
					{
						for (const TSharedPtr<FJsonValue>& DpVal : *Dps)
						{
							const TSharedPtr<FJsonObject> DpObj = DpVal.IsValid() ? DpVal->AsObject() : nullptr;
							if (!DpObj.IsValid()) continue;

							const FString DpId = DpObj->GetStringField(TEXT("dp_id"));
							if (DpId.IsEmpty()) continue;

							FSGDecisionPointRow Prompt;
							Prompt.act = DpObj->GetStringField(TEXT("act"));
							Prompt.scene = DpObj->GetStringField(TEXT("scene"));
							Prompt.dp_id = DpId;
							Prompt.row_type = TEXT("PROMPT");
							Prompt.prompt_text = DpObj->GetStringField(TEXT("title"));
							PromptById.Add(DpId, Prompt);

							const TArray<TSharedPtr<FJsonValue>>* Opts = nullptr;
							if (DpObj->TryGetArrayField(TEXT("options"), Opts) && Opts)
							{
								TArray<FSGDecisionPointRow>& Bucket = OptionsById.FindOrAdd(DpId);
								for (const TSharedPtr<FJsonValue>& OptVal : *Opts)
								{
									const TSharedPtr<FJsonObject> OptObj = OptVal.IsValid() ? OptVal->AsObject() : nullptr;
									if (!OptObj.IsValid()) continue;

									FSGDecisionPointRow Opt;
									Opt.act = Prompt.act;
									Opt.scene = Prompt.scene;
									Opt.dp_id = DpId;
									Opt.row_type = TEXT("OPTION");
									Opt.prompt_text = Prompt.prompt_text;
									Opt.option_key = OptObj->GetStringField(TEXT("key"));
									Opt.option_text = OptObj->GetStringField(TEXT("text"));
									Opt.immediate = OptObj->GetStringField(TEXT("immediate"));
									Opt.long_term = OptObj->GetStringField(TEXT("long_term"));
									Bucket.Add(Opt);
								}

								Bucket.Sort([](const FSGDecisionPointRow& A, const FSGDecisionPointRow& B)
								{
									return A.option_key < B.option_key;
								});
							}
						}
					}
				}
			}
		}
	}
}

void USGDecisionPointSubsystem::BuildIndex()
{
	PromptById.Reset();
	OptionsById.Reset();

	if (!DecisionPointsTable)
	{
		return;
	}

	static const FString Context = TEXT("USGDecisionPointSubsystem::BuildIndex");
	TArray<FSGDecisionPointRow*> AllRows;
	DecisionPointsTable->GetAllRows(Context, AllRows);

	for (const FSGDecisionPointRow* Row : AllRows)
	{
		if (!Row) continue;

		const FString DpId = Row->dp_id;
		if (DpId.IsEmpty()) continue;

		if (Row->row_type.Equals(TEXT("PROMPT"), ESearchCase::IgnoreCase))
		{
			PromptById.Add(DpId, *Row);
		}
		else if (Row->row_type.Equals(TEXT("OPTION"), ESearchCase::IgnoreCase))
		{
			OptionsById.FindOrAdd(DpId).Add(*Row);
		}
	}

	for (auto& Pair : OptionsById)
	{
		Pair.Value.Sort([](const FSGDecisionPointRow& A, const FSGDecisionPointRow& B)
		{
			return A.option_key < B.option_key;
		});
	}
}

bool USGDecisionPointSubsystem::GetPrompt(const FString& DpId, FSGDecisionPointRow& OutPrompt) const
{
	if (const FSGDecisionPointRow* Found = PromptById.Find(DpId))
	{
		OutPrompt = *Found;
		return true;
	}
	return false;
}

bool USGDecisionPointSubsystem::GetOptions(const FString& DpId, TArray<FSGDecisionPointRow>& OutOptions) const
{
	OutOptions.Reset();

	if (const TArray<FSGDecisionPointRow>* Found = OptionsById.Find(DpId))
	{
		OutOptions = *Found;
		return OutOptions.Num() > 0;
	}
	return false;
}
