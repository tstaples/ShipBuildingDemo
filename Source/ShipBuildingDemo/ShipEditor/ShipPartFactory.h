// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "AssetData.h"
#include "ShipBuildingTypes.h"
#include "ShipPartFactory.generated.h"

class AShipPart;

USTRUCT(BlueprintType)
struct FShipPartData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = FShipPartData)
	FName Name;

	UPROPERTY(BlueprintReadOnly, Category = FShipPartData)
	EPartType PartType;
};

/**
 * 
 */
UCLASS()
class SHIPBUILDINGDEMO_API UShipPartFactory : public UObject
{
	GENERATED_BODY()
	
	UPROPERTY()
	class UObjectLibrary* ShipPartLibrary;

	TArray<FAssetData> ShipPartAssetData;
	
	UPROPERTY()
	TArray<FShipPartData> ShipPartData;

	bool bAssetDataLoaded;

public:
	UShipPartFactory();

	// Loads asset data.
	void Init(const FString& RootShipPartPath);

	AShipPart* MakeShipPart(UObject* WorldContext, FName PartName);

	bool HasLoadedAssetData() const { return bAssetDataLoaded; }
	const TArray<FShipPartData>& GetShipPartData() const { return ShipPartData; }
};
