// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ShipBuildingTypes.h"
#include "ShipPart.generated.h"

class UShipAttachPoint;

UCLASS()
class SHIPBUILDINGDEMO_API AShipPart : public AActor
{
	GENERATED_BODY()

	// Cached attach point components
	UPROPERTY(Transient)
	TArray<UShipAttachPoint*> AttachPoints;

	// Cached mesh component
	UPROPERTY(Transient)
	UStaticMeshComponent* ShipPartMesh;

protected:
	// The type of part. TODO: make config or SaveGame depending on how we serialize the parts.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PartSettings")
	EPartType PartType;

	// What other parts this part is compatible with. Attach components will use this as the default configuration.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PartSettings")
	TArray<EPartType> DefaultCompatibleParts;

	// How close in uu the point must be to another valid point before they snap together.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category="PartSettings")
	float MinSnapDistance = 50.f;

public:	
	AShipPart();

	// Begin AActor Interface.
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	// End AActor Interface.

	void Select();
	void Deselect();

	// Returns true if any of this part's attach points are attached to anything
	bool IsAttached() const;

	// Detatches from any other ship parts
	void DetatchAllPoints();

	/**
	 *	Enables/Disables highlighting on all attach points associated with this part.
	 *
	 *	@param bHighlighted: Whether the points should be highlighted or not.
	 */
	void SetAllPointsHighlighted(bool bHighlighted);

	/**
	 *	Gets all attach points that aren't attached to anything.
	 */
	TArray<UShipAttachPoint*> GetAvailableAttachPoints() const;

	// Accessors
	FORCEINLINE EPartType GetPartType() const { return PartType; }
	FORCEINLINE TArray<EPartType> GetDefaultCompatibleParts() const { return DefaultCompatibleParts; }
	FORCEINLINE TArray<UShipAttachPoint*>& GetAttachPoints() { return AttachPoints; }
	FORCEINLINE const TArray<UShipAttachPoint*>& GetAttachPoints() const { return AttachPoints; }
	FORCEINLINE float GetMinSnapDistance() const { return MinSnapDistance; }
	FORCEINLINE FBoxSphereBounds GetSnapBounds() const { return ShipPartMesh->Bounds.ExpandBy(MinSnapDistance); }
};
