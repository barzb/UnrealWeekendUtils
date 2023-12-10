///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2023 by Benjamin Barz in cooperation with Aesir Interactive GmbH.
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See accompanying file LICENSE.
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "EnumUtils.h"

///////////////////////////////////////////////////////////////////////////////////////
/// UFUNCTION(BlueprintCallable, BlueprintPure = False, ExpandEnumAsExecs = "ReturnValue")
/// ECommonAvailability BranchAvailability() const;

UENUM(BlueprintType)
enum class ECommonAvailability : uint8
{
	Available,
	Unavailable
};

DEFINE_ENUM_STRING_CONVERTERS(CommonAvailability, ECommonAvailability);
DEFINE_ENUM_BOOL_CONVERTERS(CommonAvailability, ECommonAvailability, Available, Unavailable);

///////////////////////////////////////////////////////////////////////////////////////
