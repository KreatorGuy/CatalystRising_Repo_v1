#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SGDialogueTypes.generated.h"

/**
 * Dialogue + decision table row.
 * Property names intentionally match the CSV headers for easy import.
 */
USTRUCT(BlueprintType)
struct SGNARRATIVE_API FSGDialogueDecisionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString quest;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString hub;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName id;

	/** DIALOGUE / DECISION / DECISION_OPTION */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString speaker;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString emotion;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString anim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString line_or_prompt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString option_key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString option_text;

	/** Stored as JSON-like strings in the CSV (ex: ["flag_a"], ["academy_clearance>=1"]) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString checks;

	/** Stored as JSON-like string array in the CSV */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString conditions;

	/** Stored as JSON-like string array in the CSV */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString set_flags;

	/** Stored as JSON-like string (ex: { "rep": { "rep_palace": "+1" }, "xp": 500 }) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString grants;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName next;
};

USTRUCT(BlueprintType)
struct SGNARRATIVE_API FSGCinematicShotRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString questline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString scene_id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 shot_no = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString framing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float lens_mm = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString cam_move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float duration_s = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString audio_notes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ui_notes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString references;
};

/** Main quest dialogue + optional prompts share the same schema. */
USTRUCT(BlueprintType)
struct SGNARRATIVE_API FSGMainQuestLineRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 seq = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName beat_id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString beat_desc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 pos = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString speaker_guess;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString dialogue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool optional = false;
};

/** Branch quest outline summary row (two primary branches). */
USTRUCT(BlueprintType)
struct SGNARRATIVE_API FSGBranchQuestSummaryRow : public FTableRowBase
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
	FString branch_1_name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString branch_1_variables;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString branch_1_rejoin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString branch_1_rewards;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString branch_2_name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString branch_2_variables;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString branch_2_rejoin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString branch_2_rewards;
};

/** Decision Points extracted from the Catalyst Rising decision-point PDF (v2). */
USTRUCT(BlueprintType)
struct SGNARRATIVE_API FSGDecisionPointRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString act;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString scene;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString dp_id;

	/** PROMPT or OPTION */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString row_type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString prompt_text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString option_key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString option_text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString immediate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString long_term;
};
