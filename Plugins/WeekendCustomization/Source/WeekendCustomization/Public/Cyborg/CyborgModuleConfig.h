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
#include "CyborgModule.h"
#include "Engine/DataAsset.h"

#include "CyborgModuleConfig.generated.h"

UCLASS(CollapseCategories, Const)
class WEEKENDCUSTOMIZATION_API UCyborgModuleConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FName ModuleName = NAME_None;

	template <typename T = UCyborgModule>
	T& CreateNewModuleFor(UObject* Owner) const
	{
		check(Module && Module->IsA<T>());
		T& NewModule = *DuplicateObject<T>(Module.Get(), Owner, MakeUniqueObjectName(Owner, Module->GetClass(), ModuleName)); // Use configured Module sub-object as template.
		NewModule.Config = this;
		return NewModule;
	}

protected:
	UPROPERTY(EditAnywhere, Instanced, NoClear, meta = (ShowOnlyInnerProperties))
	TObjectPtr<const UCyborgModule> Module = CreateDefaultSubobject<UCyborgModule>("Module");
};

USTRUCT()
struct FCyborgModuleTableRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, NoClear)
	TObjectPtr<const UCyborgModuleConfig> ModuleConfig = nullptr;
};