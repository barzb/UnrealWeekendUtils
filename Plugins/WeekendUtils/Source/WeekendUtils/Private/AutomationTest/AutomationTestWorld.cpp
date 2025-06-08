///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
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
			GEngine->CancelPending(World);
			GEngine->ShutdownWorldNetDriver(World);

			World->bIsLevelStreamingFrozen = false;
			World->SetShouldForceUnloadStreamingLevels((true));
			World->FlushLevelStreaming(EFlushLevelStreamingType::Visibility);
			World->EndPlay(EEndPlayReason::Quit);

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

			World->CleanupWorld();
			World->DestroyWorld(true);
			GEngine->DestroyWorldContext(World);
		}

		World = nullptr;
		GameInstance = nullptr;
		GameMode = nullptr;
		LocalPlayer = nullptr;
		PlayerController = nullptr;
		Viewport = nullptr;

		// Run Garbage Collection to force destruction
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
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
