// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ShipBuilding/ShipBuildingTypes.h"
#include "ShipRecords.generated.h"


USTRUCT()
struct FShipPartRecord
{
	GENERATED_BODY()

	// The ShipPart class this part is.
	UPROPERTY()
	UClass* ShipTemplate;

	// Transform of the part.
	UPROPERTY()
	FTransform PartTransform;

	// Part data - should retain attach point info like which other point it's attached to.
	UPROPERTY()
	TArray<uint8> ShipPartData;
};

