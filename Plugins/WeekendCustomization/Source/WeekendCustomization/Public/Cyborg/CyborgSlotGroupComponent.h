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
#include "ComponentVisualizer.h"
#include "CyborgCustomizableInterface.h"
#include "CyborgSlot.h"
#include "Components/SceneComponent.h"

#include "CyborgSlotGroupComponent.generated.h"

class FCanvas;
class FPrimitiveDrawInterface;
class FSceneView;
class FViewport;
class UCyborgModuleConfig;

///////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct WEEKENDCUSTOMIZATION_API FCyborgSlotGroupDefaultModuleConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<const UCyborgModuleConfig> ModuleConfig = nullptr;

	UPROPERTY(EditAnywhere)
	TArray<FName> SpecificSlotNames{};
};

///////////////////////////////////////////////////////////////////////////////////////

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class WEEKENDCUSTOMIZATION_API UCyborgSlotGroupComponent : public USceneComponent,
														   public ICyborgCustomizableInterface
{
	GENERATED_BODY()

public:
	UCyborgSlotGroupComponent();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void InstallTestModule(const UCyborgModuleConfig* ModuleConfig);

	virtual void InstallDefaultModules();

	FGameplayTag GetGroupTag() const { return GroupTag; }

	// - USceneComponent
	virtual void InitializeComponent() override;
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	// - ICyborgCustomizableInterface
	virtual TArray<UCyborgSlot*> GetCustomizationSlots() const override { return ObjectPtrDecay(ConfiguredSlots); }
	virtual const UCyborgModuleInstaller& GetModuleInstaller() const override;
	using ICyborgCustomizableInterface::GetEmptyCustomizationSlots;
	using ICyborgCustomizableInterface::GetCustomizationSlotsWithInstalledModules;
	using ICyborgCustomizableInterface::GetNamedCustomizationSlots;
	using ICyborgCustomizableInterface::GetInstalledCustomizationModules;
	using ICyborgCustomizableInterface::AreAnyCustomizationModulesInstalled;
	using ICyborgCustomizableInterface::CanInstallNewModule;
	using ICyborgCustomizableInterface::TryInstallNewModule;
	using ICyborgCustomizableInterface::CanReplaceInstalledModule;
	using ICyborgCustomizableInterface::TryReplaceInstalledModule;
	using ICyborgCustomizableInterface::CanUninstallNewModule;
	using ICyborgCustomizableInterface::TryUninstallInstalledModule;
	// --

	class FComponentVisualizer : public ::FComponentVisualizer
	{
	protected:
		virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	};

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, NoClear, meta = (Categories = "Customization.SlotGroup"))
	FGameplayTag GroupTag{};

	UPROPERTY(EditAnywhere)
	bool bUseModuleInstallerFromOwner = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, NoClear, meta = (EditCondition = "!bUseModuleInstallerFromOwner"))
	TSubclassOf<UCyborgModuleInstaller> ModuleInstaller{UCyborgModuleInstaller::StaticClass()};

	UPROPERTY(EditAnywhere)
	bool bShouldAutoInstallDefaultModules = true;

	UPROPERTY(EditAnywhere)
	TArray<FCyborgSlotGroupDefaultModuleConfig> DefaultModules{};

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Instanced, meta = (ShowOnlyInnerProperties, TitleProperty = "SlotName"))
	TArray<TObjectPtr<UCyborgSlot>> ConfiguredSlots{};
};
