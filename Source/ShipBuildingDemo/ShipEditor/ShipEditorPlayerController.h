// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "ShipEditorPlayerController.generated.h"

class AShipPart;
class UShipAttachPoint;

/**
 * 
 */
UCLASS()
class SHIPBUILDINGDEMO_API AShipEditorPlayerController : public APlayerController
{
	GENERATED_BODY()

	struct FAttachPointCacheEntry
	{
		UShipAttachPoint* OwnedPoint;
		UShipAttachPoint* OtherPoint;

		FAttachPointCacheEntry() {}
		FAttachPointCacheEntry(UShipAttachPoint* Owned, UShipAttachPoint* Other) 
			: OwnedPoint(Owned), OtherPoint(Other) {}

		bool IsValid() const { return (OwnedPoint && OtherPoint); }
	};

	// Ship attach points compatible with the currently held ship part.
	// Populated when a ship part is selected.
	// TODO: Need to make uproperty to prevent gc?
	TArray<FAttachPointCacheEntry> CachedCompatiblePoints;

	// Ship part currently being held.
	UPROPERTY(Transient)
	AShipPart* CurrentlyHeldShipPart;

	// Wolrd space position of CurrentlyHeldShipPart last frame.
	//FVector PreviousHeldPartLocation;

	// All of the ship parts.
	UPROPERTY()
	TArray<AShipPart*> ShipParts;

	UPROPERTY()
	class UShipPartFactory* ShipPartFactory;

public:
	AShipEditorPlayerController();
	
	// Begin PlayerController Interface.
	virtual void PostInitializeComponents() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;
	// End PlayerController Interface.

	UFUNCTION(BlueprintCallable, Category = "ShipPartFactory")
	void SpawnShipPart(FName PartName);

	// Temp BPCallable for use in our hacky temp UI.
	// ShipName is name user enters.
	// Returns if it saved successfully or not.
	UFUNCTION(Exec, BlueprintCallable, Category = "ShipSaving")
	bool SaveShip(const FString& ShipName);

	// Temp BPCallable for use in our hacky temp UI.
	// ShipName is name of the ship to load.
	// Returns if it loaded successfully or not.
	UFUNCTION(Exec, BlueprintCallable, Category = "ShipSaving")
	bool LoadShip(const FString& ShipName);

	/**
	 *	Clears all current ship parts.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "ShipManipulation")
	void ClearShip();

	FORCEINLINE bool HoldingShipPart() const { return (CurrentlyHeldShipPart != nullptr); }
	FORCEINLINE class UShipPartFactory* GetShipPartFactory() const { return ShipPartFactory; }

private:
	// Input callbacks
	UFUNCTION()
	void OnClick();
	UFUNCTION()
	void OnReleaseClick();
	UFUNCTION()
	void DeleteSelectedPart();

	/**
	 *	Gathers all Attach points that are compatible with the ship part.
	 *
	 *	@param ShipPart: The part to find compatible points for.
	 *	@param OutCompatiblePoints: The compatible points that were found.
	 *	@return: True if OutCompatiblePoints contains any points.
	 */
	bool CollectCompatiblePoints(const AShipPart* ShipPart, TArray<FAttachPointCacheEntry>& OutCompatiblePoints) const;

	/**
	 *	Highlights/Unhighlights all points in a cache.
	 *
	 *	@param bHighlighted: Whether or not to highlight the points.
	 *	@param InPoints: The list of cache entries whose points are to be highlighted/not.
	 */
	void SetCachedPointsHighlighted(bool bHighlighted, TArray<FAttachPointCacheEntry>& InPoints) const;

	/**
	 *	Searches the cache for any entry whose two points meet the requirements to snap together.
	 *
	 *	@param CompatiblePoints: The cache of points to search.
	 *	@param Delta: The movement delta of the mouse since last frame.
	 *	@return: The index of the cache entry whose points should be snapped together. INDEX_NONE otherwise.
	 */
	int32 FindPointsToSnapTogether(const TArray<FAttachPointCacheEntry>& CompatiblePoints, const FVector& Delta) const;

	/**
	 *	Destroys a ship part and handles detaching it from other parts.
	 *
	 *	@param ShipPart: The ship part to destroy.
	 */
	void DestroyShipPart(AShipPart* ShipPart);

	//////////////////////////////////////////////////////////////////////////
	// Saving
	//////////////////////////////////////////////////////////////////////////

	/**
	 *	Loads or creates a UShipSaveGame instance.
	 *
	 *	@param ShipName: The name of the ship to load the data for.
	 *	@return: The sava data or null if it failed to load or create a new instance.
	 */
	class UShipSaveGame* GetSaveDataForShip(const FString& ShipName) const;
};
