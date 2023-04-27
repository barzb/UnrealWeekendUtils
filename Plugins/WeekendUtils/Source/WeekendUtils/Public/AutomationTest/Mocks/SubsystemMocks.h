// (c) 2023 Nine Worlds Studios GmbH. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "Subsystems/EngineSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Subsystems/WorldSubsystem.h"

#include "SubsystemMocks.generated.h"

#define ONLY_CREATE_FOR_TESTS \
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return WITH_AUTOMATION_WORKER; }

UCLASS(Hidden, ClassGroup=Tests)
class WEEKENDUTILS_API UEngineSubsystemMock : public UEngineSubsystem { GENERATED_BODY() ONLY_CREATE_FOR_TESTS };

UCLASS(Hidden, ClassGroup=Tests)
class WEEKENDUTILS_API UWorldSubsystemMock : public UWorldSubsystem
{
	GENERATED_BODY()
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return (WITH_AUTOMATION_WORKER && GIsAutomationTesting); }
};

UCLASS(Hidden, ClassGroup=Tests)
class WEEKENDUTILS_API UGameInstanceSubsystemMock : public UGameInstanceSubsystem { GENERATED_BODY() ONLY_CREATE_FOR_TESTS };

UCLASS(Hidden, ClassGroup=Tests)
class WEEKENDUTILS_API ULocalPlayerSubsystemMock : public ULocalPlayerSubsystem { GENERATED_BODY() ONLY_CREATE_FOR_TESTS };

#undef ONLY_CREATE_FOR_TESTS
