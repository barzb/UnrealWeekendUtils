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

		World = UWorld::CreateWorld(EWorldType::Game, false, *InWorldName, nullptr, true, ERHIFeatureLevel::Num, nullptr, true);
		World->GetWorldSettings()->DefaultGameMode = AGameModeBase::StaticClass();
		World->SetShouldTick(false);
		World->AddToRoot();

		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		WorldContext.OwningGameInstance = GameInstance;
		World->SetGameInstance(GameInstance);
		WorldContext.SetCurrentWorld(World);

		// (i) GameInstance is initialized BEFORE the world here. This behavior seems inconsistent in the engine flow.
		// GameInstance->InitializeStandalone() initializes the world first, which leads to issues with GameServices.
		// But loading a world from an asset will init the world after the GameInstance, which sounds more reasonable.
		GameInstance->Init();
		World->InitWorld();
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

			if (IsValid(Viewport))
			{
				Viewport->DetachViewportClient();
			}

			if (GameInstance != nullptr)
			{
				GameInstance->Shutdown();
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

		// Run Garbage Collection to force destruction
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		GEngine->CheckAndHandleStaleWorldObjectReferences();
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
