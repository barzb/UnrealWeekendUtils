///////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) by Benjamin Barz and contributors. See file: CREDITS.md
///
/// This file is part of the WeekendUtils UE5 Plugin.
///
/// Distributed under the MIT License. See file: LICENSE.md
///
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#if WITH_AUTOMATION_WORKER

//////////////////////////////////////////////////////////////////////////
// This header file contains utility macros that should aid in the creation
// of parametrized automated tests.
// Parametrization means that a single Describe or It call is reused and executed
// for a set of parameter payloads.
//
// All of the macros in this file must contain the initials WE to avoid
// name clashes and make them easily distinguishable!
//
// Example of using the WE_SPEC_CASE* macros to parametrize an automation spec test:
// -------------------------
//	WE_SPEC_CASES_SIGNATURE_2(FVector, bool)
//		WE_SPEC_CASE_2(FVector(0.f, 0.f), false)
//		WE_SPEC_CASE_2(FVector(0.f, 1.f), true)
//	WE_SPEC_CASES_CODE_BEGIN_2(Direction, bExpectIsZero)
//	Describe("With direction", [Case]()
//	{
//		TestEquals(Case.Direction.IsNearlyZero(), Case.bExpectIsZero);
//	});
//	WE_SPEC_CASES_CODE_END()
//////////////////////////////////////////////////////////////////////////

////////////////////////
// Case Signature Macros

/**
 * Use this macro in an automation spec to declare the type signature of case parameters.
 * The WE_SPEC_CASE_* macro usages following this will expect their parameters to have
 * the same types in the same order.
 * For a full example in context of an automation spec see the beginning of this header.
 */
#define WE_SPEC_CASES_SIGNATURE_1(Param1Type) \
{ \
	struct FTestCaseInternal { Param1Type Param1; }; \
	TArray<FTestCaseInternal> WE_Spec_Cases = {}; \
	typedef Param1Type WE_Param1Type;

#define WE_SPEC_CASES_SIGNATURE_2(Param1Type, Param2Type) \
{ \
	struct FTestCaseInternal { Param1Type Param1; Param2Type Param2; }; \
	TArray<FTestCaseInternal> WE_Spec_Cases = {}; \
	typedef Param1Type WE_Param1Type; \
	typedef Param2Type WE_Param2Type;

#define WE_SPEC_CASES_SIGNATURE_3(Param1Type, Param2Type, Param3Type) \
{ \
	struct FTestCaseInternal { Param1Type Param1; Param2Type Param2; Param3Type Param3; }; \
	TArray<FTestCaseInternal> WE_Spec_Cases = {}; \
	typedef Param1Type WE_Param1Type; \
	typedef Param2Type WE_Param2Type; \
	typedef Param3Type WE_Param3Type;

#define WE_SPEC_CASES_SIGNATURE_4(Param1Type, Param2Type, Param3Type, Param4Type) \
{ \
	struct FTestCaseInternal { Param1Type Param1; Param2Type Param2; Param3Type Param3; Param4Type Param4; }; \
	TArray<FTestCaseInternal> WE_Spec_Cases = {}; \
	typedef Param1Type WE_Param1Type; \
	typedef Param2Type WE_Param2Type; \
	typedef Param3Type WE_Param3Type; \
	typedef Param4Type WE_Param4Type;

#define WE_SPEC_CASES_SIGNATURE_5(Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
{ \
	struct FTestCaseInternal { Param1Type Param1; Param2Type Param2; Param3Type Param3; Param4Type Param4; Param5Type Param5; }; \
	TArray<FTestCaseInternal> WE_Spec_Cases = {}; \
	typedef Param1Type WE_Param1Type; \
	typedef Param2Type WE_Param2Type; \
	typedef Param3Type WE_Param3Type; \
	typedef Param4Type WE_Param4Type; \
	typedef Param5Type WE_Param5Type;

#define WE_SPEC_CASES_SIGNATURE_6(Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type) \
{ \
	struct FTestCaseInternal { Param1Type Param1; Param2Type Param2; Param3Type Param3; Param4Type Param4; Param5Type Param5; Param6Type Param6; }; \
	TArray<FTestCaseInternal> WE_Spec_Cases = {}; \
	typedef Param1Type WE_Param1Type; \
	typedef Param2Type WE_Param2Type; \
	typedef Param3Type WE_Param3Type; \
	typedef Param4Type WE_Param4Type; \
	typedef Param5Type WE_Param5Type; \
	typedef Param6Type WE_Param6Type;

#define WE_SPEC_CASES_SIGNATURE_7(Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type, Param7Type) \
{ \
	struct FTestCaseInternal { Param1Type Param1; Param2Type Param2; Param3Type Param3; Param4Type Param4; Param5Type Param5; Param6Type Param6; Param7Type Param7; }; \
	TArray<FTestCaseInternal> WE_Spec_Cases = {}; \
	typedef Param1Type WE_Param1Type; \
	typedef Param2Type WE_Param2Type; \
	typedef Param3Type WE_Param3Type; \
	typedef Param4Type WE_Param4Type; \
	typedef Param5Type WE_Param5Type; \
	typedef Param6Type WE_Param6Type; \
	typedef Param7Type WE_Param7Type;

#define WE_SPEC_CASES_SIGNATURE_8(Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type, Param7Type, Param8Type) \
{ \
	struct FTestCaseInternal { Param1Type Param1; Param2Type Param2; Param3Type Param3; Param4Type Param4; Param5Type Param5; Param6Type Param6; Param7Type Param7; Param8Type Param8; }; \
	TArray<FTestCaseInternal> WE_Spec_Cases = {}; \
	typedef Param1Type WE_Param1Type; \
	typedef Param2Type WE_Param2Type; \
	typedef Param3Type WE_Param3Type; \
	typedef Param4Type WE_Param4Type; \
	typedef Param5Type WE_Param5Type; \
	typedef Param6Type WE_Param6Type; \
	typedef Param7Type WE_Param7Type; \
	typedef Param8Type WE_Param8Type;

#define WE_SPEC_CASES_SIGNATURE_9(Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type, Param7Type, Param8Type, Param9Type) \
{ \
	struct FTestCaseInternal { Param1Type Param1; Param2Type Param2; Param3Type Param3; Param4Type Param4; Param5Type Param5; Param6Type Param6; Param7Type Param7; Param8Type Param8; Param9Type Param9; }; \
	TArray<FTestCaseInternal> WE_Spec_Cases = {}; \
	typedef Param1Type WE_Param1Type; \
	typedef Param2Type WE_Param2Type; \
	typedef Param3Type WE_Param3Type; \
	typedef Param4Type WE_Param4Type; \
	typedef Param5Type WE_Param5Type; \
	typedef Param6Type WE_Param6Type; \
	typedef Param7Type WE_Param7Type; \
	typedef Param8Type WE_Param8Type; \
	typedef Param9Type WE_Param9Type;

////////////////////////
// Case Macros

/**
 * Use this macro in an automation spec to describe a single case parameter set.
 * The values must follow the type signature declared with WE_SPEC_CASES_SIGNATURE_.
 * For a full example in context of an automation spec see the beginning of this header.
 */
#define WE_SPEC_CASE_1(Param1) \
	WE_Spec_Cases.Add(FTestCaseInternal{ Param1 });
#define WE_SPEC_CASE_2(Param1, Param2) \
	WE_Spec_Cases.Add(FTestCaseInternal{ Param1, Param2 });
#define WE_SPEC_CASE_3(Param1, Param2, Param3) \
	WE_Spec_Cases.Add(FTestCaseInternal{ Param1, Param2, Param3 });
#define WE_SPEC_CASE_4(Param1, Param2, Param3, Param4) \
	WE_Spec_Cases.Add(FTestCaseInternal{ Param1, Param2, Param3, Param4 });
#define WE_SPEC_CASE_5(Param1, Param2, Param3, Param4, Param5) \
	WE_Spec_Cases.Add(FTestCaseInternal{ Param1, Param2, Param3, Param4, Param5 });
#define WE_SPEC_CASE_6(Param1, Param2, Param3, Param4, Param5, Param6) \
	WE_Spec_Cases.Add(FTestCaseInternal{ Param1, Param2, Param3, Param4, Param5, Param6 });
#define WE_SPEC_CASE_7(Param1, Param2, Param3, Param4, Param5, Param6, Param7) \
	WE_Spec_Cases.Add(FTestCaseInternal{ Param1, Param2, Param3, Param4, Param5, Param6, Param7 });
#define WE_SPEC_CASE_8(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8) \
	WE_Spec_Cases.Add(FTestCaseInternal{ Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8 });
#define WE_SPEC_CASE_9(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9) \
	WE_Spec_Cases.Add(FTestCaseInternal{ Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9 });

////////////////////////
// Parametrized Code Begin Macros

/**
 * Use this macro in an automation spec to begin a parametrized testing code and name
 * the case parameters described before.
 * After this call, you'll have a variable `Case : FTestCase` available, on which you
 * can access the case values using the names you declared with this macro.
 * For a full example in context of an automation spec see the beginning of this header.
 */
#define WE_SPEC_CASES_CODE_BEGIN_1(Param1Name) \
	for (const FTestCaseInternal& WE_TestCaseInternal : WE_Spec_Cases) \
	{ \
		struct FTestCase { WE_Param1Type Param1Name; }; \
		const FTestCase Case { WE_TestCaseInternal.Param1 };
		
#define WE_SPEC_CASES_CODE_BEGIN_2(Param1Name, Param2Name) \
	for (const FTestCaseInternal& WE_TestCaseInternal : WE_Spec_Cases) \
	{ \
		struct FTestCase { WE_Param1Type Param1Name; WE_Param2Type Param2Name; }; \
		const FTestCase Case { WE_TestCaseInternal.Param1, WE_TestCaseInternal.Param2 };
		
#define WE_SPEC_CASES_CODE_BEGIN_3(Param1Name, Param2Name, Param3Name) \
	for (const FTestCaseInternal& WE_TestCaseInternal : WE_Spec_Cases) \
	{ \
		struct FTestCase { WE_Param1Type Param1Name; WE_Param2Type Param2Name; WE_Param3Type Param3Name; }; \
		const FTestCase Case { WE_TestCaseInternal.Param1, WE_TestCaseInternal.Param2, WE_TestCaseInternal.Param3 };
		
#define WE_SPEC_CASES_CODE_BEGIN_4(Param1Name, Param2Name, Param3Name, Param4Name) \
	for (const FTestCaseInternal& WE_TestCaseInternal : WE_Spec_Cases) \
	{ \
		struct FTestCase { WE_Param1Type Param1Name; WE_Param2Type Param2Name; WE_Param3Type Param3Name;; WE_Param4Type Param4Name; }; \
		const FTestCase Case { WE_TestCaseInternal.Param1, WE_TestCaseInternal.Param2, WE_TestCaseInternal.Param3, WE_TestCaseInternal.Param4 };
		
#define WE_SPEC_CASES_CODE_BEGIN_5(Param1Name, Param2Name, Param3Name, Param4Name, Param5Name) \
	for (const FTestCaseInternal& WE_TestCaseInternal : WE_Spec_Cases) \
	{ \
		struct FTestCase { WE_Param1Type Param1Name; WE_Param2Type Param2Name; WE_Param3Type Param3Name; WE_Param4Type Param4Name; WE_Param5Type Param5Name; }; \
		const FTestCase Case { WE_TestCaseInternal.Param1, WE_TestCaseInternal.Param2, WE_TestCaseInternal.Param3, WE_TestCaseInternal.Param4, WE_TestCaseInternal.Param5 };
		
#define WE_SPEC_CASES_CODE_BEGIN_6(Param1Name, Param2Name, Param3Name, Param4Name, Param5Name, Param6Name) \
	for (const FTestCaseInternal& WE_TestCaseInternal : WE_Spec_Cases) \
	{ \
		struct FTestCase { WE_Param1Type Param1Name; WE_Param2Type Param2Name; WE_Param3Type Param3Name; WE_Param4Type Param4Name; WE_Param5Type Param5Name; WE_Param6Type Param6Name; }; \
		const FTestCase Case { WE_TestCaseInternal.Param1, WE_TestCaseInternal.Param2, WE_TestCaseInternal.Param3, WE_TestCaseInternal.Param4, WE_TestCaseInternal.Param5, WE_TestCaseInternal.Param6 };
		
#define WE_SPEC_CASES_CODE_BEGIN_7(Param1Name, Param2Name, Param3Name, Param4Name, Param5Name, Param6Name, Param7Name) \
	for (const FTestCaseInternal& WE_TestCaseInternal : WE_Spec_Cases) \
	{ \
		struct FTestCase { WE_Param1Type Param1Name; WE_Param2Type Param2Name; WE_Param3Type Param3Name; WE_Param4Type Param4Name; WE_Param5Type Param5Name; WE_Param6Type Param6Name; WE_Param7Type Param7Name; }; \
		const FTestCase Case { WE_TestCaseInternal.Param1, WE_TestCaseInternal.Param2, WE_TestCaseInternal.Param3, WE_TestCaseInternal.Param4, WE_TestCaseInternal.Param5, WE_TestCaseInternal.Param6, WE_TestCaseInternal.Param7 };

#define WE_SPEC_CASES_CODE_BEGIN_8(Param1Name, Param2Name, Param3Name, Param4Name, Param5Name, Param6Name, Param7Name, Param8Name) \
	for (const FTestCaseInternal& WE_TestCaseInternal : WE_Spec_Cases) \
	{ \
		struct FTestCase { WE_Param1Type Param1Name; WE_Param2Type Param2Name; WE_Param3Type Param3Name; WE_Param4Type Param4Name; WE_Param5Type Param5Name; WE_Param6Type Param6Name; WE_Param7Type Param7Name; WE_Param8Type Param8Name; }; \
		const FTestCase Case { WE_TestCaseInternal.Param1, WE_TestCaseInternal.Param2, WE_TestCaseInternal.Param3, WE_TestCaseInternal.Param4, WE_TestCaseInternal.Param5, WE_TestCaseInternal.Param6, WE_TestCaseInternal.Param7, WE_TestCaseInternal.Param8 };

#define WE_SPEC_CASES_CODE_BEGIN_9(Param1Name, Param2Name, Param3Name, Param4Name, Param5Name, Param6Name, Param7Name, Param8Name, Param9Name) \
	for (const FTestCaseInternal& WE_TestCaseInternal : WE_Spec_Cases) \
	{ \
		struct FTestCase { WE_Param1Type Param1Name; WE_Param2Type Param2Name; WE_Param3Type Param3Name; WE_Param4Type Param4Name; WE_Param5Type Param5Name; WE_Param6Type Param6Name; WE_Param7Type Param7Name; WE_Param8Type Param8Name; WE_Param9Type Param9Name; }; \
		const FTestCase Case { WE_TestCaseInternal.Param1, WE_TestCaseInternal.Param2, WE_TestCaseInternal.Param3, WE_TestCaseInternal.Param4, WE_TestCaseInternal.Param5, WE_TestCaseInternal.Param6, WE_TestCaseInternal.Param7, WE_TestCaseInternal.Param8, WE_TestCaseInternal.Param9 };

////////////////////////
// Parametrized Code End Macro

/**
 * Use this macro in an automation spec to end a parametrized testing code.
 * Does nothing special besides closing code scopes.
 * For a full example in context of an automation spec see the beginning of this header.
 */
#define WE_SPEC_CASES_CODE_END() \
	} \
}

#endif
