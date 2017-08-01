// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "ShipBuildingTypes.h"
#include "ShipAttachPoint.generated.h"


/**
 * Represents a point on a ShipPart that other ShipParts can attach to. These are created as child components of a ShipPart and placed where the parts should attach. 
 * By default these will inherit the `DefaultCompatibleParts` of it's owning ShipPart at runtime, but you can override those directly on the attach point.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHIPBUILDINGDEMO_API UShipAttachPoint : public USceneComponent
{
	GENERATED_BODY()

	// The ship part that this attach point belongs to.
	UPROPERTY(Transient)
	class AShipPart* OwningShipPart;

	// Arrow to indicate the attach point's normal which is used to determine if points can snap together.
	UPROPERTY()
	class UArrowComponent* DirectionArrow;

	// Sprite to represent attach points when building the ship. Should only be shown then.
	UPROPERTY()
	class UStaticMeshComponent* AttachPointSphere;

	// Dynamic material instance of the sphere material. Used to change the color when highlighting.
	UPROPERTY(Transient)
	class UMaterialInstanceDynamic* SphereDMI;

	// Is this attach point currently highlighted.
	bool bIsHighlighted;

	// The attach point of another ship part this point is attached to.
	UPROPERTY()
	class UShipAttachPoint* AttachedToPoint;

protected:
	// What other parts this part is compatible with.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PartSettings")
	TArray<EPartType> CompatibleParts;

public:	
	// Sets default values for this component's properties
	UShipAttachPoint();

	// Begin SceneComponent Interface
	void InitializeComponent() override;
	void PostInitProperties() override;
	void BeginPlay() override;
	void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	// End SceneComponent Interface

	static void AttachPoints(UShipAttachPoint* A, UShipAttachPoint* B);
	static void DetachPoints(UShipAttachPoint* A, UShipAttachPoint* B);

	/**
	 *	Checks if this attach point is compatible with a part type.
	 *
	 *	@param PartType: the type of the part to check for compatibility with.
	 *	@return: true if it is compatible.
	 */
	bool IsCompatibleWith(EPartType PartType) const;

	/**
	 *	Is this point attached to another ship part
	 */
	bool IsAttached() const noexcept;

	// If this point is attached to the other.
	bool IsAttachedToPoint(const UShipAttachPoint* OtherPoint) const noexcept;

	// Detaches from it's current point
	void DetachFromPoint();

	/**
	 *	Returns the normal of the attach point (based on arrow component direction.
	 */
	FVector GetNormal() const;

	/**
	 *	Changes the sphere color to indicate it's highlighted.
	 *
	 *	@param bHighlighted: if we should enable or disable highlighting.
	 */
	void SetHighlighted(bool bHighlighted);

	// Acessors
	FORCEINLINE AShipPart* GetOwningShipPart() const { return OwningShipPart; }
	FORCEINLINE UShipAttachPoint* GetAttachedToPoint() const { return AttachedToPoint; }
	FORCEINLINE AShipPart* GetAttachedToShipPart() const { return AttachedToPoint ? AttachedToPoint->GetOwningShipPart() : nullptr; }
	FORCEINLINE const TArray<EPartType>& GetCompatibleParts() const { return CompatibleParts; }

private:
	/**
	 *	Attaches This point to a ship part.
	 */
	void AttachToPoint(UShipAttachPoint* PointToAttachTo);
};
