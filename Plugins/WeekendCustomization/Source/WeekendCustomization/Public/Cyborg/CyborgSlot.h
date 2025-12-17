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
#include "GameplayTagContainer.h"
#include "UObject/Object.h"

#include "CyborgSlot.generated.h"

class UCyborgModule;

/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, CollapseCategories, DisplayName = "Default Slot")
class WEEKENDCUSTOMIZATION_API UCyborgSlot : public UObject
{
	GENERATED_BODY()

	friend class UCyborgModuleInstaller;

public:
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FName GetSlotName() const { return SlotName; }

	UFUNCTION(BlueprintCallable, Category = "Customization")
	FGameplayTagContainer GetSockets() const { return Sockets; }

	UFUNCTION(BlueprintCallable, Category = "Customization")
	UCyborgModule* GetInstalledModule() const { return InstalledModule.Get(); }

	UFUNCTION(BlueprintCallable, Category = "Customization")
	bool IsAnyModuleInstalled() const { return !!InstalledModule; }

	UFUNCTION(BlueprintCallable, Category = "Customization")
	bool IsAvailableForInstallation() const { return !InstalledModule; }

	FVector GetRelativePosition() const { return RelativePosition; }

	virtual FString GetDebugInfo() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SlotName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MakeEditWidget = true))
	FVector RelativePosition = FVector::ZeroVector;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Customization.Socket"))
	FGameplayTagContainer Sockets = FGameplayTagContainer();

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCyborgModule> InstalledModule = nullptr;
};
