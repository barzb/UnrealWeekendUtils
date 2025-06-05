///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameService/GameServiceConfig.h"

#include "GameModeServiceConfigBase.generated.h"

/**
 * Base class for configurations whose CDO will be automatically registered with the @UGameServiceManager.
 * Derived configurations should define at least one @AGameModeBase class.
 * When a world with that game mode is entered, the @UWorldGameServiceRunner will automatically register
 * the CDO of the config with the service manager.
 */
UCLASS(Abstract, EditInlineNew)
class WEEKENDUTILS_API UGameModeServiceConfigBase : public UGameServiceConfig
{
	GENERATED_BODY()

public:
	/** This is where services should be configured in derived classes. Can be called multiple times in the editor. */
	virtual void Configure() PURE_VIRTUAL(Configure);

	/** @returns the CDO of the game mode service config class configured for given world, or nullptr. */
	static const UGameModeServiceConfigBase* FindConfigForWorld(const UWorld& World);

	/** @returns whether this config instance is configured for given game mode class. */
	bool ShouldUseWithGameMode(const TSubclassOf<AGameModeBase>& GameModeClass) const;
	template<class T> bool ShouldUseWithGameMode() const { return ShouldUseWithGameMode(T::StaticClass()); }

	/** Registers this config class to be used for worlds with given game mode class. */
	void RegisterFor(const TSubclassOf<AGameModeBase>& GameModeClass);
	template<class T> void RegisterFor() { RegisterFor(T::StaticClass()); }

protected:
	UPROPERTY(VisibleAnywhere)
	TArray<TSubclassOf<AGameModeBase>> ConfiguredGameModes = {};

	void Reconfigure();

	// - UObject
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// --
};
