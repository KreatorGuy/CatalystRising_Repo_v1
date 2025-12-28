#include "SGQuestSubsystem.h"
#include "SGNarrativeSettings.h"

#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void USGQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Reload();
}

void USGQuestSubsystem::Reload()
{
	BranchQuestSummaryTable = nullptr;
	DetailsByCode.Reset();

	const USGNarrativeSettings* Settings = GetDefault<USGNarrativeSettings>();
	if (Settings)
	{
		BranchQuestSummaryTable = Settings->BranchQuestSummaryTable.LoadSynchronous();
	}

	LoadDetailsJson();
}

bool USGQuestSubsystem::GetQuestSummary(FName Code, FSGBranchQuestSummaryRow& OutRow) const
{
	if (!BranchQuestSummaryTable)
	{
		return false;
	}

	// The UE-friendly CSV uses the first column as the DataTable row name.
	// In DT_BranchQuestOutlines_Summary_UE.csv the Name is the quest code.
	static const FString Context(TEXT("USGQuestSubsystem::GetQuestSummary"));
	if (const FSGBranchQuestSummaryRow* Row = BranchQuestSummaryTable->FindRow<FSGBranchQuestSummaryRow>(Code, Context))
	{
		OutRow = *Row;
		return true;
	}

	// Fallback: scan by code field
	TArray<FSGBranchQuestSummaryRow*> AllRows;
	BranchQuestSummaryTable->GetAllRows(Context, AllRows);
	for (const FSGBranchQuestSummaryRow* R : AllRows)
	{
		if (R && R->code == Code)
		{
			OutRow = *R;
			return true;
		}
	}

	return false;
}

bool USGQuestSubsystem::GetQuestDetails(FName Code, FSGBranchQuestDetails& OutDetails) const
{
	if (const FSGBranchQuestDetails* Found = DetailsByCode.Find(Code))
	{
		OutDetails = *Found;
		return true;
	}
	return false;
}

void USGQuestSubsystem::StartQuest(FName Code, int32 BranchIndex)
{
	FSGQuestProgress P;
	P.quest_code = Code;
	P.branch_index = FMath::Max(0, BranchIndex);
	P.objective_index = 0;
	P.bCompleted = false;

	// Clamp to available branches if details exist.
	if (const FSGBranchQuestDetails* D = DetailsByCode.Find(Code))
	{
		if (D->branches.Num() > 0)
		{
			P.branch_index = FMath::Clamp(P.branch_index, 0, D->branches.Num() - 1);
		}
		else
		{
			P.branch_index = 0;
		}
	}

	ProgressByCode.Add(Code, P);
}

bool USGQuestSubsystem::GetCurrentObjectiveText(FName Code, FString& OutObjective) const
{
	OutObjective.Empty();

	const FSGQuestProgress* P = ProgressByCode.Find(Code);
	if (!P || P->bCompleted)
	{
		return false;
	}

	const FSGBranchQuestDetails* D = DetailsByCode.Find(Code);
	if (D && D->branches.IsValidIndex(P->branch_index))
	{
		const FSGBranchQuestBranch& B = D->branches[P->branch_index];
		if (B.objectives.IsValidIndex(P->objective_index))
		{
			OutObjective = B.objectives[P->objective_index];
			return true;
		}
	}

	// Fallback to summary overview.
	FSGBranchQuestSummaryRow Summary;
	if (GetQuestSummary(Code, Summary))
	{
		OutObjective = Summary.overview;
		return true;
	}

	return false;
}

void USGQuestSubsystem::AdvanceObjective(FName Code, bool bCompleteWhenOutOfObjectives)
{
	FSGQuestProgress* P = ProgressByCode.Find(Code);
	if (!P || P->bCompleted)
	{
		return;
	}

	P->objective_index++;

	// If we have details, see if we've gone past the end.
	const FSGBranchQuestDetails* D = DetailsByCode.Find(Code);
	if (D && D->branches.IsValidIndex(P->branch_index))
	{
		const int32 MaxIdx = D->branches[P->branch_index].objectives.Num();
		if (P->objective_index >= MaxIdx)
		{
			if (bCompleteWhenOutOfObjectives)
			{
				P->bCompleted = true;
			}
			else
			{
				P->objective_index = FMath::Max(0, MaxIdx - 1);
			}
		}
	}
}

bool USGQuestSubsystem::IsQuestActive(FName Code) const
{
	if (const FSGQuestProgress* P = ProgressByCode.Find(Code))
	{
		return !P->bCompleted;
	}
	return false;
}

bool USGQuestSubsystem::IsQuestCompleted(FName Code) const
{
	if (const FSGQuestProgress* P = ProgressByCode.Find(Code))
	{
		return P->bCompleted;
	}
	return false;
}

void USGQuestSubsystem::LoadDetailsJson()
{
	DetailsByCode.Reset();

	const USGNarrativeSettings* Settings = GetDefault<USGNarrativeSettings>();
	if (!Settings)
	{
		return;
	}

	FString RelPath = Settings->BranchQuestDetailsJson;
	RelPath = RelPath.TrimStartAndEnd();
	if (RelPath.IsEmpty())
	{
		return;
	}

	const FString AbsPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), RelPath));

	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *AbsPath))
	{
		return;
	}

	FSGBranchQuestDetailsFile FileObj;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(Json, &FileObj, 0, 0))
	{
		return;
	}

	for (const FSGBranchQuestDetails& Q : FileObj.quests)
	{
		DetailsByCode.Add(Q.code, Q);
	}
}
