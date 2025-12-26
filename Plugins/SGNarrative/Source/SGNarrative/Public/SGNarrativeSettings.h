#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"

// Prefer engine-provided FFilePath where available.
#if __has_include("Misc/FilePath.h")
#include "Misc/FilePath.h"
#elif __has_include("UObject/SoftObjectPath.h")
#include "UObject/SoftObjectPath.h"
#else
// Fallback when neither header is available; keep it plain so UHT stays happy.
struct FFilePath
{
    FString FilePath;
};
#endif

#include "SGNarrativeSettings.generated.h"

/**
 * Project-level settings for narrative data sources.
 *
 * These are intentionally soft references so the project can import CSVs into DataTables anywhere in Content/.
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Shattered Gods Narrative"))
class SGNARRATIVE_API USGNarrativeSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** Dialogue + decisions table. */
    UPROPERTY(config, EditAnywhere, Category="Data", meta=(AllowedClasses="DataTable"))
    TSoftObjectPtr<UDataTable> DialogueDecisionTable;

    /** Cinematics shotlist table. */
    UPROPERTY(config, EditAnywhere, Category="Data", meta=(AllowedClasses="DataTable"))
    TSoftObjectPtr<UDataTable> CinematicsShotlistTable;

    /** Branch quest summary table. */
    UPROPERTY(config, EditAnywhere, Category="Data", meta=(AllowedClasses="DataTable"))
    TSoftObjectPtr<UDataTable> BranchQuestSummaryTable;

    /** Main quest dialogue table. */
    UPROPERTY(config, EditAnywhere, Category="Data", meta=(AllowedClasses="DataTable"))
    TSoftObjectPtr<UDataTable> MainQuestDialogueTable;

    /** Optional prompts table (same schema as main quest dialogue). */
    UPROPERTY(config, EditAnywhere, Category="Data", meta=(AllowedClasses="DataTable"))
    TSoftObjectPtr<UDataTable> OptionalPromptsTable;

    /** Optional: parsed decision points table. */
    UPROPERTY(config, EditAnywhere, Category="Data", meta=(AllowedClasses="DataTable"))
    TSoftObjectPtr<UDataTable> DecisionPointsTable;

    /** JSON file path (relative to ProjectDir) containing expanded branch quest details. */
    UPROPERTY(config, EditAnywhere, Category="Data")
    FFilePath BranchQuestDetailsJson;

    /** JSON file path (relative to ProjectDir) containing parsed decision points. */
    UPROPERTY(config, EditAnywhere, Category="Data")
    FFilePath DecisionPointsJson;

    virtual FName GetCategoryName() const override { return FName("Project"); }
};
