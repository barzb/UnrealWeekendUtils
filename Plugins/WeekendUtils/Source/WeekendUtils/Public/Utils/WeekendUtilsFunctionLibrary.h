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
#include "Kismet/BlueprintFunctionLibrary.h"

#include "WeekendUtilsFunctionLibrary.generated.h"

/**
 * Assorted utility functions for code and blueprint.
 */
UCLASS()
class WEEKENDUTILS_API UWeekendUtilsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Utils|Distance", meta = (DeterminesOutputType = "ComponentClass"))
	static USceneComponent* FindClosestComponentOnActorToWorldLocation(const TSubclassOf<USceneComponent> ComponentClass, const AActor* Actor, const FVector WorldLocation);

	UFUNCTION(BlueprintCallable, Category = "Utils|Distance", meta = (WorldContext = "WorldContext", DeterminesOutputType = "ActorClass"))
	static AActor* FindClosestActorToWorldLocation(const UObject* WorldContext, const TSubclassOf<AActor> ActorClass, const FVector WorldLocation);

	/** Objects in list must derive from Actor or SceneComponent. */
	UFUNCTION(BlueprintCallable, Category = "Utils|Distance")
	static UObject* FindClosestObjectToWorldLocation(const TArray<UObject*> Objects, const FVector WorldLocation);
};
