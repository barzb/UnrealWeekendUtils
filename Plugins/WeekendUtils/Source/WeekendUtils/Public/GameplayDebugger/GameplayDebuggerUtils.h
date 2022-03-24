// (c) by Benjamin Barz

#pragma once

#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebugger.h"
#include "Templates/UnrealTypeTraits.h"

////////////////////////////////////////////////////////////////
/// GameplayDebuggerExtension

#define GENERATE_DEBUGGER_EXTENSION(ExtensionName) \
	static FName GetExtensionName() { return #ExtensionName ; } \
	static TSharedRef<FGameplayDebuggerExtension> MakeInstance() { return MakeShareable(new FGameplayDebuggerExtension_##ExtensionName); }

namespace FGameplayDebugger
{
	template<typename T, typename TEnableIf<TIsDerivedFrom<T, FGameplayDebuggerExtension>::IsDerived, bool>::Type = true>
static TSharedRef<FGameplayDebuggerExtension> MakeInstance()
	{
		return MakeShareable(new T());
	}

	template<typename T, typename TEnableIf<TIsDerivedFrom<T, FGameplayDebuggerExtension>::IsDerived, bool>::Type = true>
	void RegisterExtension()
	{
		IGameplayDebugger::Get().RegisterExtension(T::GetExtensionName(), IGameplayDebugger::FOnGetExtension::CreateStatic(&T::MakeInstance));
	}

	template<typename T, typename TEnableIf<TIsDerivedFrom<T, FGameplayDebuggerExtension>::IsDerived, bool>::Type = true>
	void RegisterExtension(FName ExtensionName)
	{
		IGameplayDebugger::Get().RegisterExtension(ExtensionName, IGameplayDebugger::FOnGetExtension::CreateStatic(&MakeInstance<T>));
	}

	template<typename T, typename TEnableIf<TIsDerivedFrom<T, FGameplayDebuggerExtension>::IsDerived, bool>::Type = true>
	void UnregisterExtension()
	{
		IGameplayDebugger::Get().UnregisterExtension(T::GetExtensionName());
	}

	template<typename T, typename TEnableIf<TIsDerivedFrom<T, FGameplayDebuggerExtension>::IsDerived, bool>::Type = true>
	void UnregisterExtension(FName ExtensionName)
	{
		IGameplayDebugger::Get().UnregisterExtension(ExtensionName);
	}
}

////////////////////////////////////////////////////////////////
/// GameplayDebuggerCategory

#define GENERATE_DEBUGGER_CATEGORY(CategoryName) \
	static FName GetCategoryName() { return #CategoryName ; } \
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance() { return MakeShareable(new FGameplayDebuggerCategory_##CategoryName); }

namespace FGameplayDebugger
{
	template<typename T, typename TEnableIf<TIsDerivedFrom<T, FGameplayDebuggerCategory>::IsDerived, bool>::Type = true>
	void RegisterCategory(EGameplayDebuggerCategoryState DefaultState = EGameplayDebuggerCategoryState::Disabled)
	{
		IGameplayDebugger::Get().RegisterCategory(T::GetCategoryName(), IGameplayDebugger::FOnGetCategory::CreateStatic(&T::MakeInstance), DefaultState);
	}

	template<typename T, typename TEnableIf<TIsDerivedFrom<T, FGameplayDebuggerExtension>::IsDerived, bool>::Type = true>
	void RegisterCategory(FName CategoryName, EGameplayDebuggerCategoryState DefaultState = EGameplayDebuggerCategoryState::Disabled)
	{
		IGameplayDebugger::Get().RegisterCategory(CategoryName, IGameplayDebugger::FOnGetCategory::CreateStatic(&MakeInstance<T>), DefaultState);
	}

	template<typename T, typename TEnableIf<TIsDerivedFrom<T, FGameplayDebuggerCategory>::IsDerived, bool>::Type = true>
	void UnregisterCategory()
	{
		IGameplayDebugger::Get().UnregisterCategory(T::GetCategoryName());
	}

	template<typename T, typename TEnableIf<TIsDerivedFrom<T, FGameplayDebuggerCategory>::IsDerived, bool>::Type = true>
	void UnregisterCategory(FName CategoryName)
	{
		IGameplayDebugger::Get().UnregisterCategory(CategoryName);
	}
}

#endif