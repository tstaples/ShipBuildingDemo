// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "ShipBuildingDemo.h"
#include "ShipBuildingDemoGameMode.h"
#include "ShipBuildingDemoPawn.h"

AShipBuildingDemoGameMode::AShipBuildingDemoGameMode()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = AShipBuildingDemoPawn::StaticClass();
}
