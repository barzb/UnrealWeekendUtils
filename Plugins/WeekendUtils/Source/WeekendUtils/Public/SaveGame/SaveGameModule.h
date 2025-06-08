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

#include "SaveGameModule.generated.h"

/**
 * Base class for polymorphic SaveGame modules of the @UModularSaveGame.
 * Subclasses should override the @ModuleName in their constructor.
 */
UCLASS(Abstract, EditInlineNew, CollapseCategories)
class WEEKENDUTILS_API USaveGameModule : public UObject
{
	GENERATED_BODY()

public:
	/** Event fired before the module is being saved, before all SaveGame specified properties have been serialized. */
	DECLARE_MULTICAST_DELEGATE(FOnBeforeModuleSaved)
	FOnBeforeModuleSaved OnBeforeModuleSaved;

	/** Event fired after the module was restored, after all SaveGame specified properties have been deserialized. */
	DECLARE_MULTICAST_DELEGATE(FOnAfterModuleRestored)
	FOnAfterModuleRestored OnAfterModuleRestored;

	/** Default identifier that must be unique across all modules and is used when a module is not registered by custom name. */
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Weekend Utils|Save Game")
	FName DefaultModuleName = NAME_None;

	/** Module version for potential compatibility checks. */
	UPROPERTY(SaveGame, EditDefaultsOnly, Category = "Weekend Utils|Save Game")
	int32 ModuleVersion = 0;

	// - UObject
	virtual void Serialize(FArchive& Ar) override;
	// --

protected:
	/** Called before the module is being saved, before all SaveGame specified properties have been serialized. */
	virtual void PreSaveModule() { OnBeforeModuleSaved.Broadcast(); }

	/** Called after the module was restored, after all SaveGame specified properties have been deserialized. */
	virtual void PostRestoreModule() { OnAfterModuleRestored.Broadcast(); }
};

///////////////////////////////////////////////////////////////////////////////////////

inline void USaveGameModule::Serialize(FArchive& Ar)
{
	if (Ar.ArIsSaveGame && Ar.IsSaving())
	{
		PreSaveModule();
	}

	Super::Serialize(Ar);

	if (Ar.ArIsSaveGame && Ar.IsLoading())
	{
		PostRestoreModule();
	}
}
