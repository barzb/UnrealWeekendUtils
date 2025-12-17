///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendCustomization UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Cyborg/CyborgSlotGroupComponent.h"

#include "WeekendCustomization.h"
#include "Cyborg/CyborgModuleConfig.h"
#include "Misc/DataValidation.h"

namespace
{
	struct HCyborgSlotHitProxy : HHitProxy
	{
		DECLARE_HIT_PROXY();

		HCyborgSlotHitProxy() : HHitProxy(HPP_Foreground) {}
		virtual EMouseCursor::Type GetMouseCursor() override { return EMouseCursor::CardinalCross; }
	};

	IMPLEMENT_HIT_PROXY(HCyborgSlotHitProxy, HComponentVisProxy);
}

UCyborgSlotGroupComponent::UCyborgSlotGroupComponent()
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = false;
}

void UCyborgSlotGroupComponent::InstallTestModule(const UCyborgModuleConfig* ModuleConfig)
{
	if (!IsValid(ModuleConfig))
		return;

	UCyborgModule& Module = ModuleConfig->CreateNewModuleFor(this);
	TryInstallNewModule(Module);
}

void UCyborgSlotGroupComponent::InstallDefaultModules()
{
	for (const FCyborgSlotGroupDefaultModuleConfig& DefaultModuleConfig : DefaultModules)
	{
		check(DefaultModuleConfig.ModuleConfig);

		TArray<UCyborgSlot*> Slots;
		if (DefaultModuleConfig.SpecificSlotNames.IsEmpty())
		{
			Slots = GetEmptyCustomizationSlots();
		}
		else
		{
			Slots = GetNamedCustomizationSlots(DefaultModuleConfig.SpecificSlotNames);
			check(Slots.Num() == DefaultModuleConfig.SpecificSlotNames.Num());
		}

		UCyborgModule& Module = DefaultModuleConfig.ModuleConfig->CreateNewModuleFor(this);
		if (!GetModuleInstaller().Install(Module, Slots).bWasSuccess)
		{
			UE_LOG(LogWeekendCustomization, Error, TEXT("Could not install default module %s for %s"), *Module.GetName(), *GetPathName());
		}
	}
}

void UCyborgSlotGroupComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Skip auto-install if there are already modules installed, e.g. after restoring from save game.
	if (bShouldAutoInstallDefaultModules && !AreAnyCustomizationModulesInstalled())
	{
		InstallDefaultModules();
	}
}

#if WITH_EDITOR
EDataValidationResult UCyborgSlotGroupComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (const FCyborgSlotGroupDefaultModuleConfig& DefaultModuleConfig : DefaultModules)
	{
		if (!DefaultModuleConfig.ModuleConfig)
		{
			Context.AddError(INVTEXT("Invalid DefaultModuleConfig.ModuleConfig"));
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
		}

		const TArray<UCyborgSlot*> RequestedSlots = GetNamedCustomizationSlots(DefaultModuleConfig.SpecificSlotNames);
		if (RequestedSlots.Num() != DefaultModuleConfig.SpecificSlotNames.Num())
		{
			Context.AddError(INVTEXT("Mismatching Slots in DefaultModuleConfig.SpecificSlotNames"));
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
		}
	}

	if (!bUseModuleInstallerFromOwner && !ModuleInstaller)
	{
		Context.AddError(INVTEXT("Invalid ModuleInstaller"));
		Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
	}

	if (bUseModuleInstallerFromOwner && GetOwner() && !GetOwner<ICyborgCustomizableInterface>())
	{
		Context.AddError(INVTEXT("Cannot use ModuleInstaller from owner that doesn't implement ICyborgCustomizableInterface"));
		Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
	}

	return Result;
}
#endif

const UCyborgModuleInstaller& UCyborgSlotGroupComponent::GetModuleInstaller() const
{
	if (bUseModuleInstallerFromOwner)
	{
		const ICyborgCustomizableInterface* CustomizableOwner = GetOwner<ICyborgCustomizableInterface>();
		checkf(CustomizableOwner, TEXT("%s was expected to implement ICyborgCustomizableInterface"), *GetOwner()->GetName());
		return CustomizableOwner->GetModuleInstaller();
	}

	checkf(ModuleInstaller, TEXT("%s has not configured a valid ModuleInstaller"), *GetPathName())
	return *GetDefault<UCyborgModuleInstaller>(ModuleInstaller);
}

void UCyborgSlotGroupComponent::FComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (View == nullptr || PDI == nullptr)
		return;

	const UCyborgSlotGroupComponent* CyborgSlotGroupComponent = Cast<const UCyborgSlotGroupComponent>(Component);
	if (CyborgSlotGroupComponent == nullptr)
		return;

	static constexpr float SlotRadius = 50.f;

	const FTransform OwnerLocalToWorld = CyborgSlotGroupComponent->GetComponentTransform();
	for (const UCyborgSlot* Slot : CyborgSlotGroupComponent->GetCustomizationSlots())
	{
		PDI->SetHitProxy(new HCyborgSlotHitProxy());
		const FTransform SlotWorldTransform = FTransform(FRotator::ZeroRotator, Slot->GetRelativePosition()) * OwnerLocalToWorld;
		::DrawCircle(PDI,
			SlotWorldTransform.GetLocation(),
			SlotWorldTransform.GetUnitAxis(EAxis::X),
			SlotWorldTransform.GetUnitAxis(EAxis::Y),
			FColor::Yellow,
			SlotRadius, 64,
			SDPG_World, 4.f, 2.f, true);
	}

	PDI->SetHitProxy(nullptr);
}
