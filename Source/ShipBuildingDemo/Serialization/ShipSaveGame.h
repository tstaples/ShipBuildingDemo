// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "ArchiveUObject.h"
#include "ShipRecords.h"
#include "ShipSaveGame.generated.h"


/**
 * TBH I'm not entirely sure what this is for.
 */
struct FShipSaveGameArchiveProxy : public FObjectAndNameAsStringProxyArchive 
{
	explicit FShipSaveGameArchiveProxy(FArchive& innerArchive);
};


/**
 * 
 */
UCLASS()
class SHIPBUILDINGDEMO_API UShipSaveGame : public USaveGame
{
	GENERATED_BODY()
	
	// Name given by the user.
	UPROPERTY()
	FString ShipName;

	// The parts that make up the ship.
	UPROPERTY()
	TArray<FShipPartRecord> ShipPartRecords;
	
public:
	bool SaveShip(const FString& NameOfShip, const TArray<class AShipPart*>& InShipParts);

	bool LoadShip(UObject* WorldContext, TArray<class AShipPart*>& OutShipParts) const;

	FORCEINLINE const FString& GetShipName() const { return ShipName; }
};
