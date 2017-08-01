// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#ifndef __SHIPBUILDINGDEMO_H__
#define __SHIPBUILDINGDEMO_H__

#include "Engine.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFlying, Log, All);

namespace ShipUtils
{
	/**
	 *	Clears an array but retains the memory.
	 *	TODO: make constexpr once upgraded to C++14
	 *
	 *	@param Arr: The array to clear.
	 *	@param bRetainSlack: Should the memory be left allocated.
	 */
	template<typename T>
	FORCEINLINE static void ClearArray(TArray<T>& Arr, bool bRetainSlack = true)
	{
		Arr.Empty(bRetainSlack ? Arr.Num() : 0);
	}

	/**
	 *	Destroys an array of actors, optionally leaving the memory intact.
	 *	TODO: make constexpr once upgraded to C++14
	 *
	 *	@param ShipParts: The ship parts to destroy.
	 *	@param bRetainSlack: Should the memory be left allocated.
	 */
	template<typename ActorSubclassType>
	FORCEINLINE static void DestroyActorArray(TArray<ActorSubclassType*>& Arr, bool bRetainSlack = false)
	{
		const int32 Count = bRetainSlack ? Arr.Num() : 0;
		for (int32 i = Arr.Num() - 1; i >= 0; --i)
		{
			Arr[i]->Destroy();
		}
		Arr.Empty(Count);
	}

	/**
	 *	Gets the name of the enum value. Corresponds to the actual enum value name and not the meta display name.
	 *	Source: https://wiki.unrealengine.com/Enums_For_Both_C%2B%2B_and_BP#Get_Name_of_Enum_as_String
	 *
	 *	@param Name: The name of the enum type (aka TEnum).
	 *	@param Value: The enum value whose name should be looked up.
	 *	@return: The name of the enum or an empty string if the lookup failed.
	 */
	template<typename TEnum>
	FORCEINLINE static FString EnumValueToString(const FString& Name, TEnum Value)
	{
		static const FString EmptyStr{ "" };
		if (auto enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true))
		{
			return enumPtr->GetEnumName((int32)Value);
		}
		return EmptyStr;
	}
}

#endif
