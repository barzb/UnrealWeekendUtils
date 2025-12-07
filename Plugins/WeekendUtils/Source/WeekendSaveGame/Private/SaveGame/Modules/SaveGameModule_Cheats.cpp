///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "SaveGame/Modules/SaveGameModule_Cheats.h"

#include "GameService/GameServiceLocator.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SaveGame/ModularSaveGame.h"
#include "SaveGame/SaveGameService.h"

bool UExecuteCheatFromSaveGameSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	switch (WorldType)
	{
		case EWorldType::Game:
		case EWorldType::PIE:
			return true;

		default:
			return false;
	}
}

bool UExecuteCheatFromSaveGameSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if (UE_BUILD_SHIPPING || UE_BUILD_TEST)
	return false;
#else
	if (!Super::ShouldCreateSubsystem(Outer))
		return false;

	const auto& SaveGameService = UGameServiceLocator::FindServiceAsWeakPtr<USaveGameService>(Outer);
	if (!SaveGameService.IsValid())
		return false;

	const UModularSaveGame* ModularSaveGame = SaveGameService->GetCurrentSaveGame().GetPtr<UModularSaveGame>();
	return (IsValid(ModularSaveGame) && ModularSaveGame->HasModule<USaveGameModule_Cheats>());
#endif
}

void UExecuteCheatFromSaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const auto& SaveGameService = UGameServiceLocator::FindServiceAsWeakPtr<USaveGameService>(this);
	const UModularSaveGame* ModularSaveGame = SaveGameService->GetCurrentSaveGame().GetPtr<UModularSaveGame>();
	const USaveGameModule_Cheats* CheatsModule = ModularSaveGame->FindModule<USaveGameModule_Cheats>();
	for (const FString& CheatCommand : CheatsModule->CheatsToExecuteAfterTravel)
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(this, CheatCommand);
	}
}
