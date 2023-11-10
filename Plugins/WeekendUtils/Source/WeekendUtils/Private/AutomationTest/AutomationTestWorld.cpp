///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Nine Worlds Studios GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#include "AutomationTest/AutomationTestWorld.h"

#if WITH_AUTOMATION_WORKER

#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"

namespace WeekendUtils
{
	FScopedAutomationTestWorld::FScopedAutomationTestWorld(FString InWorldName)
	{
		const FString NewWorldName = "WeekendUtils_ScopedAutomationTestWorld_" + InWorldName;

		// Create and initialize game instance
		GameInstance = NewObject<UGameInstance>(GEngine);
		GameInstance->InitializeStandalone(*NewWorldName); // -> indirectly calls GameInstance->Init();

		World = GameInstance->GetWorld();
		World->GetWorldSettings()->DefaultGameMode = AGameModeBase::StaticClass();
	}

	FScopedAutomationTestWorld::~FScopedAutomationTestWorld()
	{
		// Copied from UGameEngine::PreExit()
		if (IsValid(World))
		{
			World->BeginTearingDown();
			GEngine->ShutdownWorldNetDriver(World);

			for (AActor* ActorItr : TActorRange<AActor>(World))
			{
				ActorItr->RouteEndPlay(EEndPlayReason::Quit);
			}

			if (IsValid(Viewport))
			{
				Viewport->DetachViewportClient();
			}

			if (World->GetGameInstance() != nullptr)
			{
				World->GetGameInstance()->Shutdown();
			}

			World->DestroyWorld(true);
			GEngine->DestroyWorldContext(World);
		}

		World = nullptr;
		GameInstance = nullptr;
		GameMode = nullptr;
		LocalPlayer = nullptr;
		PlayerController = nullptr;
		Viewport = nullptr;
	}

	void FScopedAutomationTestWorld::InitializeGame() { InitializeGame(FConfig()); }
	void FScopedAutomationTestWorld::InitializeGame(FConfig Config)
	{
		if (!ensure(IsValid(World)))
			return;

		const FURL UrlParam = Config.Url;
		if (!ensure(World->SetGameMode(UrlParam)))
			return;

		GameMode = World->GetAuthGameMode();
		GameMode->PlayerStateClass = (Config.PlayerStateClass ? Config.PlayerStateClass : TSubclassOf<APlayerState>(APlayerState::StaticClass()));

		Viewport = NewObject<UGameViewportClient>(GEngine);
		GameInstance->GetWorldContext()->GameViewport = Viewport;
		Viewport->Init(*GameInstance->GetWorldContext(), GameInstance);

		FString ErrorString;
		LocalPlayer = World->GetGameInstance()->CreateLocalPlayer(0, OUT ErrorString, false);

		if (!ensureMsgf(ErrorString.Len() == 0 && LocalPlayer != nullptr, TEXT("Error creating LocalPlayer: %s"), *ErrorString))
			return;

		World->InitializeActorsForPlay(UrlParam);
		World->BeginPlay();

		// Create a new unique net ID to spawn the local play actor = PlayerController:
		FUniqueNetIdRepl NetIdRepl = GameInstance->GetPrimaryPlayerUniqueIdRepl();
		PlayerController = World->SpawnPlayActor(LocalPlayer, ENetRole::ROLE_Authority, UrlParam, NetIdRepl, OUT ErrorString);
		ensureMsgf(ErrorString.Len() == 0 && PlayerController != nullptr, TEXT("Error creating PlayerController: %s"), *ErrorString);
	}
}

#endif
