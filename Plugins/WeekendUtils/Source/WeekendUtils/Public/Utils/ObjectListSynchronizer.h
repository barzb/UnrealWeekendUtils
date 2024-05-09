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

/**
 * Utility container for synchronizing the elements of an object pointer list with an arbitrary array of data.
 *
 * @example:
 *
 *	TArray<TObjectPtr<USomeViewModel>> ViewModels = { ... };
 *	TArray<FSomeModel> Models = { ... };
 *
 *	TObjectListSynchronizer(ViewModels, Models)
 *	.ForEachMissingElement([this](FSomeModel& Model) -> USomeViewModel*
 *	{
 *		USomeViewModel* NewViewModel = NewObject<USomeViewModel>(this);
 *		NewViewModel->BindToModel(Model);
 *		return NewViewModel;
 *	})
 *	.ForEachUpdatedElement([](USomeViewModel& ViewModel, FSomeModel& Model)
 *	{
 *		ViewModel.UnbindFromModel();
 *		ViewModel.BindToModel(Model);
 *	})
 *	.ForEachRemovedElement([](USomeViewModel& ViewModel)
 *	{
 *		ViewModel.UnbindFromModel();
 *	});
 */
template <typename ObjectType, typename InitListType>
class TObjectListSynchronizer
{
public:
	TObjectListSynchronizer(TArray<ObjectType*>& InObjectList, InitListType& InInitDataList) :
		ObjectList(ObjectPtrWrap(InObjectList)), InitDataList(InInitDataList) { Init(); }
	TObjectListSynchronizer(TArray<TObjectPtr<ObjectType>>& InObjectList, InitListType& InInitDataList) :
		ObjectList(InObjectList), InitDataList(InInitDataList) { Init(); }

	/** PredicateType must equivalent to: TFunctionRef<ObjectType*(InitDataType&)> */
	template <typename PredicateType>
	TObjectListSynchronizer& ForEachMissingElement(PredicateType Predicate)
	{
		const int32 NumMissing = (InitDataList.Num() - ObjectList.Num());
		if (NumMissing <= 0)
			return *this;

		ObjectList.SetNumUninitialized(InitDataList.Num());
		for (int32 i = (InitDataList.Num() - NumMissing); i < InitDataList.Num(); ++i)
		{
			ObjectList[i] = Predicate(InitDataList[i]);
		}
		return *this;
	}

	/** PredicateType must equivalent to: TFunctionRef<ObjectType*(InitDataType&, int32)> */
	template <typename PredicateType>
	TObjectListSynchronizer& ForEachMissingElementAndIndex(PredicateType Predicate)
	{
		const int32 NumMissing = (InitDataList.Num() - ObjectList.Num());
		if (NumMissing <= 0)
			return *this;

		ObjectList.SetNumUninitialized(InitDataList.Num());
		for (int32 i = (InitDataList.Num() - NumMissing); i < InitDataList.Num(); ++i)
		{
			ObjectList[i] = Predicate(InitDataList[i], i);
		}
		return *this;
	}

	/** PredicateType must equivalent to: TFunctionRef<void(ObjectType&)> */
	template <typename PredicateType>
	TObjectListSynchronizer& ForEachRemovedElement(PredicateType Predicate)
	{
		for (int32 i = 0; i < RemovedObjects.Num(); ++i)
		{
			Predicate(*RemovedObjects[i]);
		}
		return *this;
	}

	/** PredicateType must equivalent to: TFunctionRef<void(ObjectType&, InitDataType&)> */
	template <typename PredicateType>
	TObjectListSynchronizer& ForEachUpdatedElement(PredicateType Predicate)
	{
		for (int32 i = 0; i < UpdatedObjects.Num(); ++i)
		{
			Predicate(*UpdatedObjects[i], InitDataList[i]);
		}
		return *this;
	}

	/** PredicateType must equivalent to: TFunctionRef<void(ObjectType&, InitDataType&, int32)> */
	template <typename PredicateType>
	TObjectListSynchronizer& ForEachUpdatedElementAndIndex(PredicateType Predicate)
	{
		for (int32 i = 0; i < UpdatedObjects.Num(); ++i)
		{
			Predicate(*UpdatedObjects[i], InitDataList[i], i);
		}
		return *this;
	}

private:
	TArray<TObjectPtr<ObjectType>>& ObjectList;
	InitListType& InitDataList;
	TArray<ObjectType*> RemovedObjects = {};
	TArray<ObjectType*> UpdatedObjects = {};

	void Init()
	{
		while (ObjectList.Num() > InitDataList.Num())
		{
			RemovedObjects.Add(ObjectList.Pop());
		}
		UpdatedObjects = ObjectList;
	}
};
