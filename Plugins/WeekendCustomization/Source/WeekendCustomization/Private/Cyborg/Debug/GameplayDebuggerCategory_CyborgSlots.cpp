///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebuggerCategory_CyborgSlots.h"

#include "Cyborg/CyborgSlot.h"

FGameplayDebuggerCategory_CyborgSlots::FGameplayDebuggerCategory_CyborgSlots()
{
	bShowOnlyWithDebugActor = false;
}

void FGameplayDebuggerCategory_CyborgSlots::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	if (!IsValid(OwnerPC))
		return;

	TMap<const AActor*, TArray<FString>> SlotInfoByActor;
	for (const UCyborgSlot* Slot : TObjectRange<UCyborgSlot>(RF_ClassDefaultObject | RF_ArchetypeObject, true, EInternalObjectFlags::Garbage))
	{
		if (!IsValid(Slot) || Slot->GetWorld() != OwnerPC->GetWorld())
			continue;

		if (const AActor* SlotOwningActor = Slot->GetTypedOuter<AActor>())
		{
			SlotInfoByActor.FindOrAdd(SlotOwningActor)
			.Add("{white}[{yellow}" + Slot->GetSlotName().ToString() + "{white}] " + Slot->GetDebugInfo());
		}
	}

	for (const TTuple<const AActor*, TArray<FString>>& InfoByActor : SlotInfoByActor)
	{
		AddTextLine("{white}" + InfoByActor.Key->GetName());
		for (const FString& InfoLine : InfoByActor.Value)
		{
			AddTextLine("\t - " + InfoLine);
		}
	}
}

#endif
