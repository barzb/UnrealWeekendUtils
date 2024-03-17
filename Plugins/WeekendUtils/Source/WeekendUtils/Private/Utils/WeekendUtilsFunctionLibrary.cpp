///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#include "Utils/WeekendUtilsFunctionLibrary.h"

#include "EngineUtils.h"

USceneComponent* UWeekendUtilsFunctionLibrary::FindClosestComponentOnActorToWorldLocation(const TSubclassOf<USceneComponent> ComponentClass, const AActor* Actor, const FVector WorldLocation)
{
	TArray<USceneComponent*> Components;
	Actor->GetComponents(ComponentClass, OUT Components);

	float ClosestDistSqr = MAX_FLT;
	USceneComponent* ClosestComponent = nullptr;

	for (USceneComponent* Component : Components)
	{
		const float DistSqr = FVector::DistSquared(WorldLocation, Component->GetComponentLocation());
		if (!ClosestComponent || DistSqr < ClosestDistSqr)
		{
			ClosestComponent = Component;
			ClosestDistSqr = DistSqr;
		}
	}

	return ClosestComponent;
}

AActor* UWeekendUtilsFunctionLibrary::FindClosestActorToWorldLocation(const UObject* WorldContext, const TSubclassOf<AActor> ActorClass, const FVector WorldLocation)
{
	float ClosestDistSqr = MAX_FLT;
	AActor* ClosestActor = nullptr;

	for (AActor* Actor : TActorRange<AActor>(WorldContext->GetWorld(), ActorClass))
	{
		const float DistSqr = FVector::DistSquared(WorldLocation, Actor->GetActorLocation());
		if (!ClosestActor || DistSqr < ClosestDistSqr)
		{
			ClosestActor = Actor;
			ClosestDistSqr = DistSqr;
		}
	}

	return ClosestActor;
}

UObject* UWeekendUtilsFunctionLibrary::FindClosestObjectToWorldLocation(const TArray<UObject*> Objects, const FVector WorldLocation)
{
	float ClosestDistSqr = MAX_FLT;
	UObject* ClosestObject = nullptr;

	for (UObject* Object : Objects)
	{
		const FVector ObjectLocation = (Object->IsA<AActor>() ? Cast<AActor>(Object)->GetActorLocation() : Cast<USceneComponent>(Object)->GetComponentLocation());
		const float DistSqr = FVector::DistSquared(WorldLocation, ObjectLocation);
		if (!ClosestObject || DistSqr < ClosestDistSqr)
		{
			ClosestObject = Object;
			ClosestDistSqr = DistSqr;
		}
	}

	return ClosestObject;
}