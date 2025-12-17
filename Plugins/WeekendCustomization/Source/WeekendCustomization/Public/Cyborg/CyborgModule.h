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
#include "Cyborg/CyborgModuleTrait.h"
#include "UObject/Object.h"

#include "CyborgModule.generated.h"

class UCyborgModuleConfig;
class UCyborgModuleInstallRequirement;
class UCyborgModuleInstaller;
class UCyborgSlot;

USTRUCT(BlueprintType)
struct FCyborgModuleInstallationReceipt
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, VisibleInstanceOnly)
	TSubclassOf<UCyborgModuleInstaller> InstallerClass = nullptr;

	UPROPERTY(SaveGame, VisibleInstanceOnly)
	FDateTime InstallDate = FDateTime::UtcNow();

	// FInstancedStruct CustomMetaData;
};

/**
 * 
 */
UCLASS(DefaultToInstanced, EditInlineNew)
class WEEKENDCUSTOMIZATION_API UCyborgModule : public UObject,
											   public FTickableGameObject
{
	GENERATED_BODY()

	friend class UCyborgModuleConfig;

public:
	UCyborgModule();

	UFUNCTION(BlueprintCallable, Category = "Customization")
	TArray<UCyborgModuleTrait*> GetTraits() const { return ObjectPtrDecay(Traits); }

	UFUNCTION(BlueprintCallable, Category = "Customization")
	bool HasAnyTraits() const { return Traits.Num() > 0; }

	UFUNCTION(BlueprintCallable, Category = "Customization")
	virtual bool IsInstalled() const;

	template <typename TTrait>
	bool HasTraits() const;

	template <typename TTrait>
	TArray<TTrait*> GetTraits() const;

	template <typename TTrait = UCyborgModuleTrait, typename TFunction>
	void ForEachTrait(TFunction Function) const;

	virtual void FinishInstallation(const FCyborgModuleInstallationReceipt& Receipt);
	virtual void FinishUninstallation();

	TArray<const UCyborgModuleInstallRequirement*> GetInstallRequirements() const { return ObjectPtrDecay(InstallRequirements); }

	// - UObject
	virtual void Serialize(FArchive& Ar) override;
	// - FTickableGameObject
	virtual bool IsTickable() const override;
	virtual void Tick(float DeltaTime) override;
	// --

protected:
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Instanced)
	TArray<TObjectPtr<const UCyborgModuleInstallRequirement>> InstallRequirements = {};

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Instanced)
	TArray<TObjectPtr<UCyborgModuleTrait>> Traits = {};

	UPROPERTY(SaveGame, VisibleInstanceOnly)
	TArray<FCyborgModuleInstallationReceipt> InstallationReceipts = {};

	UPROPERTY(SaveGame, VisibleInstanceOnly)
	TObjectPtr<const UCyborgModuleConfig> Config = nullptr;

	// - FTickableGameObject
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual TStatId GetStatId() const override;
	// --
};

///////////////////////////////////////////////////////////////////////////////////////
/// TEMPLATE INLINES

template <typename TTrait>
bool UCyborgModule::HasTraits() const
{
	static_assert(TIsDerivedFrom<TTrait, UCyborgModuleTrait>::Value, "Type is not derived from UCyborgModuleTrait.");
	return Traits.ContainsByPredicate([](const auto* Trait) { return Trait->template IsA<TTrait>(); });
}

template <typename TTrait>
TArray<TTrait*> UCyborgModule::GetTraits() const
{
	static_assert(TIsDerivedFrom<TTrait, UCyborgModuleTrait>::Value, "Type is not derived from UCyborgModuleTrait.");
	TArray<TTrait*> Result{};
	Algo::TransformIf(Traits, OUT Result,
					  [](const UCyborgModuleTrait* Trait) { return Trait->IsA<TTrait>(); },
					  [](UCyborgModuleTrait* Trait) { return Cast<TTrait>(Trait); });
	return Result;
}

template <typename TTrait, typename TFunction>
void UCyborgModule::ForEachTrait(TFunction Function) const
{
	static_assert(TIsDerivedFrom<TTrait, UCyborgModuleTrait>::Value, "Type is not derived from UCyborgModuleTrait.");
	for (TTrait* Trait : GetTraits<TTrait>())
	{
		Invoke(Function, Trait);
	}
}
