///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cheat/CheatCommandCollection.h"

#include "Cheat/CheatCommand.h"

namespace Cheats
{
	TArray<FCheatCommandCollection*> FCheatCommandCollection::AllCollections;

	FCheatCommandCollection::FCheatCommandCollection()
	{
		AllCollections.AddUnique(this);
	}

	FCheatCommandCollection::FCheatCommandCollection(const FCheatMenuCategorySettings& InCheatMenuSettings) :
		CheatMenuSettings(InCheatMenuSettings)
	{
		AllCollections.AddUnique(this);
	}

	void FCheatCommandCollection::AddCheat(ICheatCommand* CheatCommand)
	{
		RegisteredCheatCommands.AddUnique(CheatCommand);
	}
}


DEFINE_CHEAT_COLLECTION(CheatCollectionCheats)
{
	DEFINE_CHEAT_COMMAND(WriteCheatsToFileCheat, "Cheat.WriteCheatListToFile")
	.DescribeCheat("Collects all cheats and writes them into a CSV file")
	.DescribeArgument<FString>("Target CSV File", "(Optional) Filepath + filename + .csv extension to save to")
	DEFINE_CHEAT_EXECUTE(WriteCheatsToFileCheat)
	{
		TArray<FString> Lines;
		Lines.Add("Cheat;Command;Arguments;Description"); // Header row.
		for (const FCheatCommandCollection* CheatCollection : FCheatCommandCollection::AllCollections)
		{
			for (const ICheatCommand* CheatCommand : CheatCollection->GetRegisteredCheatCommands())
			{
				const FString Arguments = FString::JoinBy(CheatCommand->GetArgumentsInfo(), TEXT(", "), &FArgumentInfo::ToString);
				Lines.Add(FString::Printf(TEXT("\"%s\";\"%s\";\"%s\";\"%s\""),
					*CheatCommand->GetDisplayName(),
					*CheatCommand->GetCommandName(),
					*Arguments,
					*CheatCommand->GetCommandInfo()));
			}
		}

		const FString FileName = GetNextArgumentOr<FString>(FPaths::ProjectSavedDir() / "Cheats.csv");
		if (FText Error; FFileHelper::IsFilenameValidForSaving(FileName, OUT Error) == false)
		{
			LogError("Filename not valid: " + FileName + "; Reason: " + Error.ToString());
			return;
		}

		if (FFileHelper::SaveStringArrayToFile(Lines, *FileName))
		{
			LogInfo("Cheat list was saved to file: " + FileName);
		}
		else
		{
			LogError("Cheat list could not be saved to file: " + FileName);
		}
	}
}
