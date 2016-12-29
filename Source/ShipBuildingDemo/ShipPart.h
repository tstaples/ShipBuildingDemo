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

public:	
	AShipPart();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;

	void Select();
	void Deselect();

	void UnhighlightAllAttachPoints();

	// Gets all attach points that aren't attached to anything.
	TArray<UShipAttachPoint*> GetAvailableAttachPoints() const;

	FORCEINLINE EPartType GetPartType() const { return PartType; }
	FORCEINLINE TArray<EPartType> GetDefaultCompatibleParts() const { return DefaultCompatibleParts; }
};
