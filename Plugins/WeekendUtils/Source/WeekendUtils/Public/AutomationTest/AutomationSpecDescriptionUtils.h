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

namespace WeekendUtils
{
	namespace SpecStringUtils
	{
		/**
		 * Alternative implementation of ToString for float which doesn't introduce weird sub-categories
		 * based on '.' characters in the session frontend when used in automation spec Describe parameters.
		 * e.g. Describe(FString::Printf(TEXT("%s"), *SpecStringUtils::ToString(2.3f)), [this](){ ... });
		 */
		FORCEINLINE FString ToString(const float Float)
		{
			return FString::FromInt(FMath::TruncToInt(Float)) + ","
				+ FString::FromInt(StaticCast<int>(FMath::Frac(Float) * 100));
		}
		
		/**
		 * Alternative implementation of ToString for double which doesn't introduce weird sub-categories
		 * based on '.' characters in the session frontend when used in automation spec Describe parameters.
		 * e.g. Describe(FString::Printf(TEXT("%s"), *SpecStringUtils::ToString(2.3f)), [this](){ ... });
		 */
		FORCEINLINE FString ToString(const double Double)
		{
			return FString::FromInt(StaticCast<int>(Double)) + "," + FString::FromInt(StaticCast<int>(FMath::Frac(Double) * 100));
		}
		
		/**
		 * Alternative implementation of ToString for FVector2 which doesn't introduce weird sub-categories
		 * based on '.' characters in the session frontend when used in automation spec Describe parameters.
		 * e.g. Describe(FString::Printf(TEXT("%s"), *SpecStringUtils::ToString(Vector)), [this](){ ... });
		 */
		FORCEINLINE FString ToString(const FVector2f& Vector)
		{
			return "(" + ToString(Vector.X) + " | " + ToString(Vector.Y) + ")";
		}
		
		/**
		 * Alternative implementation of ToString for FVector2 which doesn't introduce weird sub-categories
		 * based on '.' characters in the session frontend when used in automation spec Describe parameters.
		 * e.g. Describe(FString::Printf(TEXT("%s"), *SpecStringUtils::ToString(Vector)), [this](){ ... });
		 */
		FORCEINLINE FString ToString(const FVector2d& Vector)
		{
			return "(" + ToString(Vector.X) + " | " + ToString(Vector.Y) + ")";
		}
		
		/**
		 * Alternative implementation of ToString for FVector2 which doesn't introduce weird sub-categories
		 * based on '.' characters in the session frontend when used in automation spec Describe parameters.
		 * e.g. Describe(FString::Printf(TEXT("%s"), *SpecStringUtils::ToString(Vector)), [this](){ ... });
		 */
		FORCEINLINE FString ToString(const FVector& Vector)
		{
			return "(" + ToString(Vector.X) + " | " + ToString(Vector.Y) + " | " + ToString(Vector.Z) + ")";
		}
	}
}

#endif
