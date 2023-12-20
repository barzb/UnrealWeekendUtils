///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CheatCommand.h"
#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_ThreeParams(FOnCheatExecuted, const ICheatCommand&, UWorld*, TArray<FString>);

class WEEKENDUTILS_API SCheatMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCheatMenu) {}
		SLATE_ARGUMENT_DEFAULT(uint16, NumRecentlyUsedCheatsToShow) = 16;
		SLATE_EVENT(FOnCheatExecuted, OnCheatExecuted)
	SLATE_END_ARGS()

	virtual ~SCheatMenu() override;

	void Construct(const FArguments& InArgs);

protected:
	using EArgumentStyle = Cheats::EVariableStyle::Type;
	using FTabName = FName;
	using FSectionName = FName;

	struct FEntry
	{
		explicit FEntry(ICheatCommand* InCheatCommand, const FCheatMenuCategorySettings& InSettings);

		ICheatCommand* CheatCommand = nullptr;
		FCheatMenuCategorySettings Settings;
		TArray<TSharedPtr<FString>> Args;

		TArray<FString> GetArgs() const;
		const FString& GetCommandName() const;
		bool MatchesFilterText(const FString& TextToFilter) const;
		void ExecuteCheatCommand();
	};

	FTabName CurrentTabName = NAME_None;
	TArray<FString> FavoriteCheatCommands;
	TArray<FString> TextFilteredCheatCommands;
	TArray<FString> RecentlyUsedCheatCommands;
	uint16 NumRecentlyUsedCheatsToShow = 16;
	TOptional<FText> FilterText = {};

	TArray<TSharedPtr<FEntry>> Entries;
	TMap<FTabName, TArray<FSectionName>> SectionNamesInTabNames;

	TSharedPtr<SVerticalBox> TabList = nullptr;
	TSharedPtr<SWrapBox> SectionList = nullptr;
	TSharedPtr<STextBlock> ErrorText = nullptr;

	FOnCheatExecuted OnCheatExecuted;

	void CollectCheats();
	void PopulateTabList();
	void RefreshTabContent();
	void ConstructCommandsSection(const FSectionName& SectionName, const TArray<TSharedPtr<FEntry>>& CheatCommands);

	TArray<TSharedPtr<FEntry>> FilterCommands(const FTabName& TabName, const FSectionName& SectionName) const;
	TArray<TSharedPtr<FEntry>> FilterCommands(const TArray<FString>& CheatCommands) const;

	TSharedRef<SWidget> ConstructSection(const FSectionName& SectionName, TSharedRef<SWidget> Content);
	TSharedRef<SWidget> ConstructCheatCommandItems(const TArray<TSharedPtr<FEntry>>& CheatCommands);
	TSharedRef<SWidget> ConstructArgumentInput(const ICheatCommand::FArgumentInfo& ArgumentInfo, TSharedPtr<FString> InOutValue);

	void HandleFilterTextChanged(const FText& NewFilterText);
	void HandleCheatLogMessage(const ICheatCommand& CheatCommand, ELogVerbosity::Type Verbosity, const FString& Message);
	void HandleCheatExecuted(const ICheatCommand& CheatCommand, UWorld* World, TArray<FString> Args);
	FReply HandleCheatFavoriteButtonClicked(FString CheatName);

	void InsertCheatIntoRecentlyUsedList(const FString& CheatName);
	void ToggleCheatForFavorites(const FString& CheatName);

	void SaveFavoriteAndRecentlyUsedCheats() const;
	void RestoreFavoriteAndRecentlyUsedCheats();
};
