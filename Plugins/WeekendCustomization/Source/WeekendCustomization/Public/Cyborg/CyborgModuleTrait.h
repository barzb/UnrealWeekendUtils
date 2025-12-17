///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "CyborgModuleTrait.generated.h"

/**
 * Base class for traits that compose a @UCyborgModule.
 * Traits are activated upon module installation and deactivated upon uninstallation.
 * Traits can be ticked by their owning module, when implementing @IsTickable.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced, Within = CyborgModule)
class WEEKENDCUSTOMIZATION_API UCyborgModuleTrait : public UObject
{
	GENERATED_BODY()

public:
	virtual void Activate() { bIsActive = true;}
	virtual void Tick(float DeltaTime) {}
	virtual void Deactivate() { bIsActive = false;}

	virtual bool IsTickable() const { return false; }
	virtual bool IsActive() const { return bIsActive; }

protected:
	UPROPERTY(SaveGame, VisibleInstanceOnly)
	bool bIsActive = false;
};

UCLASS()
class WEEKENDCUSTOMIZATION_API UTestCyborgModuleTrait : public UCyborgModuleTrait
{
	GENERATED_BODY()
};