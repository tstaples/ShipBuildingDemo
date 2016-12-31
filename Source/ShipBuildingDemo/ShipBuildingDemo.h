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
}

#endif
