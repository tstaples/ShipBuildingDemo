// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "ShipBuildingTypes.h"
#include "ShipAttachPoint.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHIPBUILDINGDEMO_API UShipAttachPoint : public USceneComponent
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	class AShipPart* OwningShipPart;

	// Sprite to represent attach points when building the ship. Should only be shown then.
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* AttachPointSphere;

	// Dynamic material instance of the sphere material
	UPROPERTY(Transient)
	class UMaterialInstanceDynamic* SphereDMI;

	// Is this attach point currently highlighted.
	bool bIsHighlighted;

	// The ship part this point is attached to.
	UPROPERTY()
	class AShipPart* AttachedToShipPart;

protected:
	// What other parts this part is compatible with.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PartSettings")
	TArray<EPartType> CompatibleParts;

public:	
	// Sets default values for this component's properties
	UShipAttachPoint();

	//virtual void InitializeComponent() override;
	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	/**
	 *	Checks if this attach point is compatible with another attach point.
	 *
	 * @param OtherPoint: the point to compare this attach point with.
	 * @return: true if it is compatible.
	 */
	bool IsCompatibleWith(const UShipAttachPoint* OtherPoint) const;

	// Is this point attached to another ship part
	bool IsAttached() const;

	void AttachToShipPart(AShipPart* PartToAttachTo);

	// Changes the sphere color to indicate it's highlighted.
	void SetHighlighted(bool bHighlighted);

	FORCEINLINE const TArray<EPartType>& GetCompatibleParts() const { return CompatibleParts; }
};
