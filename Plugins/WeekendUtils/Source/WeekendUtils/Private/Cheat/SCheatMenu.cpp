///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cheat/SCheatMenu.h"

#include "Styling/StyleColors.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

///////////////////////////////////////////////////////////////////////////////////////
/// Cheats:

DEFINE_CHEAT_COLLECTION(CheatMenuCheats)
{
	DEFINE_CHEAT_COMMAND(OpenCheatMenuCheat, "Cheat.OpenCheatMenu")
	.DescribeCheat("Opens the cheat menu in a new window")
	DEFINE_CHEAT_EXECUTE(OpenCheatMenuCheat)
	{
		static TSharedPtr<SWindow> SlateWindow = nullptr;

		// Window already open somewhere:
		if (SlateWindow.IsValid() && SlateWindow->IsActive())
		{
			SlateWindow->BringToFront();
			return;
		}

		// Create new window:
		SlateWindow = SNew(SWindow)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.ClientSize(FVector2d(1280.f, 720.f))
		.CreateTitleBar(true)
		.HasCloseButton(true)
		.IsInitiallyMaximized(false)
		.MinHeight(64.f)
		.MinWidth(128.f)
		.SaneWindowPlacement(true)
		.SizingRule(ESizingRule::UserSized)
		.Style(&FCoreStyle::Get().GetWidgetStyle<FWindowStyle>("Window"))
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		.Title(INVTEXT("Cheat Menu"))
		.UseOSWindowBorder(false);
		SlateWindow->SetContent(SNew(SCheatMenu));

		// Open the window:
		FSlateApplication& SlateApp = FSlateApplication::Get();
		SlateApp.AddWindow(SlateWindow.ToSharedRef());
		SlateApp.GetRenderer()->CreateViewport(SlateWindow.ToSharedRef());
		SlateApp.OnPreShutdown().AddLambda([]
		{
			if (SlateWindow.IsValid())
			{
				FSlateApplication::Get().RequestDestroyWindow(SlateWindow.ToSharedRef());
				SlateWindow.Reset();
			}
		});
	}
}

///////////////////////////////////////////////////////////////////////////////////////
/// Internal Helpers:

namespace
{
	const FName TAB_NAME_NONE = FName();
	const FName SECTION_NAME_NONE = FName();
	const FName FAVORITE_TAB_NAME = FName("Favorites");
	const FName RECENTLY_USED_TAB_NAME = FName("RecentlyUsed");
	const FName FILTER_RESULT_TAB_NAME = FName("Filtered");
	const FName DEFAULT_TAB_NAME = FAVORITE_TAB_NAME;

	const FString CHEAT_MENU_INI_FILE = GGameUserSettingsIni;
	const TCHAR* CHEAT_MENU_INI_SECTION = TEXT("WeekendUtils.CheatMenu");
	const TCHAR* CHEAT_MENU_INI_FAVORITES = TEXT("FavoritedCheats");
	const TCHAR* CHEAT_MENU_INI_RECENTLY_USED = TEXT("RecentlyUsedCheats");

	const FMargin DEFAULT_PADDING = FMargin(5.f);

	struct FSortMenuEntryPredicate
	{
		FORCEINLINE bool operator()(const FName& A, const FName& B) const
		{
			return (A.IsNone() || A.Compare(B) < 0);
		}
	};

	UWorld* FindPlayWorld()
	{
		// Determine the target world at the moment of execution,
		// because the cheat menu might've been created in another level or even the editor
		// before PIE, so it can't know which world is "current".
		// The following approach is also how the engine does it for console commands:
		UWorld* TargetWorld = nullptr;
		if (const ULocalPlayer* Player = GEngine->GetDebugLocalPlayer(); IsValid(Player))
		{
			TargetWorld = Player->GetWorld();
		}
		return GEngine->GetCurrentPlayWorld(TargetWorld);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
/// SCheatMenu:

void SCheatMenu::Construct(const FArguments& InArgs)
{
	CurrentTabName = DEFAULT_TAB_NAME;
	NumRecentlyUsedCheatsToShow = InArgs._NumRecentlyUsedCheatsToShow;
	OnCheatExecuted = InArgs._OnCheatExecuted;

	TSharedRef<SHorizontalBox> MainContent = SNew(SHorizontalBox);

	// [Left] Tab List:
	MainContent->AddSlot()
	.AutoWidth()
	.Padding(DEFAULT_PADDING)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SScrollBox)
		.ScrollBarAlwaysVisible(true)
		.ScrollBarThickness(FVector2d(5.f))

		+SScrollBox::Slot()
		.Padding(DEFAULT_PADDING)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(TabList, SVerticalBox)

			// [Left|Top] Search Box:
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 5.f)
			[
				SNew(SSearchBox)
				.ToolTipText(INVTEXT("Filter Cheats"))
				.MinDesiredWidth(96.f)
				.DelayChangeNotificationsWhileTyping(true)
				.OnTextChanged(this, &SCheatMenu::HandleFilterTextChanged)
				.OnTextCommitted_Lambda([this](const FText& NewFilterText, ETextCommit::Type){ HandleFilterTextChanged(NewFilterText); })
			]
		]
	];

	// [Right] Current Tab Content:
	MainContent->AddSlot()
	.FillWidth(1.f)
	.Padding(DEFAULT_PADDING)
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)

		// [Right|Top] Content:
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(SectionList, SWrapBox)
				.HAlign(HAlign_Fill)
				.Orientation(Orient_Horizontal)
			]
		]

		// [Right|Bottom] Error Bar:
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		[
			SNew(SOverlay)

			// Background:
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			]

			// Output Box:
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(5.f)
			[
				SNew(SHorizontalBox)

				// Text:
				+SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					SAssignNew(ErrorText, STextBlock)
					.Justification(ETextJustify::Left)
				]

				// Clear Button:
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.ToolTipText(INVTEXT("Clear Output"))
					.IsEnabled_Lambda([this](){ return !ErrorText->GetText().IsEmpty(); })
					.OnClicked_Lambda([this]()
					{
						ErrorText->SetText(FText::GetEmpty());
						return FReply::Handled();
					})
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Icons.Delete"))
					]
				]
			]
		]
	];

	// Populate:
	CollectCheats();
	PopulateTabList();
	RefreshTabContent();

	// Add the sub-widgets to the ChildSlot of this compound widget:
	ChildSlot
	[
		MainContent
	];
}

SCheatMenu::FEntry::FEntry(ICheatCommand* InCheatCommand, const FCheatMenuCategorySettings& InSettings) :
	CheatCommand(InCheatCommand), Settings(InSettings)
{
	const int32 NumArgs = CheatCommand->GetArgumentsInfo().Num();
	TArray<FString> ArgValues;

	// Try to load arg values from config ini, if they have already been executed:
	if (GConfig != nullptr)
	{
		//FString ArgValuesFromCfg;
		GConfig->GetSingleLineArray(CHEAT_MENU_INI_SECTION, *GetCommandName(), OUT ArgValues, CHEAT_MENU_INI_FILE);
		//ArgValuesFromCfg.ParseIntoArray(OUT ArgValues, TEXT(","));
	}

	// Make sure the value array size is in sync with the actual args array:
	ArgValues.SetNum(NumArgs);

	// Construct the args with possibly some default values:
	Args.Empty(NumArgs);
	for (const FString& ArgValue : ArgValues)
	{
		Args.Add(MakeShared<FString>(ArgValue));
	}
}

SCheatMenu::~SCheatMenu()
{
	for (const TSharedPtr<FEntry>& Entry : Entries)
	{
		Entry->CheatCommand->OnLogMessage.RemoveAll(this);
		Entry->CheatCommand->OnAfterExecuted.RemoveAll(this);
	}
	Entries.Empty();
	TabList.Reset();
	SectionList.Reset();
	ErrorText.Reset();
}

TArray<FString> SCheatMenu::FEntry::GetArgs() const
{
	TArray<FString> Result;
	Algo::Transform(Args, OUT Result, [](const TSharedPtr<FString>& Elem){ return *Elem.Get(); });
	return Result;
}

const FString& SCheatMenu::FEntry::GetCommandName() const
{
	return CheatCommand->GetCommandName();
}

bool SCheatMenu::FEntry::MatchesFilterText(const FString& TextToFilter) const
{
	return CheatCommand->GetCommandName().Contains(TextToFilter)
		|| CheatCommand->GetDisplayName().Contains(TextToFilter);
}

void SCheatMenu::FEntry::ExecuteCheatCommand()
{
	CheatCommand->Execute(GetArgs(), FindPlayWorld());

	// Save the args to the config ini, so they can be restored the next time the cheat menu opens:
	if (GConfig != nullptr)
	{
		GConfig->SetSingleLineArray(CHEAT_MENU_INI_SECTION, *GetCommandName(), GetArgs(), CHEAT_MENU_INI_FILE);
		GConfig->Flush(false, CHEAT_MENU_INI_FILE);
	}
}

void SCheatMenu::CollectCheats()
{
	const UWorld* CurrentWorld = FindPlayWorld();

	// Clear previous:
	for (const TSharedPtr<FEntry>& Entry : Entries)
	{
		Entry->CheatCommand->OnLogMessage.RemoveAll(this);
		Entry->CheatCommand->OnAfterExecuted.RemoveAll(this);
	}
	Entries.Empty();
	SectionNamesInTabNames.Empty();

	// Collect anew:
	for (const Cheats::FCheatCommandCollection* Collection : Cheats::FCheatCommandCollection::AllCollections)
	{
		if (!Collection->ShowInCheatMenu())
			continue;

		const FCheatMenuCategorySettings& Settings = Collection->GetCheatMenuSettings();
		const FName& Section = Settings.MenuTabName.Get(TAB_NAME_NONE);
		const FName& Tab = Settings.MenuSectionName.Get(SECTION_NAME_NONE);
		SectionNamesInTabNames.FindOrAdd(Section).AddUnique(Tab);

		for (ICheatCommand* CheatCommand : Collection->GetRegisteredCheatCommands())
		{
			Entries.Add(MakeShared<FEntry>(CheatCommand, Settings));
			CheatCommand->OnLogMessage.AddSP(this, &SCheatMenu::HandleCheatLogMessage);
			CheatCommand->OnAfterExecuted.AddSP(this, &SCheatMenu::HandleCheatExecuted);

			for (ICheatCommand* Variant : CheatCommand->GetVariants(CurrentWorld))
			{
				Entries.Add(MakeShared<FEntry>(Variant, Settings));
			}
		}
	}

	// Sort tabs and sections by name:
	SectionNamesInTabNames.KeySort(FSortMenuEntryPredicate());
	for (TTuple<FTabName, TArray<FSectionName>>& Pair : SectionNamesInTabNames)
	{
		Pair.Value.Sort(FSortMenuEntryPredicate());
	}

	RestoreFavoriteAndRecentlyUsedCheats();
}

TArray<TSharedPtr<SCheatMenu::FEntry>> SCheatMenu::FilterCommands(const FTabName& TabName, const FSectionName& SectionName) const
{
	return Entries.FilterByPredicate([TabName, SectionName](const TSharedPtr<FEntry>& Entry)
	{
		return Entry->Settings.MenuTabName.Get(TAB_NAME_NONE).IsEqual(TabName)
			&& Entry->Settings.MenuSectionName.Get(SECTION_NAME_NONE).IsEqual(SectionName);
	});
}

TArray<TSharedPtr<SCheatMenu::FEntry>> SCheatMenu::FilterCommands(const TArray<FString>& CheatCommands) const
{
	TArray<TSharedPtr<FEntry>> Result = Entries.FilterByPredicate([&CheatCommands](const TSharedPtr<FEntry>& Entry)
	{
		return CheatCommands.Contains(Entry->GetCommandName());
	});
	Result.Sort([&CheatCommands](const TSharedPtr<FEntry>& Lhs, const TSharedPtr<FEntry>& Rhs)
	{
		return CheatCommands.IndexOfByKey(Lhs->GetCommandName()) < CheatCommands.IndexOfByKey(Rhs->GetCommandName());
	});
	return Result;
}

void SCheatMenu::PopulateTabList()
{
	struct FTabProperties
	{
		explicit FTabProperties(FName InTabName) :
			TabName(InTabName), Text(FText::Format(INVTEXT("📃 {0}"), FText::FromName(InTabName))) {}
		explicit FTabProperties(FName InTabName,const FText& InText, const FText& InToolTip) :
			TabName(InTabName), Text(InText), ToolTip(InToolTip) {}

		FName TabName;
		FText Text;
		FText ToolTip;
	};

	TArray<FTabProperties> Tabs;
	Algo::Transform(SectionNamesInTabNames, OUT Tabs, [](const auto& Itr){ return FTabProperties(Itr.Key); });
	Tabs.Insert(FTabProperties(FAVORITE_TAB_NAME, INVTEXT("⭐ Favorites"), INVTEXT("Favorited Cheats")), 0);
	Tabs.Insert(FTabProperties(RECENTLY_USED_TAB_NAME, INVTEXT("🕙 Recently Used"), INVTEXT("Recently Used Cheats")), 1);

	for (const FTabProperties& Tab : Tabs)
	{
		TabList->AddSlot()
		.HAlign(HAlign_Fill)
		.AutoHeight()
		[
			SNew(SButton)
			.ButtonColorAndOpacity_Lambda([this, TabName = Tab.TabName]()
			{
				return (CurrentTabName == TabName ? FStyleColors::Black : FStyleColors::White);
			})
			.OnClicked_Lambda([this, Tab]() -> FReply
			{
				FilterText.Reset();
				CurrentTabName = Tab.TabName;
				RefreshTabContent();
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Left)
				.Text(FText::Format(INVTEXT("{0}    "), Tab.Text)) // Hack to compensate width offset from scroll bar.
				.ToolTipText(Tab.ToolTip)
			]
		];
	}
}

void SCheatMenu::RefreshTabContent()
{
	SectionList->ClearChildren();
	if (!CurrentTabName.IsValid())
		return;

	if (SectionNamesInTabNames.Contains(CurrentTabName))
	{
		for (const FSectionName& SectionName : SectionNamesInTabNames[CurrentTabName])
		{
			ConstructCommandsSection(SectionName, FilterCommands(CurrentTabName, SectionName));
		}
	}
	else if (CurrentTabName == FAVORITE_TAB_NAME)
	{
		ConstructCommandsSection(NAME_None, FilterCommands(FavoriteCheatCommands));
	}
	else if (CurrentTabName == RECENTLY_USED_TAB_NAME)
	{
		ConstructCommandsSection(NAME_None, FilterCommands(RecentlyUsedCheatCommands));
	}
	else if (CurrentTabName == FILTER_RESULT_TAB_NAME)
	{
		ConstructCommandsSection(NAME_None, FilterCommands(TextFilteredCheatCommands));
	}
}

void SCheatMenu::ConstructCommandsSection(const FSectionName& SectionName, const TArray<TSharedPtr<FEntry>>& CheatCommands)
{
	SectionList->AddSlot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.FillEmptySpace(true)
	[
		ConstructSection(SectionName, ConstructCheatCommandItems(CheatCommands))
	];
}

TSharedRef<SWidget> SCheatMenu::ConstructSection(const FSectionName& SectionName, TSharedRef<SWidget> Content)
{
	TSharedRef<SVerticalBox> Section = SNew(SVerticalBox);

	// Section Header:
	if (!SectionName.IsNone())
	{
		Section->AddSlot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.AutoHeight()
		[
			SNew(SOverlay)

			// Background:
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			]

			// Header:
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(5.f)
			[
				SNew(STextBlock)
				.Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
				.Text(FText::FromString(SectionName.ToString()))
				.ColorAndOpacity(EStyleColor::AccentWhite)
				.Justification(ETextJustify::Center)
				.Margin(FMargin(10.f, -2.f))
			]
		];
	}

	// Section Content:
	Section->AddSlot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.AutoHeight()
	[
		SNew(SOverlay)

		// Background:
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		]

		// Commands List:
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(5.f)
		[
			Content
		]
	];

	return SNew(SBox)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.MinDesiredWidth(256.f)
	.Padding(DEFAULT_PADDING)
	[
		Section
	];
}

TSharedRef<SWidget> SCheatMenu::ConstructCheatCommandItems(const TArray<TSharedPtr<FEntry>>& CheatCommands)
{
	TSharedRef<SWrapBox> OuterWrapBox =
		SNew(SWrapBox)
		.HAlign(HAlign_Left)
		.Orientation(Orient_Horizontal)
		.InnerSlotPadding(FVector2d(10.f))
		.UseAllottedSize(true);

	for (auto Itr = CheatCommands.CreateConstIterator(); Itr; ++Itr)
	{
		TSharedPtr<FEntry> Entry = *Itr;
		const ICheatCommand* CheatCommand = Entry->CheatCommand;
		const FString CheatName = CheatCommand->GetCommandName();
		TSharedPtr<SVerticalBox> CommandBox;

		OuterWrapBox->AddSlot()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Left)
		.Padding(DEFAULT_PADDING)
		[
			SAssignNew(CommandBox, SVerticalBox)

			// Execution Button + Fav Icon:
			+SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)

				// Favorite Star:
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.ButtonStyle(FAppStyle::Get(), "NoBorder")
					.ContentPadding(0.f)
					.ToolTipText_Lambda([this, CheatName]()
					{
						return FavoriteCheatCommands.Contains(CheatName) ?
							INVTEXT("Remove from favorites") : INVTEXT("Add to favorites");
					})
					.OnClicked(this, &SCheatMenu::HandleCheatFavoriteButtonClicked, CheatName)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Font(FAppStyle::Get().GetFontStyle("NormalBold"))
						.ColorAndOpacity_Lambda([this, CheatName]()
						{
							return FavoriteCheatCommands.Contains(CheatName) ?
								EStyleColor::AccentYellow : EStyleColor::AccentGray;
						})
						.Text_Lambda([this, CheatName]()
						{
							return FavoriteCheatCommands.Contains(CheatName) ?
								INVTEXT("★") : INVTEXT("☆");
						})
					]
				]

				// Execution Button:
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.HAlign(HAlign_Fill)
				[
					SNew(SButton)
					.OnClicked_Lambda([Entry]() -> FReply
					{
						Entry->ExecuteCheatCommand();
						return FReply::Handled();
					})
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString(CheatCommand->GetDisplayName()))
						.ToolTipText(FText::FromString(CheatCommand->GetCommandInfo()))
						.HighlightText_Lambda([this](){ return FilterText.Get(FText()); })
					]
				]
			]
		];

		// Arguments:
		auto ArgValueItr = Entry->Args.CreateIterator();
		for (const ICheatCommand::FArgumentInfo& ArgumentInfo : CheatCommand->GetArgumentsInfo())
		{
			TSharedPtr<FString>& ArgValue = *ArgValueItr; ++ArgValueItr;
			const bool bIsText = (ArgumentInfo.Style == EArgumentStyle::Text);

			CommandBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(5.f, 0.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)

				// Argument Name:
				+ SHorizontalBox::Slot()
				.FillWidth(3.f)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ArgumentInfo.Name + ": "))
					.Justification(ETextJustify::Right)
					.ToolTipText(FText::FromString(ArgumentInfo.Description))
				]

				// Argument Input:
				+ SHorizontalBox::Slot()
				.FillWidth(2.f)
				.MaxWidth(bIsText ? 256.f : 64.f)
				.HAlign(bIsText ? HAlign_Fill : HAlign_Left)
				.VAlign(VAlign_Center)
				[
					ConstructArgumentInput(ArgumentInfo, IN OUT ArgValue)
				]
			];
		}
	}
	return OuterWrapBox;
}

TSharedRef<SWidget> SCheatMenu::ConstructArgumentInput(const ICheatCommand::FArgumentInfo& ArgumentInfo, TSharedPtr<FString> InOutValue)
{
	static constexpr float MinDesiredWith = 64.f;
	switch (ArgumentInfo.Style)
	{
		case EArgumentStyle::Number:
		{
			int32 DefaultValue = 0;
			LexFromString(OUT DefaultValue, **InOutValue);
			*InOutValue = FString::FromInt(DefaultValue);
			return SNew(SSpinBox<int32>)
			.Delta(1)
			.MinDesiredWidth(MinDesiredWith)
			.ToolTipText(FText::FromString(ArgumentInfo.Description))
			.Value(DefaultValue)
			.OnValueCommitted_Lambda([InOutValue](int32 Value, ETextCommit::Type) { *InOutValue = FString::FromInt(Value); });
		}
		case EArgumentStyle::FloatNumber:
		{
			float DefaultValue = 0.f;
			LexFromString(OUT DefaultValue, **InOutValue);
			*InOutValue = LexToString(DefaultValue);
			return SNew(SSpinBox<float>)
			.Delta(0.1f)
			.MinDesiredWidth(MinDesiredWith)
			.ToolTipText(FText::FromString(ArgumentInfo.Description))
			.Value(DefaultValue)
			.OnValueCommitted_Lambda([InOutValue](float Value, ETextCommit::Type) { *InOutValue = LexToString(Value); });
		}
		case EArgumentStyle::TrueFalse:
		{
			bool bDefaultValue = false;
			LexFromString(OUT bDefaultValue, **InOutValue);
			*InOutValue = LexToString(bDefaultValue);
			return SNew(SCheckBox)
			.ToolTipText(FText::FromString(ArgumentInfo.Description))
			.IsChecked(bDefaultValue ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([InOutValue](ECheckBoxState State) { *InOutValue = (State == ECheckBoxState::Checked ? "1" : "0"); });
		}
		default: case EArgumentStyle::Text:
		{
			const FString DefaultValue = *InOutValue;
			return SNew(SEditableTextBox)
			.MinDesiredWidth(MinDesiredWith * 2.f)
			.ToolTipText(FText::FromString(ArgumentInfo.Description))
			.Text(FText::FromString(DefaultValue))
			.OnTextCommitted_Lambda([InOutValue](const FText& Text, ETextCommit::Type) { *InOutValue = Text.ToString(); });
		}
	}
}

void SCheatMenu::HandleFilterTextChanged(const FText& NewFilterText)
{
	TextFilteredCheatCommands.Empty();
	if (NewFilterText.IsEmpty())
	{
		FilterText.Reset();
		CurrentTabName = DEFAULT_TAB_NAME;
	}
	else
	{
		FilterText = NewFilterText;
		CurrentTabName = FILTER_RESULT_TAB_NAME;

		Algo::TransformIf(Entries, OUT TextFilteredCheatCommands,
			/*Condition*/ [NewFilterText](const TSharedPtr<FEntry>& Entry){ return Entry->MatchesFilterText(NewFilterText.ToString()); },
			/*Transform*/ [](const TSharedPtr<FEntry>& Entry){ return Entry->GetCommandName(); });
	}

	RefreshTabContent();
}

void SCheatMenu::HandleCheatLogMessage(const ICheatCommand& Command, ELogVerbosity::Type Verbosity, const FString& Message)
{
	EStyleColor Color;
	FText PrefixIcon = FText();
	switch (Verbosity)
	{
		case ELogVerbosity::Fatal:
		case ELogVerbosity::Error:
			Color = EStyleColor::AccentRed;
			PrefixIcon = INVTEXT("⛔ ");
			break;

		case ELogVerbosity::Warning:
			Color = EStyleColor::AccentYellow;
			PrefixIcon = INVTEXT("⚠ ");
			break;

		default: // (i) Do not show normie logs in the error/warning box.
			ErrorText->SetText(FText());
			return;
	}

	ErrorText->SetText(FText::Format(INVTEXT("{0}[{1}] {2}"), PrefixIcon, FText::FromString(Command.GetCommandName()), FText::FromString(Message)));
	ErrorText->SetColorAndOpacity(Color);
}

void SCheatMenu::HandleCheatExecuted(const ICheatCommand& CheatCommand, UWorld* World, TArray<FString> Args)
{
	InsertCheatIntoRecentlyUsedList(CheatCommand.GetCommandName());
	OnCheatExecuted.ExecuteIfBound(CheatCommand, World, Args);
}

void SCheatMenu::InsertCheatIntoRecentlyUsedList(const FString& CheatName)
{
	RecentlyUsedCheatCommands.Remove(CheatName);
	RecentlyUsedCheatCommands.Insert(CheatName, 0);

	if (RecentlyUsedCheatCommands.Num() > NumRecentlyUsedCheatsToShow)
	{
		RecentlyUsedCheatCommands.SetNum(NumRecentlyUsedCheatsToShow);
	}

	SaveFavoriteAndRecentlyUsedCheats();

	if (CurrentTabName == RECENTLY_USED_TAB_NAME)
	{
		RefreshTabContent();
	}
}

FReply SCheatMenu::HandleCheatFavoriteButtonClicked(FString CheatName)
{
	ToggleCheatForFavorites(CheatName);
	return FReply::Handled();
}

void SCheatMenu::ToggleCheatForFavorites(const FString& CheatName)
{
	if (FavoriteCheatCommands.Contains(CheatName))
	{
		FavoriteCheatCommands.Remove(CheatName);
	}
	else
	{
		FavoriteCheatCommands.Add(CheatName);
	}

	SaveFavoriteAndRecentlyUsedCheats();

	if (CurrentTabName == FAVORITE_TAB_NAME)
	{
		RefreshTabContent();
	}
}

void SCheatMenu::SaveFavoriteAndRecentlyUsedCheats() const
{
	if (GConfig != nullptr)
	{
		GConfig->SetArray(CHEAT_MENU_INI_SECTION, CHEAT_MENU_INI_FAVORITES, FavoriteCheatCommands, CHEAT_MENU_INI_FILE);
		GConfig->SetArray(CHEAT_MENU_INI_SECTION, CHEAT_MENU_INI_RECENTLY_USED, RecentlyUsedCheatCommands, CHEAT_MENU_INI_FILE);
		GConfig->Flush(false, CHEAT_MENU_INI_FILE);
	}
}

void SCheatMenu::RestoreFavoriteAndRecentlyUsedCheats()
{
	if (GConfig != nullptr)
	{
		GConfig->GetArray(CHEAT_MENU_INI_SECTION, CHEAT_MENU_INI_FAVORITES, OUT FavoriteCheatCommands, CHEAT_MENU_INI_FILE);
		GConfig->GetArray(CHEAT_MENU_INI_SECTION, CHEAT_MENU_INI_RECENTLY_USED, OUT RecentlyUsedCheatCommands, CHEAT_MENU_INI_FILE);
	}
}
