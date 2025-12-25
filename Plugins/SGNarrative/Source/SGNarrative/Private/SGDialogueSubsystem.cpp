#include "SGDialogueSubsystem.h"
#include "SGNarrativeSettings.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void USGDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Reload();
}

void USGDialogueSubsystem::Reload()
{
	DialogueDecisionTable = nullptr;
	RowsById.Reset();

	const USGNarrativeSettings* Settings = GetDefault<USGNarrativeSettings>();
	if (Settings && Settings->DialogueDecisionTable.IsValid() == false)
	{
		// Try to load synchronously if a path was set.
		DialogueDecisionTable = Settings->DialogueDecisionTable.LoadSynchronous();
	}
	else if (Settings)
	{
		DialogueDecisionTable = Settings->DialogueDecisionTable.Get();
	}

	BuildIndex();
}

void USGDialogueSubsystem::BuildIndex()
{
	RowsById.Reset();

	if (!DialogueDecisionTable)
	{
		return;
	}

	static const FString Context = TEXT("USGDialogueSubsystem::BuildIndex");
	TArray<FSGDialogueDecisionRow*> AllRows;
	DialogueDecisionTable->GetAllRows(Context, AllRows);

	for (const FSGDialogueDecisionRow* Row : AllRows)
	{
		if (!Row)
		{
			continue;
		}

		TArray<FSGDialogueDecisionRow>& Bucket = RowsById.FindOrAdd(Row->id);
		Bucket.Add(*Row);
	}
}

bool USGDialogueSubsystem::GetRowsByNarrativeId(FName Id, TArray<FSGDialogueDecisionRow>& OutRows) const
{
	OutRows.Reset();

	if (const TArray<FSGDialogueDecisionRow>* Found = RowsById.Find(Id))
	{
		OutRows = *Found;
		return OutRows.Num() > 0;
	}

	return false;
}

bool USGDialogueSubsystem::GetFirstRowByNarrativeId(FName Id, FSGDialogueDecisionRow& OutRow) const
{
	TArray<FSGDialogueDecisionRow> Rows;
	if (!GetRowsByNarrativeId(Id, Rows))
	{
		return false;
	}

	// Prefer a non-option row for the "node".
	for (const FSGDialogueDecisionRow& R : Rows)
	{
		if (!R.type.Equals(TEXT("DECISION_OPTION"), ESearchCase::IgnoreCase))
		{
			OutRow = R;
			return true;
		}
	}

	// Fallback: first row.
	OutRow = Rows[0];
	return true;
}

bool USGDialogueSubsystem::GetDecisionOptions(FName DecisionId, const FSGStoryState& State, TArray<FSGDialogueDecisionRow>& OutOptions) const
{
	OutOptions.Reset();

	TArray<FSGDialogueDecisionRow> Rows;
	if (!GetRowsByNarrativeId(DecisionId, Rows))
	{
		return false;
	}

	for (const FSGDialogueDecisionRow& Row : Rows)
	{
		if (!Row.type.Equals(TEXT("DECISION_OPTION"), ESearchCase::IgnoreCase))
		{
			continue;
		}

		if (!AreConditionsMet(Row, State))
		{
			continue;
		}

		if (!AreChecksMet(Row, State))
		{
			continue;
		}

		OutOptions.Add(Row);
	}

	return OutOptions.Num() > 0;
}

bool USGDialogueSubsystem::AreConditionsMet(const FSGDialogueDecisionRow& Row, const FSGStoryState& State) const
{
	TArray<FString> Conds;
	ParseStringArrayJson(Row.conditions, Conds);

	for (const FString& Raw : Conds)
	{
		const FString Trim = Raw.TrimStartAndEnd();
		if (Trim.IsEmpty())
		{
			continue;
		}

		const bool bNegated = Trim.StartsWith(TEXT("!"));
		const FString FlagStr = bNegated ? Trim.Mid(1) : Trim;
		const FName Flag(*FlagStr);

		const bool bHas = State.Flags.Contains(Flag);
		if (!bNegated && !bHas)
		{
			return false;
		}
		if (bNegated && bHas)
		{
			return false;
		}
	}

	return true;
}

bool USGDialogueSubsystem::AreChecksMet(const FSGDialogueDecisionRow& Row, const FSGStoryState& State) const
{
	TArray<FString> Checks;
	ParseStringArrayJson(Row.checks, Checks);

	for (const FString& Expr : Checks)
	{
		if (!EvalSimpleCheck(Expr, State))
		{
			return false;
		}
	}

	return true;
}

void USGDialogueSubsystem::ApplyRowEffects(const FSGDialogueDecisionRow& Row, FSGStoryState& State) const
{
	// 1) Set flags
	{
		TArray<FString> Flags;
		ParseStringArrayJson(Row.set_flags, Flags);
		for (const FString& F : Flags)
		{
			const FString Trim = F.TrimStartAndEnd();
			if (!Trim.IsEmpty())
			{
				State.Flags.Add(FName(*Trim));
			}
		}
	}

	// 2) Grants JSON object
	const FString GrantsTrim = Row.grants.TrimStartAndEnd();
	if (GrantsTrim.IsEmpty() || GrantsTrim == TEXT("{}"))
	{
		return;
	}

	TSharedPtr<FJsonObject> RootObj;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(GrantsTrim);
	if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
	{
		// If parsing fails, we intentionally do nothing.
		return;
	}

	auto ApplyDeltaObject = [&](const TCHAR* FieldName)
	{
		const TSharedPtr<FJsonObject>* ObjPtr = nullptr;
		if (RootObj->TryGetObjectField(FieldName, ObjPtr) && ObjPtr && ObjPtr->IsValid())
		{
			for (const auto& Pair : (*ObjPtr)->Values)
			{
				const FString Key = Pair.Key;
				const FString DeltaStr = Pair.Value.IsValid() ? Pair.Value->AsString() : TEXT("0");
				const int32 Delta = ParseDelta(DeltaStr);

				const FName KName(*Key);
				const int32 Current = State.Ints.Contains(KName) ? State.Ints[KName] : 0;
				State.Ints.Add(KName, Current + Delta);
			}
		}
	};

	ApplyDeltaObject(TEXT("rep"));
	ApplyDeltaObject(TEXT("trust"));

	// xp: number or string
	if (RootObj->HasField(TEXT("xp")))
	{
		int32 XpDelta = 0;
		const TSharedPtr<FJsonValue> V = RootObj->TryGetField(TEXT("xp"));
		if (V.IsValid())
		{
			if (V->Type == EJson::Number)
			{
				XpDelta = (int32)V->AsNumber();
			}
			else if (V->Type == EJson::String)
			{
				XpDelta = ParseDelta(V->AsString());
			}
		}

		if (XpDelta != 0)
		{
			const FName Key(TEXT("xp"));
			const int32 Current = State.Ints.Contains(Key) ? State.Ints[Key] : 0;
			State.Ints.Add(Key, Current + XpDelta);
		}
	}

	// item: string or array. We store as counter ints: item_<name> += 1
	if (RootObj->HasField(TEXT("item")))
	{
		const TSharedPtr<FJsonValue> V = RootObj->TryGetField(TEXT("item"));
		if (V.IsValid())
		{
			auto AddItemCounter = [&](const FString& ItemName)
			{
				const FString TrimItem = ItemName.TrimStartAndEnd();
				if (TrimItem.IsEmpty()) return;

				const FString KeyStr = FString::Printf(TEXT("item_%s"), *TrimItem);
				const FName Key(*KeyStr);
				const int32 Current = State.Ints.Contains(Key) ? State.Ints[Key] : 0;
				State.Ints.Add(Key, Current + 1);
			};

			if (V->Type == EJson::String)
			{
				AddItemCounter(V->AsString());
			}
			else if (V->Type == EJson::Array)
			{
				const TArray<TSharedPtr<FJsonValue>> Arr = V->AsArray();
				for (const TSharedPtr<FJsonValue>& Elem : Arr)
				{
					if (Elem.IsValid())
					{
						AddItemCounter(Elem->AsString());
					}
				}
			}
		}
	}

	// Any other numeric top-level keys: mirror into Ints.
	for (const auto& Pair : RootObj->Values)
	{
		const FString Key = Pair.Key;
		if (Key == TEXT("rep") || Key == TEXT("trust") || Key == TEXT("xp") || Key == TEXT("item"))
		{
			continue;
		}

		const TSharedPtr<FJsonValue> V = Pair.Value;
		if (!V.IsValid()) continue;

		if (V->Type == EJson::Number)
		{
			const int32 Delta = (int32)V->AsNumber();
			const FName KName(*Key);
			const int32 Current = State.Ints.Contains(KName) ? State.Ints[KName] : 0;
			State.Ints.Add(KName, Current + Delta);
		}
		else if (V->Type == EJson::String)
		{
			const int32 Delta = ParseDelta(V->AsString());
			if (Delta != 0)
			{
				const FName KName(*Key);
				const int32 Current = State.Ints.Contains(KName) ? State.Ints[KName] : 0;
				State.Ints.Add(KName, Current + Delta);
			}
		}
	}
}

bool USGDialogueSubsystem::ApplyRowAndGetNext(const FSGDialogueDecisionRow& Row, FSGStoryState& State, FName& OutNextId) const
{
	ApplyRowEffects(Row, State);
	OutNextId = Row.next;
	return !OutNextId.IsNone();
}

void USGDialogueSubsystem::ParseStringArrayJson(const FString& JsonLikeArray, TArray<FString>& Out)
{
	Out.Reset();

	const FString Trim = JsonLikeArray.TrimStartAndEnd();
	if (Trim.IsEmpty() || Trim == TEXT("[]"))
	{
		return;
	}

	// First try: strict JSON parse
	{
		TSharedPtr<FJsonValue> RootValue;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Trim);
		if (FJsonSerializer::Deserialize(Reader, RootValue) && RootValue.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
			if (RootValue->TryGetArray(Arr) && Arr)
			{
				for (const TSharedPtr<FJsonValue>& V : *Arr)
				{
					if (V.IsValid())
					{
						Out.Add(V->AsString());
					}
				}
				return;
			}
		}
	}

	// Fallback: naive split for mildly broken arrays
	FString Inner = Trim;
	Inner.RemoveFromStart(TEXT("["));
	Inner.RemoveFromEnd(TEXT("]"));

	TArray<FString> Parts;
	Inner.ParseIntoArray(Parts, TEXT(","), true);
	for (FString P : Parts)
	{
		P = P.TrimStartAndEnd();
		P = P.Replace(TEXT("\""), TEXT(""));
		if (!P.IsEmpty())
		{
			Out.Add(P);
		}
	}
}

bool USGDialogueSubsystem::EvalSimpleCheck(const FString& Expr, const FSGStoryState& State)
{
	// Supports: key>=N, key<=N, key==N, key!=N, key>N, key<N
	FString Key, Op, NumStr;
	const FString E = Expr.TrimStartAndEnd();

	auto Extract = [&](const FString& InOp) -> bool
	{
		const int32 Idx = E.Find(InOp);
		if (Idx == INDEX_NONE) return false;
		Key = E.Left(Idx).TrimStartAndEnd();
		Op = InOp;
		NumStr = E.Mid(Idx + InOp.Len()).TrimStartAndEnd();
		return true;
	};

	if (!(Extract(TEXT(">=")) || Extract(TEXT("<=")) || Extract(TEXT("==")) || Extract(TEXT("!=")) || Extract(TEXT(">")) || Extract(TEXT("<"))))
	{
		// Unknown expression; be permissive by default.
		return true;
	}

	const int32 N = ParseDelta(NumStr);
	const FName KName(*Key);
	const int32 Current = State.Ints.Contains(KName) ? State.Ints[KName] : 0;

	if (Op == TEXT(">=")) return Current >= N;
	if (Op == TEXT("<=")) return Current <= N;
	if (Op == TEXT("==")) return Current == N;
	if (Op == TEXT("!=")) return Current != N;
	if (Op == TEXT(">")) return Current > N;
	if (Op == TEXT("<")) return Current < N;

	return true;
}

int32 USGDialogueSubsystem::ParseDelta(const FString& DeltaStr)
{
	FString S = DeltaStr.TrimStartAndEnd();
	S = S.Replace(TEXT("−"), TEXT("-")); // unicode minus
	return FCString::Atoi(*S);
}
