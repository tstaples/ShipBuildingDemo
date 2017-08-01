// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "AssetData.h"
#include "ShipBuildingTypes.h"
#include "ShipPartFactory.generated.h"

class AShipPart;

/**
 *	Holds basic data of a ship part for use in the UI, as well as more detailed data for internal use.
 */
USTRUCT(BlueprintType)
struct FShipPartData
{
	GENERATED_BODY()

	// Clean name of the asset used for display in UI.
	UPROPERTY(BlueprintReadOnly, Category = FShipPartData)
	FName Name;

	// The part type/category.
	UPROPERTY(BlueprintReadOnly, Category = FShipPartData)
	EPartType PartType;

	// Default constructor
	FShipPartData() = default;

private:
	friend class UShipPartFactory;

	// Constructor that should be used internally.
	// Verifies the asset name contains the prefix.
	explicit FShipPartData(const FAssetData& Data);

	// The prefix used in the asset name.
	static const FString Prefix;

	// Full name of the asset including the prefix. Used for doing lookups.
	FName FullName;

	// The asset data for this ship part.
	FAssetData AssetData;

	// Gets the GeneratedClassName from the AssetData.
	static FString GetGeneratedClassName(const FShipPartData& ShipPartData);
};

/**
 *	Responsible for loading and constructing ship parts.
 */
UCLASS()
class SHIPBUILDINGDEMO_API UShipPartFactory : public UObject
{
	GENERATED_BODY()
	
	UPROPERTY()
	class UObjectLibrary* ShipPartLibrary;

	// Custom ship part data struct for handing off to the UI. Populated during Init.
	UPROPERTY()
	TArray<FShipPartData> ShipPartData;

	// Has Init been called and has the asset data been loaded.
	bool bAssetDataLoaded;

public:
	UShipPartFactory();

	/**
	 *	Loads the asset data for the ship parts and populates ShipPartData.
	 *	Note: Must be called before using MakeShipPart or GetShipPartData.
	 *
	 *	@param RootShipPartPath: The path to the root ship part directory, relative to /Game (ie. /Game/ShipParts).
	 */
	void Init(const FString& RootShipPartPath);

	/**
	 *	Creates an instance of a ship part in the world using the name.
	 *
	 *	@param WorldContext: An object instance that has a valid reference to the world.
	 *	@param PartName: The name of the part to spawn.
	 *	@param SpawnLocation: Where in the world to spawn the part.
	 *	@return: The created ship part or nullptr if it failed.
	 */
	AShipPart* MakeShipPart(UObject* WorldContext, FName PartName, FVector SpawnLocation = FVector(0.f, 0.f, 200.f));

	// Accessors
	FORCEINLINE bool HasLoadedAssetData() const noexcept { return bAssetDataLoaded; }
	FORCEINLINE const TArray<FShipPartData>& GetShipPartData() const noexcept { return ShipPartData; }

private:
	/**
	 *	Maps the ship part paths to the corresponding enum types. Used to lookup the part category.
	 *
	 *	@param RootPath: The root path of all the ship parts.
	 *	@return: A map pairing the full path of the ship part (ie. /Game/ShipParts/Cockpit) to the corresponding enum (ie. PT_Cockpit).
	 */
	TMap<FString, EPartType> MakeShipPartPathsToTypes(const FString& RootPath) const;
};
