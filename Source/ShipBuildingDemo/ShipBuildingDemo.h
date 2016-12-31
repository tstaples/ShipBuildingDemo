// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#ifndef __SHIPBUILDINGDEMO_H__
#define __SHIPBUILDINGDEMO_H__

#include "Engine.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFlying, Log, All);

namespace ShipUtils
{
	// Clears an array but retains the memory.
	template<typename T>
	void ClearArray(TArray<T>& Arr, bool bRetainSlack = true)
	{
		const int32 Slack = bRetainSlack ? Arr.Num() : 0;
		Arr.Empty(Slack);
	}

	/**
	 *	Destroys an array of actors.
	 *
	 *	@param ShipParts: The ship parts to destroy.
	 *	@param bRetainSlack: Should the memory be left allocated.
	 */
	template<typename ActorSubclassType>
	void DestroyActorArray(TArray<ActorSubclassType*>& Arr, bool bRetainSlack = false)
	{
		const int32 Count = bRetainSlack ? Arr.Num() : 0;
		for (int32 i = Arr.Num() - 1; i >= 0; --i)
		{
			Arr[i]->Destroy();
		}
		Arr.Empty(Count);
	}
}

#endif
