// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "ArchiveUObject.h"
#include "ShipRecords.h"
#include "ShipSaveGame.generated.h"


struct FShipSaveGameArchiveProxy : public FObjectAndNameAsStringProxyArchive 
{
	explicit FShipSaveGameArchiveProxy(FArchive& innerArchive);
};


/**
 * Represents the data that is saved/loaded to/from disk for a single ship.
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
	/**
	 * Populates this save object with the data for a ship in preparation for saving (ie. converting the AShipParts to FShipPartRecords).
	 *
	 * @param NameOfShip: The name of the ship being saved.
	 * @param InShipParts: The parts that make up this ship.
	 * @return: True if it saved successfully.
	 */
	bool SaveShip(const FString& NameOfShip, const TArray<class AShipPart*>& InShipParts);

	/**
	 * Exports saved ship data this object contains to AShipParts that can be used within the game world.
	 *
	 * @param WorldContext: An object that has a valid reference to the world.
	 * @param OutShipParts: The array the created ship parts should be stored in.
	 * @return: True if loaded successfully.
	 */
	bool LoadShip(UObject* WorldContext, TArray<class AShipPart*>& OutShipParts) const;

	FORCEINLINE const FString& GetShipName() const noexcept { return ShipName; }
};
