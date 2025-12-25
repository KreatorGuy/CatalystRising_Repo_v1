#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SGStoryState.generated.h"

/**
 * Simple, engine-friendly story state store.
 *
 * Flags are stored as FName for easy interop with CSV/JSON.
 * If you prefer GameplayTags, you can mirror these into tags at runtime.
 */
USTRUCT(BlueprintType)
struct SGNARRATIVE_API FSGStoryState
{
	GENERATED_BODY()

	/** One-way flags toggled by decisions / quest progress. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Story")
	TSet<FName> Flags;

	/** Numeric state: reputation, ranks, trust, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Story")
	TMap<FName, int32> Ints;
};

/** Blueprint helpers for FSGStoryState. */
UCLASS()
class SGNARRATIVE_API USGStoryStateLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Shattered Gods|Story")
	static bool HasFlag(const FSGStoryState& State, FName Flag) { return State.Flags.Contains(Flag); }

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Story")
	static void AddFlag(UPARAM(ref) FSGStoryState& State, FName Flag) { State.Flags.Add(Flag); }

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Story")
	static void RemoveFlag(UPARAM(ref) FSGStoryState& State, FName Flag) { State.Flags.Remove(Flag); }

	UFUNCTION(BlueprintPure, Category="Shattered Gods|Story")
	static int32 GetInt(const FSGStoryState& State, FName Key, int32 DefaultValue = 0)
	{
		if (const int32* Found = State.Ints.Find(Key))
		{
			return *Found;
		}
		return DefaultValue;
	}

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Story")
	static void SetInt(UPARAM(ref) FSGStoryState& State, FName Key, int32 Value) { State.Ints.Add(Key, Value); }

	UFUNCTION(BlueprintCallable, Category="Shattered Gods|Story")
	static void AddInt(UPARAM(ref) FSGStoryState& State, FName Key, int32 Delta)
	{
		const int32 Current = GetInt(State, Key, 0);
		State.Ints.Add(Key, Current + Delta);
	}

	UFUNCTION(BlueprintPure, Category="Shattered Gods|Story")
	static FString ToDebugString(const FSGStoryState& State)
	{
		TArray<FName> SortedFlags = State.Flags.Array();
		SortedFlags.Sort(FNameLexicalLess());

		TArray<FName> Keys;
		State.Ints.GetKeys(Keys);
		Keys.Sort(FNameLexicalLess());

		FString Out;
		Out += TEXT("Flags:\n");
		for (const FName& F : SortedFlags)
		{
			Out += FString::Printf(TEXT("- %s\n"), *F.ToString());
		}

		Out += TEXT("Ints:\n");
		for (const FName& K : Keys)
		{
			Out += FString::Printf(TEXT("- %s = %d\n"), *K.ToString(), State.Ints[K]);
		}

		return Out;
	}
};
