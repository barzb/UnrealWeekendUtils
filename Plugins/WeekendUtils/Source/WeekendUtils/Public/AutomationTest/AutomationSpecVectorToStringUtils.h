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
		 * Alternative implementation of ToString for FVector2 which doesn't introduce weird sub-categories
		 * based on '.' characters in the session frontend when used in automation spec Describe parameters.
		 * e.g. Describe(FString::Printf(TEXT("%s"), *SpecStringUtils::ToString(Vector)), [this](){ ... });
		 */
		FORCEINLINE FString ToString(const FVector2f& Vector)
		{
			return { "(" +
				FString::FromInt(StaticCast<int>(Vector.X)) + "," + FString::FromInt(StaticCast<int>(FMath::Frac(Vector.X) * 10)) + " | " +
				FString::FromInt(StaticCast<int>(Vector.Y)) + "," + FString::FromInt(StaticCast<int>(FMath::Frac(Vector.Y) * 10)) + ")" };
		}
		
		/**
		 * Alternative implementation of ToString for FVector2 which doesn't introduce weird sub-categories
		 * based on '.' characters in the session frontend when used in automation spec Describe parameters.
		 * e.g. Describe(FString::Printf(TEXT("%s"), *SpecStringUtils::ToString(Vector)), [this](){ ... });
		 */
		FORCEINLINE FString ToString(const FVector2d& Vector)
		{
			return { "(" +
				FString::FromInt(StaticCast<int>(Vector.X)) + "," + FString::FromInt(StaticCast<int>(FMath::Frac(Vector.X) * 10)) + " | " +
				FString::FromInt(StaticCast<int>(Vector.Y)) + "," + FString::FromInt(StaticCast<int>(FMath::Frac(Vector.Y) * 10)) + ")" };
		}
		
		/**
		 * Alternative implementation of ToString for FVector2 which doesn't introduce weird sub-categories
		 * based on '.' characters in the session frontend when used in automation spec Describe parameters.
		 * e.g. Describe(FString::Printf(TEXT("%s"), *SpecStringUtils::ToString(Vector)), [this](){ ... });
		 */
		FORCEINLINE FString ToString(const FVector& Vector)
		{
			return { "(" +
				FString::FromInt(StaticCast<int>(Vector.X)) + "," + FString::FromInt(StaticCast<int>(FMath::Frac(Vector.X) * 10)) + " | " +
				FString::FromInt(StaticCast<int>(Vector.Y)) + "," + FString::FromInt(StaticCast<int>(FMath::Frac(Vector.Y) * 10)) + " | " +
				FString::FromInt(StaticCast<int>(Vector.Z)) + "," + FString::FromInt(StaticCast<int>(FMath::Frac(Vector.Z) * 10)) + ")" };
		}
	}
}

#endif
